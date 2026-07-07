# Release Notes

## Unreleased - 2026-07-07

### Added

- Added application-level OTA firmware update flow using the Silicon Labs OTA GATT service.
- Added `ota.c` and `ota.h` to keep OTA state, bootloader storage access, image verification, and install logic separate from the main Bluetooth application.
- Added `.gitignore` entries for generated build output and the copied Simplicity SDK directory:
  - `GNU ARM v12.2.1 - Default/`
  - `simplicity_sdk_2025.6.2/`
- Updated project README to describe the actual `BLE_OTA_EInk` project instead of the stock SoC Empty template.
- Updated the GATT Device Name to `BLE OTA EInk`.

### Changed

- Simplified `app.c` so it owns BLE advertising and event delegation, while OTA-specific behavior is handled by `ota.c`.
- Registered `ota.c` and `ota.h` in `BLE_OTA_EInk.slcp`.

### Known Limitations

- E-Ink display application behavior is not implemented yet.
- A compatible Gecko Bootloader with storage slot 0 must be flashed before OTA updates can work.
