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

#define DISPLAY_WIDTH           (CONFIG_MIPI_DISPLAY_WIDTH)
#define DISPLAY_HEIGHT          (CONFIG_MIPI_DISPLAY_HEIGHT)
#define DISPLAY_DEPTH           (CONFIG_MIPI_DISPLAY_DEPTH)

#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_8BIT_INDEXED_SELECTED
#define HAGL_HAL_FORMAT_INDEX8
#define HAGL_HAL_HARDWARE_DEPTH (16)
#else
#define HAGL_HAL_HARDWARE_DEPTH (CONFIG_MIPI_DISPLAY_DEPTH)
#endif

#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_24BIT_SELECTED
typedef uint32_t color_t;
#endif
#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_18BIT_SELECTED
typedef uint32_t color_t;
#endif
#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_16BIT_SELECTED
typedef uint16_t color_t;
#endif
#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_12BIT_SELECTED
typedef uint16_t color_t;
#endif
#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_8BIT_SELECTED
typedef uint8_t color_t;
#endif
#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_8BIT_INDEXED_SELECTED
typedef uint8_t color_t;
#endif
#ifdef CONFIG_MIPI_DCS_PIXEL_FORMAT_3BIT_SELECTED
typedef uint8_t color_t;
#endif

/* This is the only mandatory function HAL must provide. */
void hagl_hal_put_pixel(int16_t x0, int16_t y0, color_t color);

/* Unbuffered RGB565. */
#if defined (CONFIG_HAGL_HAL_NO_BUFFERING) && !defined (HAGL_HAL_FORMAT_INDEX8)
#define HAGL_HAS_HAL_INIT
#define HAGL_HAS_HAL_BLIT
#define HAGL_HAS_HAL_HLINE
#define HAGL_HAS_HAL_VLINE

bitmap_t *hagl_hal_init(void);
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src);
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t w, color_t color);
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t h, color_t color);
#endif /* CONFIG_HAGL_HAL_NO_BUFFERING */

/* Double buffered RGB565. */
#if defined (CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING) && !defined (HAGL_HAL_FORMAT_INDEX8)
#define HAGL_HAL_USE_BUFFERING

#define HAGL_HAS_HAL_INIT
#define HAGL_HAS_HAL_BLIT
#define HAGL_HAS_HAL_SCALE_BLIT
#define HAGL_HAS_HAL_HLINE
#define HAGL_HAS_HAL_VLINE
#define HAGL_HAS_HAL_FLUSH

bitmap_t *hagl_hal_init(void);
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src);
void hagl_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src);
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t w, color_t color);
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t h, color_t color);
void hagl_hal_flush();
#endif /* CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING) && !(HAGL_HAL_FORMAT_INDEX8) */

/* Double buffered INDEX8. */
#if defined (CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING) && defined (HAGL_HAL_FORMAT_INDEX8)
#define HAGL_HAL_USE_BUFFERING

#define HAGL_HAS_HAL_INIT
// #define HAGL_HAS_HAL_BLIT
// #define HAGL_HAS_HAL_SCALE_BLIT
// #define HAGL_HAS_HAL_HLINE
// #define HAGL_HAS_HAL_VLINE
#define HAGL_HAS_HAL_FLUSH

bitmap_t *hagl_hal_init(void);
// void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src);
// void hagl_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src);
// void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t w, color_t color);
// void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t h, color_t color);
void hagl_hal_flush();
#endif /* CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING) && (HAGL_HAL_FORMAT_INDEX8) */


#ifdef CONFIG_HAGL_HAL_USE_TRIPLE_BUFFERING
#define HAGL_HAL_USE_BUFFERING

#define HAGL_HAS_HAL_INIT
#define HAGL_HAS_HAL_BLIT
#define HAGL_HAS_HAL_SCALE_BLIT
#define HAGL_HAS_HAL_HLINE
#define HAGL_HAS_HAL_VLINE
#define HAGL_HAS_HAL_FLUSH

bitmap_t *hagl_hal_init(void);
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src);
void hagl_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src);
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t w, color_t color);
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t h, color_t color);
void hagl_hal_flush();
#endif /* CONFIG_HAGL_HAL_USE_TRIPLE_BUFFERING */

#ifdef __cplusplus
}
#endif
#endif /* _HAGL_HAL_H */