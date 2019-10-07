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

/* See: https://github.com/tuupola/copepod-esp-mipi */

#include <esp_log.h>

#include <string.h>
#include <mipi_display.h>
#include <bitmap.h>
#include <copepod.h>

#include "sdkconfig.h"
#include "copepod-hal.h"

#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
static bitmap_t fb = {
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
    .depth = DISPLAY_DEPTH,
};
#endif

static spi_device_handle_t spi;

/*
 * Initializes the MIPI display + framebuffer HAL.
 */
void pod_hal_init(void)
{
    mipi_display_init(&spi);
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    static uint8_t buffer[BITMAP_SIZE(DISPLAY_WIDTH, DISPLAY_HEIGHT, DISPLAY_DEPTH)];
    bitmap_init(&fb, buffer);
#endif
}

/*
 * Flushes the framebuffer contents to the actual display.
 */
void pod_hal_flush(void)
{
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    mipi_display_blit(spi, 0, 0, fb.width, fb.height, (uint16_t *) fb.buffer);
#endif
}

/*
 * Putpixel function. This is the only mandatory function which HAL
 * must implement for copepod to be able to draw graphical primitives.
 * This version draws to a framebuffer.
 */
void pod_hal_putpixel(int16_t x0, int16_t y0, uint16_t color)
{
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    *ptr = color;
#else
    mipi_display_put_pixel(spi, x0, y0, color);
#endif
}

/*
 * Blit the source bitmap to the framebuffer.
 */
void pod_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src)
{
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    bitmap_blit(x0, y0, src, &fb);
#else
    mipi_display_blit(spi, x0, y0, src->width, src->height, (uint16_t *) src->buffer);
#endif
}

/*
 * Blit the source bitmap to the framebuffer scaled up or down.
 * TODO: stretch might be more proper naming?
 */
void pod_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src)
{
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    bitmap_scale_blit(x0, y0, w, h, src, &fb);
#else
    /* TODO */
#endif
}

/*
 * Accelerated horizontal line drawing.
 */
void pod_hal_hline(int16_t x0, int16_t y0, uint16_t width, uint16_t color)
{
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t x = 0; x <= width; x++) {
        *ptr++ = color;
    }
#else
    static uint16_t line[DISPLAY_WIDTH];
    uint16_t *ptr = line;
    uint16_t height = 1;

    for (uint16_t x = 0; x <= width; x++) {
        *(ptr++) = color;
    }

    mipi_display_blit(spi, x0, y0, width, height, line);
#endif
}

/*
 * Accelerated vertical line drawing.
 */
void pod_hal_vline(int16_t x0, int16_t y0, uint16_t height, uint16_t color)
{
#ifdef CONFIG_POD_HAL_USE_FRAMEBUFFER
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t y = 0; y <= height; y++) {
        *ptr = color;
        ptr += fb.pitch / (fb.depth / 8);
    }
#else
    uint16_t line[DISPLAY_HEIGHT];
    uint16_t *ptr = line;
    uint16_t width = 1;

    for (uint16_t x = 0; x <= width; x++) {
        *(ptr++) = color;
    }

    mipi_display_blit(spi, x0, y0, width, height, &line);
#endif
}
