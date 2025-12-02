#ifndef ARDUINO_UPT_BLE_SERVER_BLE_CONNECT_SERVICE_H
#define ARDUINO_UPT_BLE_SERVER_BLE_CONNECT_SERVICE_H

#include "IBleServiceProvider.h"
#include "LedUtils.h"

namespace sensirion::upt::ble_server {

/**
 * Small helper service that provides
 * simple visual feedback via the on-board LED.
 *
 * Behavior
 * - When a central connects, the service
 *   triggers a short blue blink on the LED 3 times.
 *
 * Notes
 * - This class implements the IBleServiceProvider callbacks used by the
 *   underlying Sensirion UPT BLE Server library.
 */
class BleConnectService final : public IBleServiceProvider {
public:
  /**
   * Construct the service.
   *
   * @param bleLibrary Reference to the BLE service library used for
   *                   registration and callback dispatching.
   * @param led        Reference to the LED helper used for visual feedback.
   */
  explicit BleConnectService(IBleServiceLibrary &bleLibrary, LedUtils &led)
      : IBleServiceProvider(bleLibrary), mLed(led) {}

  /**
   * Initialize the service. Intended to be called during system startup
   * after the BLE stack has been configured.
   *
   * @return true on success
   */
  bool begin() override;

  /** Callback invoked by the BLE stack when a device connects. */
  void onConnect() override;

  /** Callback invoked by the BLE stack when a device disconnects */
  void onDisconnect() override;

private:
  LedUtils &mLed;
};

} // namespace sensirion::upt::ble_server

#endif // ARDUINO_UPT_BLE_SERVER_BLE_CONNECT_SERVICE_H