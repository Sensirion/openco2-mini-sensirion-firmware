#include <Preferences.h>
#include <SensirionI2cStcc4.h>

#include "BleConnectService.h"
#include "FrcBleService.h"
#include "LedUtils.h"
#include "SensirionUptBleServer.h"
#include "SettingsBleService.h"
#include "config.h"

using namespace sensirion::upt;
static auto constexpr TAG = "MAIN";

SensirionI2cStcc4 stcc4;
Preferences persist;
LedUtils led;

bool frcRequested = false;
int16_t stcc4Co2 = 0;
float stcc4PcbTemperature, ambientTemperature = 0.0;
float stcc4PcbHumidity, ambientHumidity = 0.0;
uint16_t stcc4Status = 0;

static int64_t lastMeasurementTimeMs = 0;

ble_server::NimBLELibraryWrapper lib;
ble_server::FrcBleService frcBleService(lib);
ble_server::SettingsBleService settingsBleService(lib);
ble_server::UptBleServer uptBleServer(lib, core::T_RH_CO2_ALT);
ble_server::BleConnectService bleConnectService(lib, led);

void frcRequestCallback(int16_t referenceCo2Level);
void nameChangeRequestCallback(const std::string &newName);
int16_t measureAndUpdate();
void checkAndSleep(bool hasError);
void setupStcc4Measurement();
void setupBleServer();

void setup() {
 setCpuFrequencyMhz(80);

  // Setup I2C, LED and Persistence
  Wire.begin(SDA_PIN, SCL_PIN);
  stcc4.begin(Wire, STCC4_I2C_ADDR_64);
  persist.begin("ble-settings", false);
  led.begin();

  // initialize measurement for STCC4
  setupStcc4Measurement();

  // start BLE server for integration with MyAmbience
  setupBleServer();

  // delay until the first measurement is ready
  delay(1000);
}

void loop() {
  int16_t error = NO_ERROR;
  if (millis() - lastMeasurementTimeMs >= STCC4_MEASUREMENT_INTERVAL_MS &&
      !frcRequested) {
    error = measureAndUpdate();
  }

  // handle download requests
  uptBleServer.handleDownload();
  delay(STCC4_MEASURE_CHECK_MS);

  // go to sleep if necessary
  checkAndSleep(error != NO_ERROR);
}

/**
 * Configure and start the STCC4 sensor measurement sequence.
 *
 * Steps
 * - Stop any ongoing continuous measurement.
 * - Perform the mandatory sensor conditioning.
 * - Start a new continuous measurement.
 *
 * Error handling
 * - On any driver error, the LED blinks red and the function returns early.
 */
void setupStcc4Measurement() {
  int16_t error = stcc4.stopContinuousMeasurement();
  if (error != NO_ERROR) {
    led.blinkRed();
    ESP_LOGE(TAG, "Error while calling stopContinuousMeasurement.");
    return;
  }

  error = stcc4.performConditioning();
  if (error != NO_ERROR) {
    led.blinkRed();
    ESP_LOGE(TAG, "Error while calling performConditioning.");
    return;
  }

  ESP_LOGI(TAG, "Starting measurement...");
  error = stcc4.startContinuousMeasurement();
  if (error != NO_ERROR) {
    led.blinkRed();
    ESP_LOGE(TAG, "Error while calling startContinuousMeasurement.");
    return;
  }
}

/**
 * Initialize and start the BLE server and services used by MyAmbience.
 *
 * Behavior
 * - Restores the alternative device name from non‑volatile storage
 *   (or uses the default).
 * - Configures connection timing, registers the FRC, Settings, and
 *   connection‑feedback services, and starts advertising.
 */
void setupBleServer() {
  const String name =
      persist.getString("alt-device-name", BLE_DEFAULT_DEVICE_NAME);
  frcBleService.registerFrcRequestCallback(frcRequestCallback);
  settingsBleService.setAltDeviceName(name.c_str());
  settingsBleService.setEnableWifiSettings(false);
  settingsBleService.registerDeviceNameChangeCallback(
      nameChangeRequestCallback);

  uptBleServer.registerBleServiceProvider(frcBleService);
  uptBleServer.registerBleServiceProvider(settingsBleService);
  uptBleServer.registerBleServiceProvider(bleConnectService);
  
  uptBleServer.begin();
  ESP_LOGI(TAG, "Setup done. Device is advertised with name = %s",
           name.c_str());
}

