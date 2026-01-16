# OpenCO2 Mini Sensirion firmware
An alternative firmware for the [OpenCO2 Mini](https://github.com/davidkreidler/OpenCO2_Mini) by [@davidkreidler](https://github.com/davidkreidler).
This firmware has a more restricted feature-set than the original firmware, the focus is given on BLE connectivity and faster measurement rate (1Hz).

## Features
- Connectivity with MyAmbience mobile app:
    - Live data
    - Data download
    - FRC support
    - Custom name support
- LED colors synced with MyAmbience alerts.

## Built using Sensirion libraries
The firmware is built around the official [STCC4 driver](https://github.com/Sensirion/arduino-i2c-stcc4) and the "[UPT BLE server](https://github.com/Sensirion/arduino-upt-ble-server)" library.

## Development
Development is done using PlatformIO.

### Prerequisites
- Install PlatformIO
  - VS Code extension: install "PlatformIO IDE" from the Extensions marketplace.
  - or CLI: `pip install -U platformio` (requires Python 3.8+).

### Build (compile)
- VS Code: Open this folder, then in the PlatformIO panel select the `openco2-mini` environment and click "Build".
- CLI:
  - From the project root run:
    - `pio run -e openco2-mini`

### Uploading firmware to OpenCO2 Mini (ESP32-S3)
The OpenCO2 Mini must be in bootloader mode to accept a new firmware.

1. Prepare the device
   - Hold down the button on the OpenCO2 Mini.
   - While holding the button, plug the device into a USB‑C port of your computer.
   - Release the button after it is connected. This puts the device into download (bootloader) mode.
2. Upload
   - VS Code: In PlatformIO, select the `openco2-mini` environment and click "Upload".
   - CLI: `pio run -e openco2-mini -t upload`
3. Reset after upload
   - After a successful upload, trigger a reset by unplugging the device and plugging it in again.

Notes
- The serial monitor (115200 baud) can be opened via PlatformIO after the device has rebooted.
- If upload fails, retry the bootloader sequence (hold button → plug in → release) and run the upload again.