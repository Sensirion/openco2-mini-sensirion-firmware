#ifndef LED_UTILS_H
#define LED_UTILS_H

#include <Adafruit_NeoPixel.h>

// Color definitions used by the LedUtils
const uint32_t LED_COLOR_OFF = Adafruit_NeoPixel::Color(0, 0, 0);
const uint32_t LED_COLOR_WHITE = Adafruit_NeoPixel::Color(255, 255, 255);
const uint32_t LED_COLOR_BLUE = Adafruit_NeoPixel::Color(0, 0, 255);

const uint32_t LED_COLOR_GREEN = Adafruit_NeoPixel::Color(0, 255, 0);
const uint32_t LED_COLOR_ORANGE = Adafruit_NeoPixel::Color(255, 120, 0);
const uint32_t LED_COLOR_RED = Adafruit_NeoPixel::Color(255, 0, 0);

/**
 * Helper for controlling the single on‑board NeoPixel LED.
 *
 * Notes
 * - Call begin() once before using any other method.
 * - Public blink methods are synchronous and block for a short time
 *   (they use delay()), but are protected by a mutex to avoid clashes
 *   with concurrent updates.
 */
class LedUtils {
public:
  /**
   * Construct the helper. Resources are allocated, but the LED is not
   * initialized yet. Call begin() before first use.
   */
  LedUtils();

  /** Destructor. Releases internal synchronization resources. */
  ~LedUtils();

  /**
   * Initialize the NeoPixel driver and set the LED to static white
   * with the configured brightness.
   */
  void begin();

  /**
   * Set the base LED color derived from a CO2 value.
   * - co2 > CO2_THRESHOLD_DANGER  -> red
   * - co2 > CO2_THRESHOLD_WARN    -> orange
   * - otherwise                   -> green
   * Applies the color immediately using the static brightness.
   */
  void setColorFromCo2(uint16_t co2);

  /** Set the LED to static blue (applied immediately). */
  void setToStaticBlue();

  /** Set the LED to static white (applied immediately). */
  void setToStaticWhite();

  /**
   * Blink the LED in red three times, then restore the previous base color.
   * Blocking: yes (brief). Thread-safe via internal mutex.
   */
  void blinkRed();

  /**
   * Blink the LED in green three times, then restore the previous base color.
   * Blocking: yes (brief). Thread-safe via internal mutex.
   */
  void blinkGreen();

  /**
   * Blink the LED in blue three times, then restore the previous base color.
   * Blocking: yes (brief). Thread-safe via internal mutex.
   */
  void blinkBlue();

private:
  Adafruit_NeoPixel mLed;
  uint8_t mStaticBrightness;
  uint32_t mBaseColor;
  SemaphoreHandle_t mLedSemaphore;

  void mApplyColor();
  void mBlink(uint32_t blinkColor);
};

#endif // LED_UTILS_H
