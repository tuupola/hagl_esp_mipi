# MIPI DCS HAL for Copepod Graphics Library

HAL for Copepod graphics library for display drivers supporting the [MIPI Display Command Set](https://www.mipi.org/specifications/display-command-set). Currently tested with ST7735S, ST7789V and ILI9341.

[![Software License](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat-square)](LICENSE.md)

## Usage

To use with an ESP-IDF project you need to include three components. Low level [MIPI DCS display driver](https://github.com/tuupola/esp_mipi), this HAL and the [Copepod graphics library](https://github.com/tuupola/copepod) itself.

```
$ cd components
$ git submodule add git@github.com:tuupola/esp_mipi.git
$ git submodule add git@github.com:tuupola/copepod_esp_mipi.git
$ git submodule add git@github.com:tuupola/copepod.git
```

You can alter  HAL behaviour via `menuconfig`. If you choose to use back buffer all drawing operations will be fast. Downside is that back buffer requires lot of memory. Each pixel is two bytes. To reduce the amount of data SPI bus has to transfer choose the option to flush only dirty part of back buffer. To reduce flickering you can also choose to lock back buffer while flushing. Locking will slow down draw operations though.

```
$ make menuconfig
```

For example usage see [M5Stack GFX](https://github.com/tuupola/esp-examples/tree/master/009-m5stack-gfx), [M5Stick GFX](https://github.com/tuupola/esp-examples/tree/master/015-m5stick-gfx) and [Mandelbrot](https://github.com/tuupola/esp-examples/tree/master/014-mandelbrot).

## License

The MIT License (MIT). Please see [License File](LICENSE.md) for more information.
