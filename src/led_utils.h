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

    private:
        Adafruit_NeoPixel led;
        uint8_t static_brightness;
        uint32_t baseColor;

        void mApplyColor();
        void mBlink(uint32_t blinkColor);
};
