#ifndef LED_UTILS_H
#define LED_UTILS_H

#include <Adafruit_NeoPixel.h>

class LedUtils{
    public:
        LedUtils();
        void setColorFromCo2(uint16_t co2);
        void setToStaticBlue();
        void setToStaticWhite();
        void blinkRed();
        void blinkGreen();
        void blinkBlue();
        void begin();

    private:
        Adafruit_NeoPixel mLed;
        uint8_t mStaticBrightness;
        uint32_t mBaseColor;

        void mApplyColor();
        void mBlink(uint32_t blinkColor);
};

#endif // LED_UTILS_H
