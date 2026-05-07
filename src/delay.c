#include "at32f403a_407_tmr.h"
#include "at32f403a_407_wdt.h"

#include "delay.h"

void delay_init(void)
{
    crm_periph_clock_enable(CRM_TMR7_PERIPH_CLOCK,1);
    tmr_base_init(TMR7, 0xffff, 48 - 1);
    tmr_cnt_dir_set(TMR7, TMR_COUNT_UP);
    tmr_counter_enable(TMR7, TRUE);
}

void delay_us(uint_fast32_t us)
{
    TMR7->cval = 0;
    while (TMR7->cval < us) { }
}

void delay_ms(uint_fast32_t ms)
{
    while (ms--)
    {
        delay_us(1000);
        wdt_counter_reload();
    }
}
