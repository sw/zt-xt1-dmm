#include <stdbool.h>

#include "at32f403a_407_conf.h"

#include "lvgl.h"

#include "beep.h"
#include "delay.h"
#include "display.h"
#include "dmm.h"
#include "lv_theme_zt.h"
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
    gpio_bits_write(GPIOB, GPIO_PINS_12, 0);
    gpio_init_simple(GPIOB, GPIO_PINS_12, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_0, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
    gpio_init_simple(GPIOB, GPIO_PINS_1, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);

    /* keypad */
    gpio_init_simple(GPIOA, GPIO_PINS_1,  GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOC, GPIO_PINS_13, GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOC, GPIO_PINS_14, GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOA, GPIO_PINS_8,  GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOA, GPIO_PINS_10, GPIO_MODE_INPUT, GPIO_PULL_UP);
    gpio_init_simple(GPIOC, GPIO_PINS_15, GPIO_MODE_INPUT, GPIO_PULL_DOWN);
}

static void init_wdt(void)
{
    wdt_register_write_enable(TRUE);
    wdt_divider_set(WDT_CLK_DIV_32);
    wdt_reload_value_set(2500);
    wdt_counter_reload();
    wdt_enable();
}

#if 0
static void display_write_cmd(uint_fast8_t cmd)
{
    gpio_bits_write(GPIOA,GPIO_PINS_15,0);
    gpio_bits_write(GPIOB,GPIO_PINS_4,0);
    spi_i2s_data_transmit(SPI3, cmd);
    while (spi_i2s_flag_get(SPI3,SPI_I2S_BF_FLAG)) { }
    gpio_bits_write(GPIOB,GPIO_PINS_4,1);
}

static void display_write_byte(uint_fast8_t byte)
{
    spi_i2s_data_transmit(SPI3, byte);
    while (spi_i2s_flag_get(SPI3,SPI_I2S_BF_FLAG)) { }
}

#define SLEEP_OUT 0x11
#define MEMORY_DATA_ACCESS_CONTROL 0x36
#define INTERFACE_PIXEL_FORMAT 0x3A
#define PORCH_SETTING 0xB2
#define LCM_CONTROL 0xC0
#define VDV_VRH_COMMAND_ENABLE 0xC2
#define GATE_CONTROL 0xB7
#define VCOM_SETTING 0xBB
#define VRH_SET 0xC4
#endif

static uint32_t systick_interrupt_config(uint32_t ticks)
{
  if ((ticks - 1UL) > SysTick_LOAD_RELOAD_Msk)
  {
    return (1UL);
  }

  SysTick->LOAD  = (uint32_t)(ticks - 1UL);
  NVIC_SetPriority (SysTick_IRQn, (1UL << __NVIC_PRIO_BITS) - 1UL);
  SysTick->VAL   = 0UL;
  SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk |
                   SysTick_CTRL_ENABLE_Msk;
  return (0UL);
}

typedef enum
{
    KEY_POWER,
    KEY_RETURN,
    KEY_RANGE_REL,
    KEY_OK_MENU_HOLD,
    KEY_UP   = LV_KEY_UP,   // LV_KEY_PREV?
    KEY_DOWN = LV_KEY_DOWN, // LV_KEY_NEXT?
} key_t;

