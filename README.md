# MIPI DCS HAL for HAGL Graphics Library

HAL for HAGL graphics library for display drivers supporting the [MIPI Display Command Set](https://www.mipi.org/specifications/display-command-set). Currently tested with ST7735S, ST7789V and ILI9341.

[![Software License](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat-square)](LICENSE.md)

## Usage

To use with an ESP-IDF project you include this HAL and the [HAGL graphics library](https://github.com/tuupola/hagl) itself.

```
$ cd components
$ git submodule add git@github.com:tuupola/hagl_esp_mipi.git
$ git submodule add git@github.com:tuupola/hagl.git
```

You can alter display behaviour via `menuconfig`. If you choose to use back buffer all drawing operations will be fast. Downside is that back buffer requires lot of memory. To reduce flickering you can also choose to lock back buffer while flushing. Locking will slow down draw operations though.

```
$ make menuconfig
```

For example usage see [ESP GFX](https://github.com/tuupola/esp_gfx), [ESP effects](https://github.com/tuupola/esp_effects) and [Mandelbrot](https://github.com/tuupola/esp-examples/tree/master/014-mandelbrot).

## License

The MIT License (MIT). Please see [License File](LICENSE.md) for more information.
