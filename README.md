# OpenCO2 Mini Sensirion firmware
An alternative firmware for the [OpenCO2 Mini](https://github.com/davidkreidler/OpenCO2_Mini) by [@davidkreidler](https://github.com/davidkreidler).
This firmware has a more restricted feature-set than the original firmware, the focus is given on performance and stability.

## Features
- Connectivity with MyAmbience mobile app:
    - Live data
    - Data download
    - FRC support
- LED colors synced with MyAmbience alerts.

## Built using Sensirion libraries
The firmware is built arround the official [STCC4 driver](https://github.com/Sensirion/arduino-i2c-stcc4) and the "[UPT ble server](https://github.com/Sensirion/arduino-upt-ble-server)" library.

## Development
Development is done using PlatformIO.