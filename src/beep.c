#include "at32f403a_407_tmr.h"

#include "beep.h"
#include "delay.h"
#include "misc.h"

void beep_init(void)
{
    tmr_output_config_type tmr_output_struct;

    crm_periph_clock_enable(CRM_TMR12_PERIPH_CLOCK, TRUE);
    tmr_base_init(TMR12, 300, 79);
    tmr_cnt_dir_set(TMR12, TMR_COUNT_UP);
    tmr_clock_source_div_set(TMR12, TMR_CLOCK_DIV1);
    tmr_output_default_para_init(&tmr_output_struct);
    tmr_output_struct.oc_mode = TMR_OUTPUT_CONTROL_PWM_MODE_A;
    tmr_output_struct.oc_idle_state = FALSE;
    tmr_output_struct.oc_output_state = TRUE;
    tmr_output_struct.oc_polarity = TMR_OUTPUT_ACTIVE_HIGH;
    tmr_output_channel_config(TMR12, TMR_SELECT_CHANNEL_1, &tmr_output_struct);
    tmr_channel_value_set(TMR12, TMR_SELECT_CHANNEL_1, 0);
    tmr_output_channel_buffer_enable(TMR12, TMR_SELECT_CHANNEL_1, TRUE);
    tmr_period_buffer_enable(TMR12, TRUE);
    tmr_counter_enable(TMR12, TRUE);
    gpio_init_simple(GPIOB, GPIO_PINS_14, GPIO_MODE_MUX, GPIO_PULL_NONE);
    /* TMR2??? */
    gpio_pin_remap_config(TMR2_GMUX_01, TRUE);
}

void beep_start(void)
{
    const uint32_t volume = 100; /* TODO */
    tmr_channel_value_set(TMR12, TMR_SELECT_CHANNEL_1, volume);
}

void beep_stop(void)
{
    tmr_channel_value_set(TMR12, TMR_SELECT_CHANNEL_1, 0);
}

void beep_short(void)
{
    beep_start();
    delay_ms(50);
    beep_stop();
}
