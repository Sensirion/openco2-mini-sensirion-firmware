#include "BleConnectService.h"

namespace sensirion::upt::ble_server {

static auto constexpr TAG = "BleConnectService";

bool BleConnectService::begin() { return true; }

void BleConnectService::onConnect() {
  mLed.blinkBlue();
  ESP_LOGD(TAG, "Device connected...");
}

void BleConnectService::onDisconnect() {
  ESP_LOGD(TAG, "Device disconnected...");
}

} // namespace sensirion::upt::ble_server