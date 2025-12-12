#include "LedUtils.h"
#include "config.h"

LedUtils::LedUtils()
    : mLed(1, LED_PIN, NEO_GRB + NEO_KHZ800),
      mBaseColor(Adafruit_NeoPixel::Color(255, 255, 255)) {
  mStaticBrightness = static_cast<uint8_t>(255 * LED_BRIGHTNESS_PERCENT / 100);
  mLedSemaphore = xSemaphoreCreateMutex();
}

LedUtils::~LedUtils() { vSemaphoreDelete(mLedSemaphore); }

void LedUtils::begin() {
  mLed.begin();
  setToStaticWhite();
}

void LedUtils::setColorFromCo2(const uint16_t co2) {
  if (co2 > CO2_THRESHOLD_DANGER) {
    mBaseColor = LED_COLOR_RED;
  } else if (co2 > CO2_THRESHOLD_WARN) {
    mBaseColor = LED_COLOR_ORANGE;
  } else {
    mBaseColor = LED_COLOR_GREEN;
  }
  mApplyColor();
}

void LedUtils::mApplyColor() {
  if (xSemaphoreTake(mLedSemaphore, 10) != pdTRUE) {
    return;
  }
  mLed.setPixelColor(0, mBaseColor);
  mLed.setBrightness(mStaticBrightness);
  mLed.show();
  xSemaphoreGive(mLedSemaphore);
}

void LedUtils::mBlink(const uint32_t blinkColor) {
  xSemaphoreTake(mLedSemaphore, portMAX_DELAY);
  // OFF, COLOR, OFF, BASE_COLOR
  mLed.setBrightness(255);
  for (int i = 0; i < 3; i++) {
    mLed.setPixelColor(0, blinkColor);
    mLed.show();
    delay(LED_BLINK_DELAY_MS);
    mLed.setPixelColor(0, LED_COLOR_OFF);
    mLed.show();
    delay(LED_BLINK_DELAY_MS);
  }
  mLed.setBrightness(mStaticBrightness);
  mLed.setPixelColor(0, mBaseColor);
  mLed.show();
  xSemaphoreGive(mLedSemaphore);
}

void LedUtils::blinkBlue() { mBlink(LED_COLOR_BLUE); }

void LedUtils::blinkRed() { mBlink(LED_COLOR_RED); }

void LedUtils::blinkGreen() { mBlink(LED_COLOR_GREEN); }

void LedUtils::setToStaticBlue() {
  mBaseColor = LED_COLOR_BLUE;
  mApplyColor();
}

void LedUtils::setToStaticWhite() {
  mBaseColor = LED_COLOR_WHITE;
  mApplyColor();
}