#include <stdbool.h>
#include <stdio.h>

#ifdef AT32F403ACGT7
#include "at32f403a_407_conf.h"
#endif

#include "lvgl.h"

#include "beep.h"
#include "delay.h"
#include "display.h"
#include "keypad.h"
#include "lv_theme_zt.h"
#include "power.h"
#include "screen.h"
#include "tester.h"
#include "tick.h"
#ifdef AT32F403ACGT7
#include "misc.h"

static void init_crm(void)
{
    crm_reset();
    crm_clock_source_enable(CRM_CLOCK_SOURCE_HICK, TRUE);
    crm_hick_sclk_frequency_select(CRM_HICK_SCLK_48MHZ);
    crm_ahb_div_set(CRM_AHB_DIV_1);
    crm_apb2_div_set(CRM_APB2_DIV_1);
    crm_apb1_div_set(CRM_APB1_DIV_1);
    crm_adc_clock_div_set(CRM_ADC_DIV_2);
    crm_auto_step_mode_enable(TRUE);
    crm_sysclk_switch(CRM_SCLK_HICK);
    while (crm_sysclk_switch_status_get() != CRM_SCLK_HICK) { }
    crm_auto_step_mode_enable(FALSE);
    system_core_clock_update();
    crm_periph_clock_enable(CRM_GPIOA_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOB_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOC_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_GPIOD_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_IOMUX_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_DMA1_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_DMA2_PERIPH_CLOCK, TRUE);
    crm_periph_clock_enable(CRM_CRC_PERIPH_CLOCK, TRUE);
}

static void init_gpio(void)
{
    gpio_pin_remap_config(SWJTAG_GMUX_010, TRUE);
    gpio_bits_write(GPIOB, GPIO_PINS_0, 0);
    gpio_bits_write(GPIOB, GPIO_PINS_1, 0);
    gpio_init_simple(GPIOB, GPIO_PINS_0, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_1, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
}

static void init_wdt(void)
{
    wdt_register_write_enable(TRUE);
    wdt_divider_set(WDT_CLK_DIV_32);
    wdt_reload_value_set(2500);
    wdt_counter_reload();
    wdt_enable();
}
#endif

void key_event_cb(lv_event_t *e)
{
    lv_indev_t *indev = (lv_indev_t *)lv_event_get_target(e);
    keypad_t key = lv_indev_get_key(indev);
    lv_event_code_t code = lv_event_get_code(e);

    if (   (code == LV_EVENT_LONG_PRESSED)
        && (key == KEYPAD_POWER) )
    {
        power_set(false);
    }

    lv_obj_t *scr = ((screen_user_data_t *)lv_obj_get_user_data(lv_screen_active()))->handle_key(code, key);
    if (scr != lv_screen_active())
    {
        lv_screen_load(scr);
        ((screen_user_data_t *)lv_obj_get_user_data(scr))->enter();
    }
}

int main(void)
{
#ifdef AT32F403ACGT7
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_vector_table_set(0x8000000, 0x6000);

    init_crm();
    init_gpio();
    init_wdt();
#endif
    power_init();
    keypad_init();
    beep_init();
    delay_init();

    /* only pull power high if power button has been pressed >1s */
    delay_ms(1000);
    power_set(true);

    tick_init();

    lv_init();

    lv_display_t *lcd_disp = display_init();
    lv_theme_t *th = lv_theme_zt_init(lcd_disp);
    lv_display_set_theme(lcd_disp, th);

    screen_dmm_create();
    screen_tester_create();
    screen_tool_create();

    lv_timer_handler();
    do
    {
#ifdef AT32F403ACGT7
        wdt_counter_reload();
#endif
    }
    while (tester_check_update());

#ifdef AT32F403ACGT7
    /* wait for power button to be released before setting up LVGL keypad handling */
    while (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_1))
    {
        wdt_counter_reload();
    }
#endif

    lv_screen_load(screen_dmm);
    ((screen_user_data_t *)lv_obj_get_user_data(lv_screen_active()))->enter();

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, keypad_read);
    lv_indev_add_event_cb(indev, key_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_indev_add_event_cb(indev, key_event_cb, LV_EVENT_LONG_PRESSED, NULL);

    beep_short();

    while (true)
    {
#ifdef AT32F403ACGT7
        wdt_counter_reload();
#endif
        lv_timer_handler();
        ((screen_user_data_t *)lv_obj_get_user_data(lv_screen_active()))->update();
    }
    return 0;
}

void tick_cb(void)
{
    lv_tick_inc(1000 / TICK_HZ);
}