static void keyboard_read(lv_indev_t *indev, lv_indev_data_t *data)
{
    (void)indev;
    if (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_1))
    {
        data->key = KEY_POWER;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else if (!gpio_input_data_bit_read(GPIOC, GPIO_PINS_13))
    {
        data->key = KEY_UP;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else if (!gpio_input_data_bit_read(GPIOC, GPIO_PINS_14))
    {
        data->key = KEY_DOWN;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else if (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_8))
    {
        data->key = KEY_RETURN;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else if (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_10))
    {
        data->key = KEY_RANGE_REL;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else if (!!gpio_input_data_bit_read(GPIOC, GPIO_PINS_15))
    {
        data->key = KEY_OK_MENU_HOLD;
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }
}

static int dmm_mode;
static bool dmm_hold;

void key_event_cb(lv_event_t *e)
{
    lv_indev_t *indev = (lv_indev_t *)lv_event_get_target(e);
    key_t key = lv_indev_get_key(indev);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED)
    {
        switch (key)
        {
            case KEY_POWER:
                beep_short();
                /* TODO: clear statistics */
                // measurement_clear_cnt = 1;
                break;

            case KEY_UP:
                beep_short();
                //measurement_clear_cnt = 10;
                switch (dmm_mode)
                {
                    case 4:  dmm_send(0x80); dmm_mode = 5; break;
                    case 5:  dmm_send(0x90); dmm_mode = 6; break;
                    case 6:  dmm_send(0x90); dmm_mode = 7; break;
                    default: dmm_send(0x80); dmm_mode = 4; break;
                }
                break;

            case KEY_DOWN:
                beep_short();
                //measurement_clear_cnt = 10;
                switch (dmm_mode)
                {
                    case 0:  dmm_send(0x50); dmm_mode = 1; break;
                    case 1:  dmm_send(0x60); dmm_mode = 2; break;
                    case 2:  dmm_send(0x60); dmm_mode = 3; break;
                    default: dmm_send(0x50); dmm_mode = 0; break;
                }
                break;

            case KEY_OK_MENU_HOLD:
                if (dmm_mode != 9)
                {
                    beep_short();
                }
                dmm_hold = !dmm_hold;
                dmm_send(0x10);
                break;

            case KEY_RANGE_REL:
                beep_short();
                dmm_send(0x20);
                break;

            case KEY_RETURN:
                //measurement_clear_cnt = 10;
                switch (dmm_mode)
                {
                    case 8:
                        beep_short();
                        dmm_send(0x40);
                        dmm_mode = 9;
                        break;

                    case 9:
                        dmm_send(0x40);
                        dmm_mode = 10;
                        break;

                    case 10:
                        beep_short();
                        dmm_send(0x40);
                        dmm_mode = 11;
                        break;

                    case 11:
                        beep_short();
                        dmm_send(0xcc);
                        dmm_mode = 12;
                        break;

                    case 13:
                        //measurement_clear_cnt = 10;
                        break;

                    default:
                        beep_short();
                        dmm_send(0x40);
                        dmm_mode = 8;
                        break;
                }
                break;
        }
    }
    else if (code == LV_EVENT_LONG_PRESSED)
    {
        switch (key)
        {
            case KEY_POWER:
                /* power off */
                gpio_bits_write(GPIOB, GPIO_PINS_12, FALSE);
                break;

            case KEY_UP:
                beep_short();
                //tmr_counter_enable(TMR5, 1);
                //tester_enter();
                break;

            case KEY_RANGE_REL:
                beep_short();
                dmm_send(0x30);
                break;

            case KEY_OK_MENU_HOLD:
                beep_short();
                //tmr_counter_enable(&TMR5,1);
                //DAT_200003fc = 3;
                //setup_draw();
                break;

            case KEY_RETURN:
            case KEY_DOWN:
                break;
        }
    }
}

static lv_obj_t *label_create_fixed(lv_obj_t *parent, const char *text)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text_static(label, text);
    lv_obj_update_layout(label);
    lv_obj_set_width(label, lv_obj_get_width(label));
    return label;
}

