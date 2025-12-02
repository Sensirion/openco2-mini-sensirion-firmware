#include <Preferences.h>
#include <SensirionI2cStcc4.h>

#include "BleConnectService.h"
#include "FrcBleService.h"
#include "SensirionUptBleServer.h"
#include "SettingsBleService.h"
#include "config.h"
#include "led_utils.h"

using namespace sensirion::upt;
static auto constexpr TAG = "MAIN";

SensirionI2cStcc4 stcc4;
Preferences persist;
LedUtils led;

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
ble_server::BleConnectService bleConnectService(lib, led);

void frcRequestCallback(const uint16_t referenceCo2Level);
void nameChangeRequestCallback(const std::string &newName);

void setup() {
  int16_t error;

  Wire.begin(SDA_PIN, SCL_PIN);
  stcc4.begin(Wire, STCC4_I2C_ADDR_64);
  persist.begin("ble-settings", false);
  led.begin();

  String name = persist.getString("alt-device-name", BLE_DEFAULT_DEVICE_NAME);

  ESP_LOGI(TAG, "Stopping measurement...");
  error = stcc4.stopContinuousMeasurement();
  if (error != NO_ERROR) {
    led.blinkRed();
    ESP_LOGE(TAG, "Error while calling stopContinuousMeasurement.");
    return;
  }

  ESP_LOGI(TAG, "Request conditioning...");
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

  ESP_LOGI(TAG, "Starting BleServer...");
  frcBleService.registerFrcRequestCallback(frcRequestCallback);
  settingsBleService.setAltDeviceName(name.c_str());
  settingsBleService.setEnableWifiSettings(false);
  settingsBleService.registerDeviceNameChangeCallback(
      nameChangeRequestCallback);
  uptBleServer.registerBleServiceProvider(frcBleService);
  uptBleServer.registerBleServiceProvider(settingsBleService);
  uptBleServer.registerBleServiceProvider(bleConnectService);
  uptBleServer.begin();

  delay(1000);

  ESP_LOGI(TAG, "Setup done. Device is advertised with name = %s",
           name.c_str());
}

void loop() {
  if (millis() - lastMeasurementTimeMs >= STCC4_MEASUREMENT_INTERVAL_MS && !frcRequested) {
    int16_t error =
        stcc4.readMeasurement(stcc4_co2, stcc4_temp, stcc4_humi, stcc4_status);
    if (error != NO_ERROR) {
      char errormessage[128];
      errorToString(error, errormessage, sizeof(errormessage));
      ESP_LOGE(
          TAG,
          "Error while calling readMeasurement. error code: %d, message:\n%s",
          error, errormessage);
      delay(100);
    }

    uptBleServer.writeValueToCurrentSample(
        stcc4_temp, core::SignalType::TEMPERATURE_DEGREES_CELSIUS);
    uptBleServer.writeValueToCurrentSample(
        stcc4_humi, core::SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
    uptBleServer.writeValueToCurrentSample(
        stcc4_co2, core::SignalType::CO2_PARTS_PER_MILLION);
    uptBleServer.commitSample();
    lastMeasurementTimeMs = millis();
    led.setColorFromCo2(stcc4_co2);
  }

  // handle download requests
  uptBleServer.handleDownload();

  delay(20);
}

void frcRequestCallback(const uint16_t referenceCo2Level) {
  if (frcRequested)
    return;

  ESP_LOGI(TAG, "FRC requested...");
  frcRequested = true;
  int16_t correction;
  stcc4.performForcedRecalibration(referenceCo2Level, correction);
  frcRequested = false;
  ESP_LOGI(TAG, "FRC completed with correction value: %d", correction);
}

void nameChangeRequestCallback(const std::string &newName) {
  ESP_LOGI(TAG, "Device name change requested: %s. Persisting new name...",
           newName.c_str());

  persist.putString("alt-device-name", newName.c_str());
}
