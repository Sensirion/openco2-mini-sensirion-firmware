#include "BleConnectService.h"

namespace sensirion::upt::ble_server {

bool BleConnectService::begin() {
  return true;
}

void BleConnectService::onConnect() {
  mLed.blinkBlue();
}

} // namespace sensirion::upt::ble_server