int main(void)
{
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_vector_table_set(0x8000000, 0x6000);

    init_crm();
    init_gpio();
    beep_init();
    delay_init();
    init_wdt();

    /* only pull power high if power button has been pressed >1s */
    delay_ms(1000);
    gpio_bits_write(GPIOB, GPIO_PINS_12, TRUE);

    lv_init();

    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    systick_interrupt_config(system_core_clock / 100);

    lv_display_t *lcd_disp = display_init();
    lv_theme_t *th = lv_theme_zt_init(lcd_disp);
    lv_display_set_theme(lcd_disp, th);
    lv_obj_t *const scr = lv_screen_active();

    lv_obj_t *rows = lv_obj_create(scr);
    lv_obj_set_size(rows, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(rows, LV_FLEX_FLOW_COLUMN);


    lv_obj_t *top_row = lv_obj_create(rows);
    lv_obj_set_size(top_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(top_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(top_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *hold = label_create_fixed(top_row, "HOLD");
    lv_obj_set_style_text_color(hold, lv_color_make(0xff, 0, 0), 0);

    lv_obj_t *auto_rel = label_create_fixed(top_row, "AUTO");

    lv_obj_t *meas_freq = label_create_fixed(top_row, "0.000Hz");

    lv_obj_t *obj;
    obj = lv_label_create(top_row);
    lv_label_set_text_static(obj, "BATT");


    lv_obj_t *meas_row = lv_obj_create(rows);
    lv_obj_set_size(meas_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(meas_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(meas_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START);

    lv_obj_t *ac_dc = label_create_fixed(meas_row, "DC");
    lv_obj_set_height(ac_dc, lv_pct(100));

    lv_obj_t *meas_main = lv_label_create(meas_row);
    lv_obj_set_style_text_font(meas_main, &noto_sans_semibold_64, 0);
    lv_label_set_text_static(meas_main, "-2.4999");
    lv_obj_update_layout(meas_main);
    lv_obj_set_width(meas_main, lv_obj_get_width(meas_main));
    lv_obj_set_style_text_align(meas_main, LV_TEXT_ALIGN_RIGHT, 0);

    lv_obj_t *meas_unit = label_create_fixed(meas_row, "MΩ");

    obj = lv_label_create(rows);
    lv_obj_set_width(obj, lv_pct(100));
    lv_label_set_text_static(obj, "TODO: statistics");

    obj = lv_label_create(rows);
    lv_obj_set_width(obj, lv_pct(100));
    lv_label_set_text_static(obj, "TODO: graph");
    lv_obj_set_style_bg_color(obj, lv_color_make(0, 0x44, 0), 0); lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_flex_grow(obj, 1);


    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, keyboard_read);
    lv_indev_add_event_cb(indev, key_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_indev_add_event_cb(indev, key_event_cb, LV_EVENT_LONG_PRESSED, NULL);

    dmm_init();

    /* seems to enable DMM chip. TODO: find out what this is exactly */
    gpio_bits_write(GPIOB,1,1);

    beep_short();

    while (true)
    {
        wdt_counter_reload();
        lv_timer_handler();
        delay_ms(20);

        dmm_result_t result;
        if (dmm_get(&result))
        {
            exint_interrupt_enable(EXINT_LINE_13, result.continuity);

            if (dmm_hold)
            {
                lv_obj_set_state(hold, LV_STATE_DISABLED, false);
                continue;
            }
            lv_obj_set_state(hold, LV_STATE_DISABLED, true);

            if (result.auto_range)
            {
                lv_label_set_text_static(auto_rel, "AUTO");
            }
            else if (result.rel)
            {
                lv_label_set_text_static(auto_rel, "REL");
            }
            else if (result.continuity)
            {
                lv_label_set_text_static(auto_rel, "CONT");
            }
            else if (result.diode)
            {
                lv_label_set_text_static(auto_rel, "-|>|-");
            }
            else
            {
                lv_label_set_text_static(auto_rel, "");
            }

            lv_label_set_text_static(meas_freq, "");
            if (result.dc)
            {
                lv_label_set_text_static(ac_dc, "DC");
            }
            else if (result.ac)
            {
                lv_label_set_text_static(ac_dc, "AC");
                const char *fmt = "";
                const char *k = result.khz ? "k" : "";
                if (result.temp_freq < 9.9995f)
                {
                    fmt = "%0.3f%sHz";
                }
                else if (result.temp_freq < 99.995f)
                {
                    fmt = "%0.2f%sHz";
                }
                else if (result.temp_freq < 999.95)
                {
                    fmt = "%0.1f%sHz";
                }
                else if (result.temp_freq < 9999.5)
                {
                    fmt = "%0.0f%sHz";
                }
                lv_label_set_text_fmt(meas_freq, fmt, result.temp_freq, k);
            }
            else
            {
                lv_label_set_text_static(ac_dc, "");
            }

            lv_label_set_text(meas_main, result.main_s);
            lv_label_set_text_fmt(meas_unit, "%s%s", result.prefix, result.unit);
        }
    }
    return 0;
}

void SysTick_Handler(void)
{
    lv_tick_inc(10);
}
