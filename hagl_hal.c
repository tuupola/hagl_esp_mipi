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

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <mipi_display.h>
#include <bitmap.h>
#include <hagl.h>

#include "hagl_hal.h"

#ifdef HAGL_HAL_USE_BUFFERING
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
static SemaphoreHandle_t mutex;
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
static uint8_t *buffer1;
static uint8_t *buffer2;

static bitmap_t fb = {
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .depth = DISPLAY_DEPTH,
};
#endif /* HAGL_HAL_USE_BUFFERING */

static spi_device_handle_t spi;
static const char *TAG = "hagl_esp_mipi";
/*
 * Initializes the MIPI display and an optional framebuffer
 */

#ifdef HAGL_HAL_USE_TRIPLE_BUFFERING
bitmap_t *hagl_hal_init(void)
{
    mipi_display_init(&spi);

    ESP_LOGI(
        TAG, "Largest free (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );
    // heap_caps_print_heap_info(MALLOC_CAP_DMA | MALLOC_CAP_32BIT);

    buffer1 = (uint8_t *) heap_caps_malloc(
        BITMAP_SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH),
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );
    if (NULL == buffer1) {
        ESP_LOGE(TAG, "NO BUFFER 1");
    } else {
        ESP_LOGI(TAG, "Buffer 1 at: %p", buffer1);
    };

    buffer2 = (uint8_t *) heap_caps_malloc(
        BITMAP_SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH),
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );

    if (NULL == buffer2) {
        ESP_LOGI(TAG, "NO BUFFER 2");
    } else {
        ESP_LOGI(TAG, "Buffer 2 at: %p", buffer2);
    };

    /* Clear both and leave pointer to buffer1. */
    bitmap_init(&fb, buffer2);
    bitmap_init(&fb, buffer1);

    ESP_LOGI(
        TAG, "Largest free (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) block: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );
    // heap_caps_print_heap_info(MALLOC_CAP_DMA | MALLOC_CAP_32BIT);

    return &fb;
}
#endif /* HAGL_HAL_USE_TRIPLE_BUFFERING */


#ifdef HAGL_HAL_USE_DOUBLE_BUFFERING
bitmap_t *hagl_hal_init(void)
{
    mipi_display_init(&spi);
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    mutex = xSemaphoreCreateMutex();
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */

    ESP_LOGI(
        TAG, "Heap (MALLOC_CAP_DMA | MALLOC_CAP_32BIT) when starting: %d",
        heap_caps_get_largest_free_block(MALLOC_CAP_DMA | MALLOC_CAP_32BIT)
    );
    heap_caps_print_heap_info(MALLOC_CAP_DMA | MALLOC_CAP_32BIT);
    buffer1 = (uint8_t *) heap_caps_malloc(
        BITMAP_SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH),
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );
    if (NULL == buffer1) {
        ESP_LOGE(TAG, "NO BUFFER 1");
    };

    bitmap_init(&fb, buffer1);

    return &fb;
}
#endif /* HAGL_HAL_USE_DOUBLE_BUFFERING */

#ifndef HAGL_HAL_USE_BUFFERING
bitmap_t *hagl_hal_init(void)
{
    mipi_display_init(&spi);
    return NULL;
}
#endif /* HAGL_HAL_USE_BUFFERING */

/*
 * Flushes the optional framebuffer contents to the display
 */
#ifdef HAGL_HAL_USE_TRIPLE_BUFFERING
void hagl_hal_flush()
{
    uint8_t *buffer = fb.buffer;
    if (fb.buffer == buffer1) {
        fb.buffer = buffer2;
    } else {
        fb.buffer = buffer1;
    }
    // ESP_LOGI(TAG, "Buffer is now %p", fb.buffer);
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) buffer);
    // ESP_LOGI(TAG, "Flushing %p", buffer);

}
#endif /* HAGL_HAL_USE_TRIPLE_BUFFERING */


#ifdef HAGL_HAL_USE_DOUBLE_BUFFERING
void hagl_hal_flush()
{
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    /* Flush the whole back buffer with locking. */
    xSemaphoreTake(mutex, portMAX_DELAY);
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) fb.buffer);
    xSemaphoreGive(mutex);
#else
    /* Flush the whole back buffer. */
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) fb.buffer);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
}
#endif /* HAGL_HAL_USE_DOUBLE_BUFFERING */

#ifndef HAGL_HAL_USE_BUFFERING
void hagl_hal_flush()
{
}
#endif /* HAGL_HAL_USE_BUFFERING */

/*
 * Put a pixel to the display
 *
 * This is the only mandatory function which HAL must implement for HAGL
 * to be able to draw graphical primitives.
 */
void hagl_hal_put_pixel(int16_t x0, int16_t y0, uint16_t color)
{
#ifdef HAGL_HAL_USE_BUFFERING
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    *ptr = color;
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
#else
    mipi_display_write(spi, x0, y0, 1, 1, (uint8_t *) &color);
#endif /* HAGL_HAL_USE_BUFFERING */
}

/*
 * Blit the source bitmap the display
 */
void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src)
{
#ifdef HAGL_HAL_USE_BUFFERING
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
    bitmap_blit(x0, y0, src, &fb);
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
#else
    mipi_display_write(spi, x0, y0, src->width, src->height, (uint8_t *) src->buffer);
#endif /* HAGL_HAL_USE_BUFFERING */
}

/*
 * Blit the source bitmap to the display scaled up or down
 */
void hagl_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src)
{
#ifdef HAGL_HAL_USE_BUFFERING
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
    bitmap_scale_blit(x0, y0, w, h, src, &fb);
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
#else
    /* TODO */
#endif /* HAGL_HAL_USE_BUFFERING */
}

/*
 * Accelerated horizontal line drawing
 */
void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t width, uint16_t color)
{
#ifdef HAGL_HAL_USE_BUFFERING
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t x = 0; x < width; x++) {
        *ptr++ = color;
    }
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
#else
    static uint16_t line[DISPLAY_WIDTH];
    uint16_t *ptr = line;
    uint16_t height = 1;

    for (uint16_t x = 0; x < width; x++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) line);
#endif /* HAGL_HAL_USE_BUFFERING */
}

/*
 * Accelerated vertical line drawing
 */
void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t height, uint16_t color)
{
#ifdef HAGL_HAL_USE_BUFFERING
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreTake(mutex, portMAX_DELAY);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t y = 0; y < height; y++) {
        *ptr = color;
        ptr += fb.pitch / (fb.depth / 8);
    }
#ifdef HAGL_HAL_LOCK_WHEN_FLUSHING
    xSemaphoreGive(mutex);
#endif /* HAGL_HAL_LOCK_WHEN_FLUSHING */
#else
    uint16_t line[DISPLAY_HEIGHT];
    uint16_t *ptr = line;
    uint16_t width = 1;

    for (uint16_t x = 0; x < height; x++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) line);
#endif /* HAGL_HAL_USE_BUFFERING */
}
