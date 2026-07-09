/***************************************************************************//**
 * @file
 * @brief SD card initialization and FatFs smoke test.
 *******************************************************************************
 * # License
 * <b>Copyright 2026 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 ******************************************************************************/

#include "sd_card.h"

#include "app_assert.h"
#include "app_log.h"
#include "diskio.h"
#include "ff.h"
#include "sl_sdc_sd_card.h"
#include "sl_sleeptimer.h"

#if defined(SLI_SI917)
#include "sl_si91x_gspi.h"

static sl_gspi_instance_t gspi_instance = SL_GSPI_MASTER;
#else
#include "sl_gpio.h"
#include "sl_spidrv_instances.h"
#endif

#define sd_card_printf(...) app_log(__VA_ARGS__)

static mikroe_spi_handle_t sd_card_spi_instance = NULL;

static const char sd_card_test_string[] = "Silabs SD Card I/O Example via SPI!\r\n";
static const char *const fat_type_names[] = { "", "FAT12", "FAT16", "FAT32", "exFAT" };

static BYTE sd_card_work_area[FF_MAX_SS];

static void assert_fatfs_ok(FRESULT result)
{
  app_assert(result == FR_OK, "FatFs error: %d", (int)result);
}

#if !FF_FS_NORTC && !FF_FS_READONLY
static void sd_card_init_fatfs_time(void)
{
  DWORD time_data;
  sl_status_t sc;
  sl_sleeptimer_date_t date = {
    .year = 122,
    .month = 2,
    .month_day = 1,
    .hour = 10,
    .min = 30,
    .sec = 0,
  };

  sc = sl_sleeptimer_set_datetime(&date);
  app_assert_status(sc);

  time_data = get_fattime();
  sd_card_printf("\nCurrent time is %lu/%lu/%lu %2lu:%02lu:%02lu.\n\n",
                 (time_data >> 25) + 1980,
                 (time_data >> 21) & 0x0f,
                 (time_data >> 16) & 0x1f,
                 (time_data >> 11) & 0x1f,
                 (time_data >> 5) & 0x3f,
                 (time_data << 1) & 0x1f);
}
#else
static void sd_card_init_fatfs_time(void)
{
}
#endif

static void sd_card_init_spi(void)
{
  sl_status_t sc;

#if defined(SLI_SI917)
  sd_card_spi_instance = &gspi_instance;
#else
  sd_card_spi_instance = sl_spidrv_mikroe_handle;

  /* The MISO pin is driven by a tri-stated output from the SD card, so hold
   * it in a known state while the card is not selected.
   */
  sl_gpio_t pinport = { sl_spidrv_mikroe_handle->initData.portRx,
                        sl_spidrv_mikroe_handle->initData.pinRx };
  sl_gpio_set_pin_mode(&pinport, SL_GPIO_MODE_INPUT_PULL, 1);
#endif

  sc = sd_card_spi_init(sd_card_spi_instance);
  app_assert_status(sc);
}

static void sd_card_mount(FATFS *fs)
{
  FRESULT ret_code;

  // Give a work area to the default drive.
  ret_code = f_mount(fs, "", 0);
  assert_fatfs_ok(ret_code);
}

static void sd_card_print_volume_status(void)
{
  FATFS *pfs;
  DWORD fre_clust;
  FRESULT ret_code;

  // Show logical drive status.
  ret_code = f_getfree("", &fre_clust, &pfs);
  assert_fatfs_ok(ret_code);
  sd_card_printf("-------------- Volume status --------------\n\r");
  sd_card_printf(("FAT type = %s\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
                  "Root DIR entries = %u\nSectors/FAT = %lu\n"
                  "Number of clusters = %lu\nVolume start (lba) = %lu\n"
                  "FAT start (lba) = %lu\nDIR start (lba,cluster) = %lu\n"
                  "Data start (lba) = %lu\n%lu KiB total disk space.\n"
                  "%lu KiB available.\n\n"),
                 fat_type_names[pfs->fs_type],
                 (DWORD)pfs->csize * 512,
                 pfs->n_fats,
                 pfs->n_rootdir,
                 pfs->fsize,
                 pfs->n_fatent - 2,
                 (DWORD)pfs->volbase,
                 (DWORD)pfs->fatbase,
                 (DWORD)pfs->dirbase,
                 (DWORD)pfs->database,
                 (pfs->n_fatent - 2) * (pfs->csize / 2),
                 fre_clust * (pfs->csize / 2));
}

static void sd_card_write_test_file(void)
{
  FIL fil;
  FRESULT ret_code;
  UINT bw;

  sd_card_printf("-------- Open file to write and read again ---------\n\r");
  ret_code = f_open(&fil, "hello.txt", FA_OPEN_ALWAYS | FA_READ | FA_WRITE);
  assert_fatfs_ok(ret_code);

  ret_code = f_write(&fil, sd_card_test_string, sizeof(sd_card_test_string), &bw);
  assert_fatfs_ok(ret_code);
  sd_card_printf("Write a message to SD card success! Byte written = %d\n\r", bw);

  ret_code = f_close(&fil);
  assert_fatfs_ok(ret_code);
}

static void sd_card_read_test_file(void)
{
  FIL fil;
  FRESULT ret_code;
  UINT br;

  ret_code = f_open(&fil, "hello.txt", FA_OPEN_EXISTING | FA_READ);
  assert_fatfs_ok(ret_code);

  ret_code = f_read(&fil, sd_card_work_area, sizeof(sd_card_work_area), &br);
  assert_fatfs_ok(ret_code);
  sd_card_printf("Read a message from SD card success! Byte read = %d\n\r", br);
  sd_card_printf("Content: %s", sd_card_work_area);

  ret_code = f_close(&fil);
  assert_fatfs_ok(ret_code);
}

static void sd_card_run_file_smoke_test(void)
{
  sd_card_write_test_file();
  sd_card_read_test_file();
}

static void sd_card_unmount(void)
{
  (void)f_mount(NULL, "", 1);
}

void sd_card_init(void)
{
  FATFS fs;

  sd_card_init_fatfs_time();
  sd_card_init_spi();
  sd_card_mount(&fs);
  sd_card_print_volume_status();
  sd_card_run_file_smoke_test();
  sd_card_unmount();
}
