/*

MIT License

Copyright (c) 2019-2020 Mika Tuupola

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

-cut-

This file is part of the MIPI DCS HAL for HAGL graphics library:
https://github.com/tuupola/hagl_esp_mipi/

SPDX-License-Identifier: MIT

*/

#ifndef _HAGL_HAL_H
#define _HAGL_HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sdkconfig.h"

#include <stdint.h>
#include <bitmap.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define DISPLAY_WIDTH       (CONFIG_MIPI_DISPLAY_WIDTH)
#define DISPLAY_HEIGHT      (CONFIG_MIPI_DISPLAY_HEIGHT)
#define DISPLAY_DEPTH       (16)
#define DISPLAY_PITCH       (DISPLAY_WIDTH * (DISPLAY_DEPTH / 8))
#define DISPLAY_BYTES       (DISPLAY_PITCH * DISPLAY_HEIGHT)

/* This is the only mandatory function HAL must provide. */
void hagl_hal_put_pixel(int16_t x0, int16_t y0, uint16_t color);


#ifdef CONFIG_HAGL_HAL_BUFFER_SPLIT
#define HAGL_HAL_BUFFER_SPLIT   (CONFIG_HAGL_HAL_BUFFER_SPLIT)

#ifdef CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING
#define CONFIG_HAGL_HAL_USE_SPLIT_DOUBLE_BUFFERING
#undef CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING
#endif

#ifdef CONFIG_HAGL_HAL_USE_TRIPLE_BUFFERING
#define CONFIG_HAGL_HAL_USE_SPLIT_TRIPLE_BUFFERING
#undef CONFIG_HAGL_HAL_USE_TRIPLE_BUFFERING
#endif

#endif /* CONFIG_HAGL_HAL_BUFFER_SPLIT */

/* No buffering is the default. */
#ifdef CONFIG_HAGL_HAL_NO_BUFFERING
#define HAGL_HAS_HAL_INIT
#define HAGL_HAS_HAL_BLIT
#define HAGL_HAS_HAL_HLINE
#define HAGL_HAS_HAL_VLINE

bitmap_t *hagl_hal_init(void);
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src);
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t w, uint16_t color);
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t h, uint16_t color);
#endif /* CONFIG_HAGL_HAL_NO_BUFFERING */

#ifdef CONFIG_HAGL_HAL_USE_TRIPLE_BUFFERING
#define HAGL_HAL_USE_BUFFERING
#define HAGL_HAL_USE_TRIPLE_BUFFERING

#define HAGL_HAS_HAL_INIT
#define HAGL_HAS_HAL_BLIT
#define HAGL_HAS_HAL_SCALE_BLIT
#define HAGL_HAS_HAL_HLINE
#define HAGL_HAS_HAL_VLINE
#define HAGL_HAS_HAL_FLUSH

bitmap_t *hagl_hal_init(void);
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src);
void hagl_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src);
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t w, uint16_t color);
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t h, uint16_t color);
void hagl_hal_flush();
#endif /* CONFIG_HAGL_HAL_USE_TRIPLE_BUFFERING */

#ifdef CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING
#define HAGL_HAL_USE_BUFFERING
#define HAGL_HAL_USE_DOUBLE_BUFFERING

#define HAGL_HAS_HAL_INIT
#define HAGL_HAS_HAL_BLIT
#define HAGL_HAS_HAL_SCALE_BLIT
#define HAGL_HAS_HAL_HLINE
#define HAGL_HAS_HAL_VLINE
#define HAGL_HAS_HAL_FLUSH

bitmap_t *hagl_hal_init(void);
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src);
void hagl_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src);
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t w, uint16_t color);
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t h, uint16_t color);
void hagl_hal_flush();
#endif /* CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING */

#ifdef CONFIG_HAGL_HAL_USE_SPLIT_DOUBLE_BUFFERING
#define HAGL_HAL_USE_BUFFERING
#define HAGL_HAL_USE_SPLIT_DOUBLE_BUFFERING

#define HAGL_HAS_HAL_INIT
#define HAGL_HAS_HAL_FLUSH
#define HAGL_HAS_HAL_CLEAR_SCREEN

bitmap_t *hagl_hal_init(void);
void hagl_hal_flush();
void hagl_hal_clear_screen();
#endif /* CONFIG_HAGL_HAL_USE_SPLIT_DOUBLE_BUFFERING */


#ifdef __cplusplus
}
#endif
#endif /* _HAGL_HAL_H */