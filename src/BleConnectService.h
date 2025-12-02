#ifndef ARDUINO_UPT_BLE_SERVER_BLE_CONNECT_SERVICE_H
#define ARDUINO_UPT_BLE_SERVER_BLE_CONNECT_SERVICE_H

#include "IBleServiceProvider.h"
#include "led_utils.h"

namespace sensirion::upt::ble_server {

class BleConnectService final : public IBleServiceProvider {
private:
  LedUtils &mLed;

public:
  explicit BleConnectService(IBleServiceLibrary &bleLibrary, LedUtils &led)
      : IBleServiceProvider(bleLibrary), mLed(led) {}

  bool begin() override;
  void onConnect() override;
};

} // namespace sensirion::upt::ble_server

#endif // ARDUINO_UPT_BLE_SERVER_BLE_CONNECT_SERVICE_H