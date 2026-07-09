/***************************************************************************//**
 * @file
 * @brief SD card initialization interface.
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 ******************************************************************************/

#ifndef SD_CARD_H
#define SD_CARD_H

/***************************************************************************//**
 * Initialize the SD card SPI interface and exercise the mounted filesystem.
 ******************************************************************************/
void sd_card_init(void);

#endif // SD_CARD_H
