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

#ifdef HAGL_HAL_USE_SPLIT_DOUBLE_BUFFERING

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <mipi_display.h>
#include <bitmap.h>
#include <hagl.h>


#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
static SemaphoreHandle_t mutex;
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
static uint8_t *buffer[2];

static bitmap_t fb = {
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .depth = DISPLAY_DEPTH,
};

static spi_device_handle_t spi;
static const char *TAG = "hagl_esp_mipi";

static inline uint8_t buffer_block(uint16_t y0)
{
    return (y0 < HAGL_HAL_BUFFER_SPLIT) ? 0 : 1;
}

/*
 * Initializes the MIPI display with double buffering
 */
bitmap_t *hagl_hal_init(void)
{
    size_t upper_size, lower_size;

#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    mutex = xSemaphoreCreateMutex();
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */

    mipi_display_init(&spi);

    upper_size = DISPLAY_PITCH * HAGL_HAL_BUFFER_SPLIT;
    lower_size = DISPLAY_PITCH * (DISPLAY_HEIGHT - HAGL_HAL_BUFFER_SPLIT);

    ESP_LOGI(TAG, "Splitted double buffer: %dB upper, %dB lower.", upper_size, lower_size);

    ESP_LOGI(
        TAG, "Largest free (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block: %dB",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );

    buffer[0] = (uint8_t *) heap_caps_malloc(
        upper_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );

    if (NULL == buffer[0]) {
        ESP_LOGE(TAG, "Upper %dx%dx2 block alloc failed", DISPLAY_WIDTH, HAGL_HAL_BUFFER_SPLIT);
    } else {
        ESP_LOGI(TAG, "Upper %dx%dx2 block at: %p", DISPLAY_WIDTH, HAGL_HAL_BUFFER_SPLIT, buffer[0]);
    };

    ESP_LOGI(
        TAG, "Largest free (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );

    buffer[1] = (uint8_t *) heap_caps_malloc(
        lower_size, MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );

    if (NULL == buffer[1]) {
        ESP_LOGE(TAG, "Lower %dx%dx2 block alloc failed", DISPLAY_WIDTH, DISPLAY_HEIGHT - HAGL_HAL_BUFFER_SPLIT);
    } else {
        ESP_LOGI(TAG, "Lower %dx%dx2 block at: %p", DISPLAY_WIDTH, DISPLAY_HEIGHT - HAGL_HAL_BUFFER_SPLIT, buffer[1]);
    };

    /* TODO: make bitmap handle split buffers. */
    bitmap_init(&fb, buffer[0]);

    // return &fb;
    return NULL;
}

/*
 * Flushes the backbuffer contents to the display
 */
void hagl_hal_flush()
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    /* Flush the whole back buffer with locking. */
    xSemaphoreTake(mutex, portMAX_DELAY);
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) buffer);
    xSemaphoreGive(mutex);
#else
    /* Flush the whole back buffer. */
    mipi_display_write(spi, 0, 0, DISPLAY_WIDTH, HAGL_HAL_BUFFER_SPLIT, (uint8_t *) buffer[0]);
    mipi_display_write(spi, 0, HAGL_HAL_BUFFER_SPLIT, DISPLAY_WIDTH, DISPLAY_HEIGHT - HAGL_HAL_BUFFER_SPLIT, (uint8_t *) buffer[1]);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

/*
 * Put a pixel to the display
 *
 * This is the only mandatory function which HAL must implement for HAGL
 * to be able to draw graphical primitives.
 */
void hagl_hal_put_pixel(int16_t x0, int16_t y0, uint16_t color)
{
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
    uint8_t block = buffer_block(y0);
    if (1 == block) {
        y0 = y0 - HAGL_HAL_BUFFER_SPLIT;
    }

    uint16_t *ptr = (uint16_t *) (buffer[block] + DISPLAY_PITCH * y0 + (DISPLAY_DEPTH / 8) * x0);
    *ptr = color;
#ifdef CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* CONFIG_HAGL_HAL_LOCK_WHEN_FLUSHING */
}

void hagl_hal_clear_screen()
{
    memset(buffer[0], 0x00, DISPLAY_PITCH * HAGL_HAL_BUFFER_SPLIT);
    memset(buffer[1], 0x00, DISPLAY_PITCH * (DISPLAY_HEIGHT - HAGL_HAL_BUFFER_SPLIT));
}


#endif /* HAGL_HAL_USE_SPLIT_DOUBLE_BUFFERING */
