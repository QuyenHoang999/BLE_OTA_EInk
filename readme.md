# BLE_OTA_EInk

Bluetooth SoC application for an EFR32MG24 target with application-level OTA firmware update support. The app starts BLE legacy advertising after boot, accepts connections, receives OTA image data through the Silicon Labs OTA GATT service, verifies the downloaded image in bootloader storage slot 0, and installs the verified image after the connection closes.

The current GATT device name is `Empty Example` in `config/btconf/gatt_configuration.btconf`.

## Project Layout

| File | Purpose |
| --- | --- |
| `app.c` | Core Bluetooth event handling and advertising lifecycle. |
| `ota.c` | OTA control/data write handling, bootloader storage access, image verification, and install trigger. |
| `ota.h` | Public OTA interface used by `app.c`. |
| `config/btconf/gatt_configuration.btconf` | Generic Access and Device Information GATT configuration. |
| `config/btconf/app_ota_dfu.xml` | Silicon Labs OTA service definition added by the Application OTA DFU component. |
| `BLE_OTA_EInk.slcp` | Simplicity Studio project/component definition. |

Generated build output and the copied SDK directory are intentionally ignored by Git:

```gitignore
GNU ARM v12.2.1 - Default/
simplicity_sdk_2025.6.2/
```

## Hardware And SDK

- Silicon Labs EFR32MG24B310F1536IM48
- BRD2601B board support
- Simplicity SDK 2025.6.2
- GNU ARM toolchain v12.2.1
- Micrium OS kernel
- Bluetooth stack with GATT server, legacy advertiser, central/peripheral roles, and Application OTA DFU

## Runtime Behavior

1. On `sl_bt_evt_system_boot_id`, the app creates an advertising set, initializes the bootloader interface through `ota_init()`, and starts connectable legacy advertising.
2. On connection close, the app checks whether OTA verification completed successfully.
3. If a verified image is ready, the app marks storage slot 0 as the bootload image and reboots into install.
4. Otherwise, advertising restarts.
5. OTA GATT write requests are delegated from `app.c` to `ota_handle_user_write_request()`.
6. OTA verification continuation is driven by Bluetooth external signals and handled in `ota.c`.

## OTA Flow

The project uses the Silicon Labs OTA service:

- Service: `Silicon Labs OTA`
- Control characteristic: `gattdb_ota_control`
- Data characteristic: `gattdb_ota_data`
- Firmware start command: `0x00`
- Firmware end command: `0x03`
- Storage slot: `0`

When the control characteristic receives the start command, `ota.c` resets the write position, reads bootloader slot information, and erases slot 0 if needed. Data characteristic writes are appended to slot 0. When the end command arrives, the image is verified asynchronously. A successful verification closes the BLE connection; the disconnect event then starts the bootloader install.

## Build And Flash

1. Open `BLE_OTA_EInk.slcp` in Simplicity Studio.
2. Let Simplicity Studio generate/update project files if prompted.
3. Build the project with the `GNU ARM v12.2.1 - Default` configuration.
4. Flash the generated `.hex` or `.s37` image to the target.

Do not flash the raw `.bin` file unless you know the flash layout; it can overwrite bootloader regions.

## Bootloader Requirement

This project calls Gecko Bootloader APIs and expects a bootloader with storage slot support to already be installed on the device. For a Series 2 SoC target, use an OTA-capable Gecko Bootloader configuration that provides storage slot 0.

If OTA fails at startup with slot information errors, check that:

- The correct Gecko Bootloader image is flashed.
- Bootloader storage slot 0 exists and is large enough for the application image.
- The application was flashed as `.hex` or `.s37`, not as a raw binary over the bootloader area.

## Testing With Simplicity Connect

1. Flash a compatible Gecko Bootloader.
2. Build and flash this application.
3. Open Simplicity Connect on a phone or desktop.
4. Scan for the device name `Empty Example`.
5. Connect and confirm the Generic Access, Device Information, and Silicon Labs OTA services are visible.
6. Start an OTA update using a compatible signed/packaged application image for the installed bootloader.

## Notes

- E-Ink display application behavior is not implemented in the current `app.c`; the current project is focused on BLE advertising and OTA update plumbing.
- Keep application logic in `app.c` or separate modules, and keep OTA-specific bootloader/GATT write handling in `ota.c`.
