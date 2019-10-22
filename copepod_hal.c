/*

Copyright (c) 2019 Mika Tuupola

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

*/

/* See: https://github.com/tuupola/copepod_esp_mipi/ */

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <esp_heap_caps.h>
#include <string.h>
#include <mipi_display.h>
#include <bitmap.h>
#include <copepod.h>

#include "sdkconfig.h"
#include "copepod_hal.h"

#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
static SemaphoreHandle_t mutex;
static uint8_t *buffer;
static bitmap_t fb = {
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .depth = DISPLAY_DEPTH,
};
#endif

static spi_device_handle_t spi;

/*
 * Initializes the MIPI display and an optional framebuffer
 */
void pod_hal_init(void)
{
    mipi_display_init(&spi);
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
    mutex = xSemaphoreCreateMutex();

    buffer = (uint8_t *) heap_caps_malloc(
        BITMAP_SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH),
        MALLOC_CAP_DMA | MALLOC_CAP_32BIT
    );
    bitmap_init(&fb, buffer);
#endif
}

/*
 * Flushes the optional framebuffer contents to the display
 */
void pod_hal_flush(bool dirty, int16_t x0, int16_t y0, int16_t x1, int16_t y1)
{
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
#ifdef CONFIG_POD_HAL_FLUSH_ONLY_DIRTY
    if (dirty) {
        uint16_t height = y1 - y0 + 1;
        uint8_t *ptr = (uint8_t *) (buffer + fb.pitch * y0 + (fb.depth / 8));
#ifdef CONFIG_POD_HAL_LOCK_WHEN_FLUSHING
        /* Flush only dirty part of back buffer with locking. */
        xSemaphoreTake(mutex, portMAX_DELAY);
        mipi_display_write(spi, 0, y0, fb.width, height, ptr);
        xSemaphoreGive(mutex);
#else
        /* Flush only dirty part of back buffer. */
        mipi_display_write(spi, 0, y0, fb.width, height, ptr);
#endif
    }
#else
#ifdef CONFIG_POD_HAL_LOCK_WHEN_FLUSHING
    /* Flush the whole back buffer with locking. */
    xSemaphoreTake(mutex, portMAX_DELAY);
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) fb.buffer);
    xSemaphoreGive(mutex);
#else
    /* Flush the whole back buffer. */
    mipi_display_write(spi, 0, 0, fb.width, fb.height, (uint8_t *) fb.buffer);
#endif
#endif
#endif
}

/*
 * Put a pixel to the display
 *
 * This is the only mandatory function which HAL must implement for copepod
 * to be able to draw graphical primitives.
 */
void pod_hal_putpixel(int16_t x0, int16_t y0, uint16_t color)
{
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    *ptr = color;
    xSemaphoreGive(mutex);
#else
    mipi_display_write(spi, x0, y0, 1, 1, (uint8_t *) &color);
#endif
}

/*
 * Blit the source bitmap the display
 */
void pod_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src)
{
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
    xSemaphoreTake(mutex, portMAX_DELAY);
    bitmap_blit(x0, y0, src, &fb);
    xSemaphoreGive(mutex);
#else
    mipi_display_write(spi, x0, y0, src->width, src->height, (uint8_t *) src->buffer);
#endif
}

/*
 * Blit the source bitmap to the display scaled up or down
 */
void pod_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src)
{
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
    xSemaphoreTake(mutex, portMAX_DELAY);
    bitmap_scale_blit(x0, y0, w, h, src, &fb);
    xSemaphoreGive(mutex);
#else
    /* TODO */
#endif
}

/*
 * Accelerated horizontal line drawing
 */
void pod_hal_hline(int16_t x0, int16_t y0, uint16_t width, uint16_t color)
{
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t x = 0; x <= width; x++) {
        *ptr++ = color;
    }
    xSemaphoreGive(mutex);
#else
    static uint16_t line[DISPLAY_WIDTH];
    uint16_t *ptr = line;
    uint16_t height = 1;

    for (uint16_t x = 0; x <= width; x++) {
        *(ptr++) = color;
    }

    /* TODO: width has off by one error somewhere */
    mipi_display_write(spi, x0, y0, width + 1, height, (uint8_t *) line);
#endif
}

/*
 * Accelerated vertical line drawing
 */
void pod_hal_vline(int16_t x0, int16_t y0, uint16_t height, uint16_t color)
{
#ifdef CONFIG_POD_HAL_USE_DOUBLE_BUFFERING
    xSemaphoreTake(mutex, portMAX_DELAY);
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t y = 0; y <= height; y++) {
        *ptr = color;
        ptr += fb.pitch / (fb.depth / 8);
    }
    xSemaphoreGive(mutex);
#else
    uint16_t line[DISPLAY_HEIGHT];
    uint16_t *ptr = line;
    uint16_t width = 1;

    for (uint16_t x = 0; x <= width; x++) {
        *(ptr++) = color;
    }

    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) line);
#endif
}