/**
 * Read a measurement from the STCC4 sensor and publish it via BLE.
 *
 * Data flow
 * - Reads temperature, humidity and CO2 from the sensor.
 * - On success, updates the timestamp, writes the values to the current
 *   UPT BLE sample, commits the sample, and updates the LED color derived
 *   from the CO2 concentration.
 *
 * Error handling
 * - On driver error, logs the error string and returns the error code
 *   without updating the BLE sample.
 *
 * @return NO_ERROR on success, otherwise the STCC4 driver error code.
 */
int16_t measureAndUpdate() {
  const int16_t error = stcc4.readMeasurement(stcc4Co2, stcc4PcbTemperature,
                                              stcc4PcbHumidity, stcc4Status);
  if (error != NO_ERROR) {
    char errormessage[128];
    errorToString(error, errormessage, sizeof(errormessage));
    ESP_LOGE(
        TAG,
        "Error while calling readMeasurement. error code: %d, message:\n%s",
        error, errormessage);
    return error;
  }
  lastMeasurementTimeMs = millis();

  ambientTemperature = stcc4PcbTemperature + T_COMP;
  ambientHumidity = stcc4PcbHumidity * exp(M * TN * ((stcc4PcbTemperature - T2) / ((TN + stcc4PcbTemperature) * (TN + ambientTemperature))));;

  uptBleServer.writeValueToCurrentSample(
      ambientTemperature, core::SignalType::TEMPERATURE_DEGREES_CELSIUS);
  uptBleServer.writeValueToCurrentSample(
      ambientHumidity, core::SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
  uptBleServer.writeValueToCurrentSample(
      stcc4Co2, core::SignalType::CO2_PARTS_PER_MILLION);
  uptBleServer.commitSample();

  led.setColorFromCo2(stcc4Co2);
  return error;
}

/**
 * Enter light sleep for a while, if safe to do so.
 *
 * Conditions
 * - Skips sleeping when there are connected BLE devices or when the
 *   last measurement resulted in an error (to allow quick retry/handling).
 */
void checkAndSleep(const bool hasError) {
  if (uptBleServer.hasConnectedDevices() || hasError) {
    return;
  }

  // Wake up once in between 2 measurements to handle BLE events
  constexpr uint64_t sleepDurationMs =
      STCC4_MEASUREMENT_INTERVAL_MS/2 - STCC4_MEASURE_CHECK_MS;
  esp_sleep_enable_timer_wakeup(sleepDurationMs * 1000);
  esp_light_sleep_start();
}

/**
 * Callback invoked on Forced Recalibration (FRC) request from BLE.
 *
 * Behavior
 * - Guards against concurrent FRC execution via the global flag.
 * - Triggers the STCC4 forced recalibration using the provided
 *   reference CO2 level and logs the resulting correction value.
 */
void frcRequestCallback(const int16_t referenceCo2Level) {
  if (frcRequested)
    return;

  ESP_LOGI(TAG, "FRC requested...");
  frcRequested = true;
  int16_t correction;
  int16_t error = stcc4.stopContinuousMeasurement();
  if (error != NO_ERROR) {
    led.blinkRed();
    ESP_LOGE(TAG, "Error while calling stopContinuousMeasurement.");
    return;
  }
  int16_t error_frc = stcc4.performForcedRecalibration(referenceCo2Level, correction);
  if (error_frc != NO_ERROR) {
    ESP_LOGE(TAG, "Error while calling performForcedRecalibration.");
  }
  error = stcc4.startContinuousMeasurement();
  if (error != NO_ERROR) {
    ESP_LOGE(TAG, "Error while calling startContinuousMeasurement.");
  }
  frcRequested = false;
  if (error_frc != NO_ERROR||error != NO_ERROR) {
    led.blinkRed();
    return;
  }
  ESP_LOGI(TAG, "FRC completed with correction value: %d", correction);
  led.blinkGreen();
}

/**
 * Callback to persist a requested device name change.
 *
 * Behavior
 * - Stores the new alternative device name in Preferences so it
 *   survives reboots; BLE service picks it up on next start.
 */
void nameChangeRequestCallback(const std::string &newName) {
  ESP_LOGI(TAG, "Device name change requested: %s. Persisting new name...",
           newName.c_str());

  persist.putString("alt-device-name", newName.c_str());
}
