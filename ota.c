/***************************************************************************//**
 * @file
 * @brief OTA update logic.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 ******************************************************************************/

#include "ota.h"

#include "app_log.h"
#include "btl_interface.h"
#include "btl_interface_storage.h"
#include "gatt_db.h"
#include "sl_bluetooth.h"

#include <stdbool.h>
#include <stdint.h>

#define OTA_FIRMWARE_START 0x00
#define OTA_FIRMWARE_END   0x03

#define OTA_EXT_SIGNAL_VERIFY (1U << 0U)

static uint8_t connection = 0xff;

/* Flag for indicating DFU Reset must be performed */
static BootloaderInformation_t bldInfo;
static BootloaderStorageSlot_t slotInfo;

/* OTA variables */
static uint32_t ota_image_position = 0;
static uint8_t ota_in_progress = 0;
static uint8_t ota_image_finished = 0;
static uint8_t ota_verify_in_progress = 0;

static uint8_t ota_verify_context[BOOTLOADER_STORAGE_VERIFICATION_CONTEXT_SIZE] __attribute__((aligned(4)));

static int32_t get_slot_info(void);
static void erase_slot_if_needed(void);
static void verify_application_start(void);
static void verify_application_continue(void);

void ota_init(void)
{
  /* bootloader init must be called before calling other bootloader_xxx API */
  bootloader_init();
}

void ota_handle_user_write_request(sl_bt_msg_t *evt)
{
  connection = evt->data.evt_gatt_server_user_write_request.connection;

  if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_control) {
    switch (evt->data.evt_gatt_server_user_write_request.value.data[0]) {
      case OTA_FIRMWARE_START:
        ota_image_position = 0;
        ota_in_progress = 1;

        if (get_slot_info() == BOOTLOADER_OK) {
          erase_slot_if_needed();
        } else {
          app_log("Check that you have installed correct type of Gecko bootloader!\r\n");
        }

        app_log("upload started\r\n");
        break;

      case OTA_FIRMWARE_END:
        ota_in_progress = 0;
        ota_image_finished = 1;
        app_log("upload finished. received file size %lu bytes\r\n",
                ota_image_position);
        verify_application_start();
        break;

      default:
        break;
    }
  } else if (evt->data.evt_gatt_server_user_write_request.characteristic == gattdb_ota_data) {
    if (ota_in_progress) {
      bootloader_writeStorage(
        0,
        ota_image_position,
        evt->data.evt_gatt_server_user_write_request.value.data,
        evt->data.evt_gatt_server_user_write_request.value.len);
      ota_image_position += evt->data.evt_gatt_server_user_write_request.value.len;
    }
  }

  sl_bt_gatt_server_send_user_write_response(
    connection,
    evt->data.evt_gatt_server_user_write_request.characteristic,
    0);
}

void ota_handle_external_signal(uint32_t extsignals)
{
  if ((extsignals & OTA_EXT_SIGNAL_VERIFY) != 0) {
    verify_application_continue();
  }
}

bool ota_is_image_ready_to_install(void)
{
  return ota_image_finished != 0;
}

void ota_install_image(void)
{
  bootloader_setImageToBootload(0);
  bootloader_rebootAndInstall();
}

static void verify_application_continue(void)
{
  if (!ota_verify_in_progress) {
    return;
  }

  int32_t result = bootloader_continueVerifyImage(ota_verify_context, NULL);

  if (result == BOOTLOADER_ERROR_PARSE_CONTINUE) {
    sl_status_t sc = sl_bt_external_signal(OTA_EXT_SIGNAL_VERIFY);
    if (sc != SL_STATUS_OK) {
      ota_verify_in_progress = false;
      ota_image_finished = 0;
      app_log("external OTA verify signal failed. err: %lx \r\n", sc);
    }
    return;
  }

  ota_verify_in_progress = false;

  if (result == BOOTLOADER_ERROR_PARSE_SUCCESS) {
    ota_image_finished = 1;
    app_log("application verification successful. \r\n");

    sl_status_t sc = sl_bt_connection_close(connection);
    if (sc != SL_STATUS_OK) {
      app_log("connection close failed after verification. err: %lx \r\n", sc);
    }
  } else {
    ota_image_finished = 0;
    app_log("application verification failed. err: %lx \r\n", result);
  }
}

static int32_t get_slot_info(void)
{
  int32_t err;

  bootloader_getInfo(&bldInfo);
  app_log("Gecko bootloader version: %lu.%lu\r\n",
          (bldInfo.version & 0xFF000000) >> 24,
          (bldInfo.version & 0x00FF0000) >> 16);

  err = bootloader_getStorageSlotInfo(0, &slotInfo);

  if (err == BOOTLOADER_OK) {
    app_log("Slot 0 starts @ 0x%8.8lx, size %lu bytes\r\n",
            slotInfo.address,
            slotInfo.length);
  } else {
    app_log("Unable to get storage slot info, error %lx\r\n", err);
  }

  return err;
}

static void erase_slot_if_needed(void)
{
  uint32_t offset = 0;
  uint32_t num_blocks = 0;
  uint32_t i = 0;
  uint8_t buffer[256];
  bool dirty = false;
  int32_t err = BOOTLOADER_OK;

  num_blocks = slotInfo.length / 256;

  while (!dirty && (offset < 256 * num_blocks) && (err == BOOTLOADER_OK)) {
    err = bootloader_readStorage(0, offset, buffer, 256);
    if (err == BOOTLOADER_OK) {
      i = 0;
      while (i < 256) {
        if (buffer[i++] != 0xFF) {
          dirty = true;
          break;
        }
      }
      offset += 256;
    }
  }

  if (err != BOOTLOADER_OK) {
    app_log("error reading flash! %lx\r\n", err);
  } else if (dirty) {
    app_log("download area is not empty, erasing...\r\n");
    bootloader_eraseStorageSlot(0);
    app_log("done\r\n");
  } else {
    app_log("download area is empty\r\n");
  }
}

static void verify_application_start(void)
{
  int32_t result = bootloader_initVerifyImage(0, ota_verify_context, sizeof(ota_verify_context));

  if (result != BOOTLOADER_OK) {
    ota_verify_in_progress = false;
    ota_image_finished = 0;
    app_log("application verification failed. err: %lx \r\n", result);
    return;
  }

  ota_verify_in_progress = true;

  sl_status_t sc = sl_bt_external_signal(OTA_EXT_SIGNAL_VERIFY);
  if (sc != SL_STATUS_OK) {
    ota_verify_in_progress = false;
    ota_image_finished = 0;
    app_log("external OTAverify signal failed. err: %lx \r\n", sc);
  }
}
