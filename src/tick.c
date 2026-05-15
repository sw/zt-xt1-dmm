#include "at32f403a_407_misc.h"

#include "tick.h"

void tick_init(void)
{
    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    SysTick_Config(system_core_clock / TICK_HZ);
}

void SysTick_Handler(void)
{
    tick_cb();
}
