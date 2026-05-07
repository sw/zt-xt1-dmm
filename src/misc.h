#pragma once

#include <stdint.h>
#include "at32f403a_407_gpio.h"

#if defined NDEBUG && defined __OPTIMIZE__ && __GNUC__ >= 5
  /* this does nothing in optimized release builds except provide optimization hints */
  #define assert(c) if (!(c)) __builtin_unreachable()
#else
  #include <assert.h>
#endif

void gpio_init_simple(gpio_type *gpio_x, uint_fast32_t pins, gpio_mode_type mode, gpio_pull_type pull);

void print(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

void fatal(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
