/*
   SPDX-FileCopyrightText: 2022-2023 Espressif Systems (Shanghai) CO LTD

   SPDX-License-Identifier: Unlicense OR CC0-1.0
*/

#ifndef ESP_HID_MSC_HOST_H
#define ESP_HID_MSC_HOST_H

#include <stdlib.h>
#include <string.h>
#include <assert.h>

// #define CONFIG_HID_HOST_MOUSE_SUPPORT 1


#include "src/usb_host.h"
#include "src/hid_host.h"
#include "src/hid_usage_keyboard.h"
#ifdef CONFIG_HID_HOST_MOUSE_SUPPORT
#include "src/hid_usage_mouse.h"
#endif

  // PD

#include "src/msc_host.h"
#include "src/msc_host_vfs.h"
#include "ffconf.h"      // Fat Fs Header file
#include "esp_vfs.h"     // virtual file
#include <esp_vfs_fat.h> // FATFS Mount Header file

#endif