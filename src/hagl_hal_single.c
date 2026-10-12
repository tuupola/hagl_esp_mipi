/*

MIT License

Copyright (c) 2019-2025 Mika Tuupola

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

This file is part of the ESP32 MIPI DCS HAL for HAGL graphics library:
https://github.com/tuupola/hagl_esp_mipi/

SPDX-License-Identifier: MIT

-cut-

This is the HAL used when buffering is disabled. I call this single buffered
since I consider the GRAM of the display driver chip to be the framebuffer.

Note that all coordinates are already clipped in the main library itself.
HAL does not need to validate the coordinates, they can alway be assumed
valid.

*/

#include "sdkconfig.h"
#include "hagl_hal.h"

#ifdef CONFIG_HAGL_HAL_NO_BUFFERING

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <mipi_display.h>
#include <hagl/bitmap.h>
#include <hagl.h>


static spi_device_handle_t spi;
static const char *TAG = "hagl_esp_mipi";

static void
put_pixel(const void *self, int16_t x0, int16_t y0, hagl_color_t color)
{
    mipi_display_write(spi, x0, y0, 1, 1, (uint8_t *) &color);
}

static void
blit_xy(const void *self, int16_t x0, int16_t y0, const void *src)
{
    const hagl_bitmap_t *bitmap = src;

    mipi_display_write(spi, x0, y0, bitmap->width, bitmap->height, (uint8_t *) bitmap->buffer);
}

static void
line_xyw(const void *self, int16_t x0, int16_t y0, uint16_t width, hagl_color_t color)
{
    static hagl_color_t line[DISPLAY_WIDTH];
    hagl_color_t *ptr = line;
    uint16_t height = 1;

    for (uint16_t x = 0; x < width; x++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) line);
}

static void
line_xyh(const void *self, int16_t x0, int16_t y0, uint16_t height, hagl_color_t color)
{
    static hagl_color_t line[DISPLAY_HEIGHT];
    hagl_color_t *ptr = line;
    uint16_t width = 1;

    for (uint16_t x = 0; x < height; x++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) line);
}

void
hagl_hal_init(hagl_backend_t *backend)
{
    mipi_display_init(&spi);

    backend->width = MIPI_DISPLAY_WIDTH;
    backend->height = MIPI_DISPLAY_HEIGHT;
    backend->depth = MIPI_DISPLAY_DEPTH;
    backend->put_pixel = put_pixel;
    // backend->get_pixel = get_pixel;
    backend->line_xyw = line_xyw;
    backend->line_xyh = line_xyh;
    backend->blit_xy = blit_xy;
}

#endif /* CONFIG_HAGL_HAL_NO_BUFFERING */
