#pragma once

#include <stdint.h>
#ifdef AT32F403ACGT7
#include "at32f403a_407_gpio.h"
#endif

#if defined NDEBUG && defined __OPTIMIZE__ && __GNUC__ >= 5
    /* this does nothing in optimized release builds except provide optimization hints */
    #define assert(c) if (!(c)) __builtin_unreachable()
#else
    #include <assert.h>
#endif

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))

#define MAX(a, b) ({ \
        /* random suffix to avoid naming conflict */ \
        __typeof__(a) _value_a_ = (a); \
        __typeof__(b) _value_b_ = (b); \
        (_value_a_ > _value_b_) ? _value_a_ : _value_b_; \
    })

#define MIN(a, b) ({ \
        /* random suffix to avoid naming conflict */ \
        __typeof__(a) _value_a_ = (a); \
        __typeof__(b) _value_b_ = (b); \
        (_value_a_ < _value_b_) ? _value_a_ : _value_b_; \
    })

#ifdef AT32F403ACGT7
void gpio_init_simple(gpio_type *gpio_x, uint_fast32_t pins, gpio_mode_type mode, gpio_pull_type pull);
#endif

void print(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));

void fatal(const char *fmt, ...) __attribute__ ((format (printf, 1, 2)));
