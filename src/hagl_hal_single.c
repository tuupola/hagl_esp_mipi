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
#include <bitmap.h>
#include <hagl.h>


static spi_device_handle_t spi;
static const char *TAG = "hagl_esp_mipi";

static DMA_ATTR uint8_t mipi_display_DMA_buffer[CONFIG_MIPI_DISPLAY_DMA_BUFFER_SIZE*CONFIG_MIPI_DISPLAY_DEPTH/8];
static DMA_ATTR int mipi_display_DMA_buffer_size=CONFIG_MIPI_DISPLAY_DMA_BUFFER_SIZE*CONFIG_MIPI_DISPLAY_DEPTH/8;
static uint16_t * mipi_display_DMA_buffer_16bit=NULL;
static uint32_t * mipi_display_DMA_buffer_32bit=NULL;

bitmap_t *hagl_hal_init(void)
{
    mipi_display_DMA_buffer_16bit=(uint16_t *)mipi_display_DMA_buffer;
    mipi_display_DMA_buffer_32bit=(uint32_t *)mipi_display_DMA_buffer;

    mipi_display_init(&spi);
    return NULL;
}

void hagl_hal_put_pixel(int16_t x0, int16_t y0, color_t color)
{
#if defined(CONFIG_MIPI_DCS_PIXEL_FORMAT_16BIT_SELECTED)
    mipi_display_DMA_buffer_16bit[0]=color;
#elif defined(CONFIG_MIPI_DCS_PIXEL_FORMAT_24BIT_SELECTED)
    mipi_display_DMA_buffer_8bit[0]=(uint8_t)color>>16;
    mipi_display_DMA_buffer_8bit[1]=(uint8_t)color>>8;
    mipi_display_DMA_buffer_8bit[2]=(uint8_t)color;
#elif defined(CONFIG_MIPI_DCS_PIXEL_FORMAT_8BIT_SELECTED)
    mipi_display_DMA_buffer[0]=color;
#endif
    mipi_display_write(spi, x0, y0, 1, 1, mipi_display_DMA_buffer);
//    mipi_display_write(spi, x0, y0, 1, 1, (uint8_t *) &color);
}

void hagl_hal_blit(uint16_t x0, uint16_t y0, bitmap_t *src)
{
    uint32_t batch_lines_count=0;
    uint32_t loop_count=0;
    uint32_t rest_lines=0;
    if(src->size>mipi_display_DMA_buffer_size)
    {
        batch_lines_count= mipi_display_DMA_buffer_size / src->pitch;
        loop_count= src->height / batch_lines_count;//because line 0 well be counted.
        rest_lines= src->height % batch_lines_count;
        for (int i = 0; i < loop_count; i++) {
            memcpy(mipi_display_DMA_buffer,src->buffer+i*src->pitch*batch_lines_count, batch_lines_count * src->pitch);
            mipi_display_write(spi, x0, y0+ i * batch_lines_count, src->width, batch_lines_count, mipi_display_DMA_buffer);
        }
        if(rest_lines){
            memcpy(mipi_display_DMA_buffer,src->buffer+loop_count*src->pitch, rest_lines * src->pitch);
            mipi_display_write(spi, x0, y0+ loop_count* batch_lines_count, src->width, rest_lines, mipi_display_DMA_buffer);
        }
    }else{
        memcpy(mipi_display_DMA_buffer,src->buffer,src->size);
        mipi_display_write(spi, x0, y0, src->width, src->height, mipi_display_DMA_buffer);
    }



//    mipi_display_write(spi, x0, y0, src->width, src->height, (uint8_t *) src->buffer);


}
uint8_t* hagl_hal_get_DMA_buf(){
return mipi_display_DMA_buffer;
}

uint32_t hagl_hal_get_DMA_buf_size(){
    return mipi_display_DMA_buffer_size;
}

void hagl_hal_fast_blit(uint16_t x0, uint16_t y0, uint16_t x1,uint16_t y1){
    mipi_display_write(spi, x0, y0, x1, y1, mipi_display_DMA_buffer);
}

void hagl_hal_hline(int16_t x0, int16_t y0, uint16_t width, color_t color)
{
//    static color_t line[DISPLAY_WIDTH];
//    color_t *ptr = line;
    uint16_t height = 1;
    color_t *ptr = (color_t *)mipi_display_DMA_buffer;

    for (uint16_t x = 0; x < width; x++) {
        *(ptr++) = color;
    }
//    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) line);
    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) mipi_display_DMA_buffer);
}

void hagl_hal_vline(int16_t x0, int16_t y0, uint16_t height, color_t color)
{
//    static color_t line[DISPLAY_HEIGHT];
//    color_t *ptr = line;
    uint16_t width = 1;
    color_t *ptr = (color_t *)mipi_display_DMA_buffer;
    for (uint16_t x = 0; x < height; x++) {
        *(ptr++) = color;
    }
//    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) line);
    mipi_display_write(spi, x0, y0, width, height, (uint8_t *) mipi_display_DMA_buffer);
}

#endif /* CONFIG_HAGL_HAL_NO_BUFFERING */
