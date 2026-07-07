/***************************************************************************//**
 * @file
 * @brief OTA update interface.
 *******************************************************************************
 * # License
 * <b>Copyright 2024 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 ******************************************************************************/

#ifndef OTA_H
#define OTA_H

#include "sl_bt_api.h"

#include <stdbool.h>
#include <stdint.h>

void ota_init(void);
void ota_handle_user_write_request(sl_bt_msg_t *evt);
void ota_handle_external_signal(uint32_t extsignals);
bool ota_is_image_ready_to_install(void);
void ota_install_image(void);

#endif // OTA_H
