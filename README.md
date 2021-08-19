# MIPI DCS HAL for HAGL Graphics Library

HAL for HAGL graphics library for display drivers supporting the [MIPI Display Command Set](https://www.mipi.org/specifications/display-command-set). This covers most displays currently used by hobbyists. Tested with ST7735S, ST7789V, ST7789V2, ILI9341, ILI9342C and ILI9163. Works with both ESP32 and ESP32-S2.

[![Software License](https://img.shields.io/badge/license-MIT-brightgreen.svg?style=flat-square)](LICENSE.md)

## Usage

To use with an ESP-IDF project you include this HAL and the [HAGL graphics library](https://github.com/tuupola/hagl) itself.  If you are using CMake based build the HAL **must** be in folder named `hagl_hal`.

```
$ cd components
$ git submodule add git@github.com:tuupola/hagl_esp_mipi.git hagl_hal
$ git submodule add git@github.com:tuupola/hagl.git
```

You can alter display behaviour via `menuconfig`. If you choose to use back buffer all drawing operations will be fast. Downside is that back buffer requires lot of memory. To reduce flickering you can also choose to lock back buffer while flushing. Locking will slow down draw operations though.

```
$ idf.py menuconfig
```

You can also use the older GNU Make based build system.

```
$ make menuconfig
```

[Default configs](https://github.com/tuupola/hagl_esp_mipi/tree/master/sdkconfig/) are provided for popular dev boards. For example to compile for M5Stack do something like the following:

```
$ cp components/hagl_hal/sdkconfig/m5stack.defaults sdkconfig.defaults
$ idf.py menuconfig
```

For example usage see [ESP GFX](https://github.com/tuupola/esp_gfx), [ESP effects](https://github.com/tuupola/esp_effects) and [Mandelbrot](https://github.com/tuupola/esp-examples/tree/master/014-mandelbrot).

## License

The MIT License (MIT). Please see [License File](LICENSE) for more information.
