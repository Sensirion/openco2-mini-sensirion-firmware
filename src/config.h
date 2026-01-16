#pragma once

#define FIRMWARE_VERSION "1.0.0"

//  GPIO
#define BUTTON GPIO_NUM_0
#define SCL_PIN GPIO_NUM_13
#define SDA_PIN GPIO_NUM_12
#define LED_PIN GPIO_NUM_6

//  CO2 Thresholds for color orange and red
#define CO2_THRESHOLD_DANGER 1600
#define CO2_THRESHOLD_WARN 1000

// LED
#define LED_BRIGHTNESS_PERCENT 10
#define LED_BLINK_DELAY_MS 100

// MEASUREMENT
#define STCC4_MEASUREMENT_INTERVAL_MS 1100
#define STCC4_MEASURE_CHECK_MS 90

// COMPENSATION
#define T_COMP -11.5f
#define M 17.62f
#define TN 243.21f


// NO_ERROR
#ifdef NO_ERROR
#undef NO_ERROR
#endif
#define NO_ERROR 0

// BLE
#define BLE_DEFAULT_DEVICE_NAME "OpenCO2-Mini"