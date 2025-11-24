#include <Arduino.h>
#include <SensirionI2cStcc4.h>
#include <Wire.h>

#include "FrcBleService.h"
#include "SensirionUptBleServer.h"

#include <Sensirion_UPT_Core.h>

// macro definitions
// make sure that we use the proper definition of NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

using namespace sensirion::upt;

SensirionI2cStcc4 stcc4;

ble_server::NimBLELibraryWrapper lib;
ble_server::FrcBleService frcBleService(lib);
ble_server::UptBleServer uptBleServer(lib, core::T_RH_CO2_ALT);

static int16_t error;

bool frcRequested = false;
int16_t stcc4_co2 = 0;
float stcc4_temp = 0.0;
float stcc4_humi = 0.0;
uint16_t stcc4_status = 0;
    

static int64_t lastMeasurementTimeMs = 0;
static int measurementIntervalMs = 1000;

void frcRequestCallback(const uint16_t referenceCo2Level) {
  if(!frcRequested){
    frcRequested = true;
    int16_t correction;
    stcc4.performForcedRecalibration(referenceCo2Level, correction);
    frcRequested = false;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // Wait for Serial monitor to start

  Wire.begin();
  stcc4.begin(Wire, STCC4_I2C_ADDR_64);

  error = stcc4.stopContinuousMeasurement();
  if (error != NO_ERROR) {
      // Blink red
      return;
  }

  error = stcc4.startContinuousMeasurement();
  if (error != NO_ERROR) {
      // Blink red
      return;
  }

  frcBleService.registerFrcRequestCallback(frcRequestCallback);
  uptBleServer.registerBleServiceProvider(frcBleService);
  uptBleServer.begin();

  // Blink Blue
}

void loop() {
  delay(1000);

  if (millis() - lastMeasurementTimeMs >= measurementIntervalMs &&
      !frcRequested) {
    
    // Read measured values
    error = stcc4.readMeasurement(stcc4_co2, stcc4_temp, stcc4_humi, stcc4_status);
    lastMeasurementTimeMs = millis();

    uptBleServer.writeValueToCurrentSample(
         stcc4_temp, core::SignalType::TEMPERATURE_DEGREES_CELSIUS);
    uptBleServer.writeValueToCurrentSample(
         stcc4_humi, core::SignalType::RELATIVE_HUMIDITY_PERCENTAGE);
    uptBleServer.writeValueToCurrentSample(
         stcc4_co2, core::SignalType::CO2_PARTS_PER_MILLION);
    uptBleServer.commitSample();
    

    if (error != NO_ERROR) {
      // Blink red
    }

    // Update LED color
  }

  // handle download requests
  uptBleServer.handleDownload();

  delay(20);
}