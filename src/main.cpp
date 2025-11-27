#include <Arduino.h>
#include <SensirionI2cStcc4.h>
#include <Wire.h>
#include <Preferences.h>

#include "FrcBleService.h"
#include "SettingsBleService.h"
#include "SensirionUptBleServer.h"

#include <Sensirion_UPT_Core.h>

#include "config.h"
#include "led_utils.h"

using namespace sensirion::upt;
static auto constexpr TAG = "MAIN";

SensirionI2cStcc4 stcc4;
LedUtils led;
Preferences persist; 

static int16_t error;

bool frcRequested = false;
int16_t stcc4_co2 = 0;
float stcc4_temp = 0.0;
float stcc4_humi = 0.0;
uint16_t stcc4_status = 0;

static int64_t lastMeasurementTimeMs = 0;

ble_server::NimBLELibraryWrapper lib;
ble_server::FrcBleService frcBleService(lib);
ble_server::SettingsBleService settingsBleService(lib);
ble_server::UptBleServer uptBleServer(lib, core::T_RH_CO2_ALT);

void frcRequestCallback(const uint16_t referenceCo2Level);
void nameChangeRequestCallback(const std::string& newName);

void setup()
{
#if SERIAL_DEBUG_ENABLE
  Serial.begin(SERIAL_BAUDRATE);
  while (!Serial)
    delay(500);
  ESP_LOGI(TAG, "Serial connected!");
#endif

  led.setToStaticWhite();

  Wire.begin(SDA_PIN, SCL_PIN);
  stcc4.begin(Wire, STCC4_I2C_ADDR_64);

#if SERIAL_DEBUG_ENABLE
  ESP_LOGI(TAG, "Stopping measurement...");
#endif
  error = stcc4.stopContinuousMeasurement();
  if (error != NO_ERROR)
  {
    led.blinkRed();
#if SERIAL_DEBUG_ENABLE
    ESP_LOGE(TAG, "Error while calling stopContinuousMeasurement.");
#endif
    return;
  }

  led.blinkGreen();

#if SERIAL_DEBUG_ENABLE
  ESP_LOGI(TAG, "Request conditioning...");
#endif
  error = stcc4.performConditioning();
  if (error != NO_ERROR)
  {
    led.blinkRed();
#if SERIAL_DEBUG_ENABLE
    ESP_LOGE(TAG, "Error while calling performConditioning.");
#endif
    return;
  }
  led.blinkGreen();

#if SERIAL_DEBUG_ENABLE
  ESP_LOGI(TAG, "Starting measurement...");
#endif
  error = stcc4.startContinuousMeasurement();
  if (error != NO_ERROR)
  {
    led.blinkRed();
#if SERIAL_DEBUG_ENABLE
    ESP_LOGE(TAG, "Error while calling startContinuousMeasurement.");
#endif
    return;
  }
  led.blinkGreen();

  // Check for persisted ble gadget name
  persist.begin("ble-settings", false);
  auto name = persist.getString("alt-device-name", BLE_DEFAULT_DEVICE_NAME);

#if SERIAL_DEBUG_ENABLE
  ESP_LOGI(TAG, "Starting BleServer...");
#endif
  frcBleService.registerFrcRequestCallback(frcRequestCallback);
  settingsBleService.setEnableAltDeviceName(true);
  settingsBleService.setEnableWifiSettings(false);
  settingsBleService.setAltDeviceName(name.c_str());
  settingsBleService.registerDeviceNameChangeCallback(nameChangeRequestCallback);
  uptBleServer.registerBleServiceProvider(frcBleService);
  uptBleServer.registerBleServiceProvider(settingsBleService);
  uptBleServer.begin();

  led.blinkGreen();
#if SERIAL_DEBUG_ENABLE
  ESP_LOGI(TAG, "Setup done.");
#endif

  // Wait for first measurement
  delay(1000);
}

void loop()
{
  // Read measurement every STCC4_MEASUREMENT_INTERVAL_MS when no FRC is ongoing
  if (millis() - lastMeasurementTimeMs >= STCC4_MEASUREMENT_INTERVAL_MS &&
      !frcRequested)
  {

    error = stcc4.readMeasurement(stcc4_co2, stcc4_temp, stcc4_humi, stcc4_status);
    if (error != NO_ERROR)
    {
      // Clock could have stretched, wait and try again
#if SERIAL_DEBUG_ENABLE
      char errormessage[128];
      errorToString(error, errormessage, sizeof(errormessage));
      ESP_LOGE(TAG, "Error while calling readMeasurement. error code: %d, message:\n%s", error, errormessage);
#endif
      delay(100);
      return;
    }

    lastMeasurementTimeMs = millis();

    uptBleServer.writeValueToCurrentSample(
        stcc4_temp, core::SignalType::TEMPERATURE_DEGREES_CELSIUS);
    uptBleServer.writeValueToCurrentSample(
        stcc4_humi, core::SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
    uptBleServer.writeValueToCurrentSample(
        stcc4_co2, core::SignalType::CO2_PARTS_PER_MILLION);
    uptBleServer.commitSample();

    led.setColorFromCo2(stcc4_co2);
  }

  // handle download requests
  uptBleServer.handleDownload();

  // light sleep to save power
  // esp_sleep_enable_timer_wakeup(400 * 1000);
  // esp_light_sleep_start();
  delay(400);
}

void frcRequestCallback(const uint16_t referenceCo2Level)
{
  if (!frcRequested)
  {
#if SERIAL_DEBUG_ENABLE
    ESP_LOGI(TAG, "FRC requested...");
#endif
    frcRequested = true;
    int16_t correction;
    stcc4.performForcedRecalibration(referenceCo2Level, correction);
    frcRequested = false;
#if SERIAL_DEBUG_ENABLE
    ESP_LOGI(TAG, "FRC completed with correction value: %d", correction);
#endif
  }
}

void nameChangeRequestCallback(const std::string& newName){
#if SERIAL_DEBUG_ENABLE
  ESP_LOGI(TAG, "Device name change requested: %s. Persisting new name...", newName.c_str());
#endif
  persist.putString("alt-device-name", newName.c_str());
}