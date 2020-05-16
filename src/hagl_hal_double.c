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

#include "sdkconfig.h"
#include "hagl_hal.h"

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <mipi_display.h>
#include <bitmap.h>
#include <hagl.h>

#ifdef CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
static SemaphoreHandle_t mutex;
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
static uint8_t *buffer1;

static bitmap_t fb = {
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .depth = DISPLAY_DEPTH,
};

static spi_device_handle_t spi;
static const char *TAG = "hagl_esp_mipi";

/*
 * Initializes the MIPI display with double buffering
 */
bitmap_t *hagl_hal_init(void)
{
    mipi_display_init(&spi);
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    mutex = xSemaphoreCreateMutex();
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */

    ESP_LOGI(
        TAG, "Largest (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block before init: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );

    buffer1 = (uint8_t *) heap_caps_malloc(
        BITMAP_SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH),
        MALLOC_CAP_DMA
    );
    if (NULL == buffer1) {
        ESP_LOGE(TAG, "NO BUFFER 1");
    };

    ESP_LOGI(
        TAG, "Largest (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block after init: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );

    heap_caps_print_heap_info(MALLOC_CAP_DMA | MALLOC_CAP_32BIT);

    bitmap_init(&fb, buffer1);

    return &fb;
}

/*
 * Flushes the backbuffer contents to the display
 */
void hagl_hal_flush()
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    /* Flush the whole back buffer with locking. */
    xSemaphoreTake(mutex, portMAX_DELAY);
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) fb.buffer);
    xSemaphoreGive(mutex);
#else
    /* Flush the whole back buffer. */
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) fb.buffer);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

/*
 * Put a pixel to the display
 *
 * This is the only mandatory function which HAL must implement for HAGL
 * to be able to draw graphical primitives.
 */
void hagl_hal_put_pixel(int16_t x0, int16_t y0, color_t color)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
    color_t *ptr = (color_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    *ptr = color;
    xSemaphoreGive(mutex);
#else
    color_t *ptr = (color_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    *ptr = color;
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

/*
 * Blit the source bitmap the display
 */
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
    bitmap_blit(x0, y0, src, &fb);
    xSemaphoreGive(mutex);
#else
    bitmap_blit(x0, y0, src, &fb);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

/*
 * Blit the source bitmap to the display scaled up or down
 */
void hagl_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
    bitmap_scale_blit(x0, y0, w, h, src, &fb);
    xSemaphoreGive(mutex);
#else
    bitmap_scale_blit(x0, y0, w, h, src, &fb);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

/*
 * Accelerated horizontal line drawing
 */
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t width, color_t color)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
    color_t *ptr = (color_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t x = 0; x < width; x++) {
        *ptr++ = color;
    }
    xSemaphoreGive(mutex);
#else
    color_t *ptr = (color_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t x = 0; x < width; x++) {
        *ptr++ = color;
    }
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

/*
 * Accelerated vertical line drawing
 */
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t height, color_t color)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
    color_t *ptr = (color_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t y = 0; y < height; y++) {
        *ptr = color;
        ptr += fb.pitch / (fb.depth / 8);
    }
    xSemaphoreGive(mutex);
#else
    color_t *ptr = (color_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t y = 0; y < height; y++) {
        *ptr = color;
        ptr += fb.pitch / (fb.depth / 8);
    }
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

#endif /* CONFIG_HAGL_HAL_USE_DOUBLE_BUFFERING */
