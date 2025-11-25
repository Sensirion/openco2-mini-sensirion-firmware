#include "led_utils.h"
#include "config.h"

LedUtils::LedUtils(){
    led = Adafruit_NeoPixel(1, LED_PIN, NEO_GRB + NEO_KHZ800);
    static_brightness = (uint8_t)(255 * LED_BRIGHTNESS_PERCENT / 100);
}
 
void LedUtils::setColorFromCo2(uint16_t co2) {
    uint16_t r, g, b;
  if (co2 > CO2_THRES_DANGER) {
    // Set LED to red
    r = 255;
    g = 0;
    b = 0;
  } else if (co2 > CO2_THRES_WARN){
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
  baseColor = led.Color(r, g, b);
  mApplyColor();
}

void LedUtils::mApplyColor(){
  led.setPixelColor(0, baseColor);
  led.setBrightness(static_brightness);
  led.show();
}

void LedUtils::mBlink(uint32_t blinkColor){
 // OFF, COLOR, OFF, BASE_COLOR
  delay(LED_BLINK_DELAY_MS);
  led.setBrightness(255);
  led.setPixelColor(0, blinkColor);
  led.show();
  delay(LED_BLINK_DELAY_MS);
  led.setBrightness(static_brightness);
  led.setPixelColor(0, baseColor);
  led.show();
}

void LedUtils::blinkBlue() {
    mBlink(led.Color(0,0,255));
}

void LedUtils::blinkRed() {
    mBlink(led.Color(255,0,0));
}

void LedUtils::blinkGreen() {
    mBlink(led.Color(0,255,0));
}

void LedUtils::setToStaticBlue(){
    baseColor = led.Color(0,0,255);
    mApplyColor();
}

void LedUtils::setToStaticWhite(){
    baseColor = led.Color(255,255,255);
    mApplyColor();
}