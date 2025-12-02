#include "led_utils.h"
#include "config.h"

LedUtils::LedUtils() : mLed(1, LED_PIN, NEO_GRB + NEO_KHZ800) {
  mStaticBrightness = (uint8_t)(255 * LED_BRIGHTNESS_PERCENT / 100);
}

void LedUtils::begin() {
  mLed.begin();
  setToStaticWhite();
}

void LedUtils::setColorFromCo2(uint16_t co2) {
  uint16_t r, g, b;
  if (co2 > CO2_THRES_DANGER) {
    // Set LED to red
    r = 255;
    g = 0;
    b = 0;
  } else if (co2 > CO2_THRES_WARN) {
    // Set LED to orange
    r = 255;
    g = 120;
    b = 0;
  } else {
    // Set LED to green
    r = 0;
    g = 255;
    b = 0;
  }
  mBaseColor = mLed.Color(r, g, b);
  mApplyColor();
}

void LedUtils::mApplyColor() {
  mLed.setPixelColor(0, mBaseColor);
  mLed.setBrightness(mStaticBrightness);
  mLed.show();
}

void LedUtils::mBlink(uint32_t blinkColor) {
  // OFF, COLOR, OFF, BASE_COLOR
  mLed.setBrightness(255);
  for (int i = 0; i < 3; i ++) {
    mLed.setPixelColor(0, blinkColor);
    mLed.show();
    delay(LED_BLINK_DELAY_MS);
    mLed.setPixelColor(0, mLed.Color(0, 0, 0));
    mLed.show();
    delay(LED_BLINK_DELAY_MS);
  }
  mLed.setBrightness(mStaticBrightness);
  mLed.setPixelColor(0, mBaseColor);
  mLed.show();
}

void LedUtils::blinkBlue() { mBlink(mLed.Color(0, 0, 255)); }

void LedUtils::blinkRed() { mBlink(mLed.Color(255, 0, 0)); }

void LedUtils::blinkGreen() { mBlink(mLed.Color(0, 255, 0)); }

void LedUtils::setToStaticBlue() {
  mBaseColor = mLed.Color(0, 0, 255);
  mApplyColor();
}

void LedUtils::setToStaticWhite() {
  mBaseColor = mLed.Color(255, 255, 255);
  mApplyColor();
}