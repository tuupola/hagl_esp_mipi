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

This is the HAL used when double buffering is enabled. The GRAM of the
display driver chip is the framebuffer. The memory allocated by this HAL
is the back buffer. Total two buffers.

Note that all coordinates are already clipped in the main library itself.
HAL does not need to validate the coordinates, they can alway be assumed
valid.

*/

#include "sdkconfig.h"
#include "hagl_hal.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <mipi_display.h>
#include <hagl/bitmap.h>
#include <hagl.h>

#ifdef CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
static SemaphoreHandle_t mutex;
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
// static uint8_t *buffer;

static hagl_bitmap_t bb;

static spi_device_handle_t spi;
static const char *TAG = "hagl_esp_mipi";

static size_t
flush(const void *self)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    size_t size = 0;
    /* Flush the whole back buffer with locking. */
    xSemaphoreTake(mutex, portMAX_DELAY);
    size = mipi_display_write(spi, 0, 0, bb.width, bb.height, (uint8_t *) bb.buffer);
    xSemaphoreGive(mutex);
    return size;
#else
    /* Flush the whole back buffer. */
    return mipi_display_write(spi, 0, 0, bb.width, bb.height, (uint8_t *) bb.buffer);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

static void
put_pixel(const void *self, int16_t x0, int16_t y0, hagl_color_t color)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
    bb.put_pixel(&bb, x0, y0, color);
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

static hagl_color_t
get_pixel(const void *self, int16_t x0, int16_t y0)
{
    return bb.get_pixel(&bb, x0, y0);
}

static void
blit_xy(const void *self, int16_t x0, int16_t y0, const void *src)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
    bb.blit_xy(&bb, x0, y0, src);
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

static void
blit_xywh(const void *self, int16_t x0, int16_t y0, uint16_t w, uint16_t h, const void *src)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
    bb.blit_xywh(&bb, x0, y0, w, h, src);
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

static void
line_xyw(const void *self, int16_t x0, int16_t y0, uint16_t width, hagl_color_t color)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
    bb.line_xyw(&bb, x0, y0, width, color);
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

static void
line_xyh(const void *self, int16_t x0, int16_t y0, uint16_t height, hagl_color_t color)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
    bb.line_xyh(&bb, x0, y0, height, color);
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

// void hagl_hal_clear_screen()
// {
// #ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
//     xSemaphoreTake(mutex, portMAX_DELAY);
// #endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
//     hagl_color_t *ptr = (hagl_color_t *) buffer;
//     size_t count = DISPLAY_WIDTH * DISPLAY_HEIGHT;

//     while (--count) {
//         *ptr++ = 0x0000;
//     }
// #ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
//     xSemaphoreGive(mutex);
// #endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
// }

void
hagl_hal_init(hagl_backend_t *backend)
{
    mipi_display_init(&spi);
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    mutex = xSemaphoreCreateMutex();
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */

    ESP_LOGI(
        TAG, "Largest (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block before init: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );

    backend->buffer = (uint8_t *) heap_caps_malloc(
            BITMAP_SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH),
            MALLOC_CAP_DMA
        );
    if (NULL == backend->buffer) {
        ESP_LOGE(TAG, "NO BUFFER");
    };

    ESP_LOGI(
        TAG, "Largest (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block after init: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );

    heap_caps_print_heap_info(MALLOC_CAP_DMA | MALLOC_CAP_32BIT);

    backend->width = MIPI_DISPLAY_WIDTH;
    backend->height = MIPI_DISPLAY_HEIGHT;
    backend->depth = MIPI_DISPLAY_DEPTH;
    backend->put_pixel = put_pixel;
    backend->get_pixel = get_pixel;
    backend->line_xyw = line_xyw;
    backend->line_xyh = line_xyh;
    backend->blit_xy = blit_xy;
    backend->blit_xywh = blit_xywh;
    backend->flush = flush;

    hagl_bitmap_init(&bb, backend->width, backend->height, backend->depth, backend->buffer);
}

#endif /* CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING */
