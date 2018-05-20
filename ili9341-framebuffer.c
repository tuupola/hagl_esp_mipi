/*

Copyright (c) 2018 Mika Tuupola

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

#include <string.h>
#include <ili9341.h>
#include <blit.h>
#include <copepod.h>
#include <framebuffer.h>

#include "ili9341-framebuffer.h"

static framebuffer_t fb = {
    .width = FRAMEBUFFER_WIDTH,
    .height = FRAMEBUFFER_HEIGHT,
    .depth = 16,
};

static spi_device_handle_t spi;

/*
 * Initializes the ILI9341 + framebuffer HAL.
 */
void pod_hal_init(void)
{
    ili9341_init(&spi);
    framebuffer_init(&fb);
}

/*
 * Flushes the framebuffer contents to the actual display.
 */
void pod_hal_flush(void)
{
    ili9431_blit(spi, 0, 0, fb.width, fb.height, (uint16_t *) fb.buffer);
}

/*
 * Putpixel function. This is the only mandatory function which HAL
 * must implement for copepod to be able to draw graphical primitives.
 * This version draws to a framebuffer.
 */
void pod_hal_putpixel(uint16_t x0, uint16_t y0, uint16_t color)
{
    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);

    if ((x0 < fb.width) && (y0 < fb.height)) {
    	*ptr = color;
    }
}

/*
 * Blit the source bitmap to the framebuffer.
 */
void pod_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src)
{
    blit(x0, y0, src, &fb);
}

/*
 * Blit the source bitmap to the framebuffer scaled up or down.
 * TODO: stretch might be more proper naming?
 */
void pod_hal_scale_blit(uint16_t x0, uint16_t y0, uint16_t w, uint16_t h, bitmap_t *src)
{
    scale_blit(x0, y0, w, h, src, &fb);
}

/*
 * Accelerated horizontal line drawing.
 */
void pod_hal_hline(uint16_t x0, uint16_t y0, uint16_t width, uint16_t color)
{
    /* x0 or y0 is over the edge, nothing to do. */
    if ((x0 > fb.width) || (y0 > fb.height))  {
        return;
    }

    /* Cut anything going over right edge. */
    if (((x0 + width) > fb.width))  {
        width = width - (x0 + width - fb.width);
    }

    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t x = 0; x < width; x++) {
        *ptr++ = color;
    }
}

/*
 * Accelerated vertical line drawing.
 */
void pod_hal_vline(uint16_t x0, uint16_t y0, uint16_t height, uint16_t color)
{
    /* x0 or y0 is over the edge, nothing to do. */
    if ((x0 > fb.width) || (y0 > fb.height))  {
        return;
    }

    /* Cut anything going over right edge. */
    if (((y0 + height) > fb.height))  {
        height = height - (y0 + height - fb.height);
    }

    uint16_t *ptr = (uint16_t *) (fb.buffer + fb.pitch * y0 + (fb.depth / 8) * x0);
    for (uint16_t y = 0; y < height; y++) {
        *ptr = color;
        ptr += fb.pitch / (fb.depth / 8);
    }
}
