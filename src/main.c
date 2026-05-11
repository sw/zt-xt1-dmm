#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef AT32F403ACGT7
#include "at32f403a_407_conf.h"
#endif

#include "lvgl.h"

#include "beep.h"
#include "delay.h"
#include "display.h"
#include "dmm.h"
#include "keypad.h"
#include "lv_theme_zt.h"
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
    gpio_bits_write(GPIOB, GPIO_PINS_12, 0);
    gpio_init_simple(GPIOB, GPIO_PINS_12, GPIO_MODE_OUTPUT, GPIO_PULL_NONE);
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
#endif

static int dmm_mode;
static bool dmm_hold;

void key_event_cb(lv_event_t *e)
{
    lv_indev_t *indev = (lv_indev_t *)lv_event_get_target(e);
    keypad_t key = lv_indev_get_key(indev);
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_SHORT_CLICKED)
    {
        switch (key)
        {
            case KEYPAD_POWER:
                beep_short();
                dmm_stat_reset();
                break;

            case KEYPAD_UP:
                beep_short();
                switch (dmm_mode)
                {
                    case 4:  dmm_send(0x80); dmm_mode = 5; break;
                    case 5:  dmm_send(0x90); dmm_mode = 6; break;
                    case 6:  dmm_send(0x90); dmm_mode = 7; break;
                    default: dmm_send(0x80); dmm_mode = 4; break;
                }
                break;

            case KEYPAD_DOWN:
                beep_short();
                switch (dmm_mode)
                {
                    case 0:  dmm_send(0x50); dmm_mode = 1; break;
                    case 1:  dmm_send(0x60); dmm_mode = 2; break;
                    case 2:  dmm_send(0x60); dmm_mode = 3; break;
                    default: dmm_send(0x50); dmm_mode = 0; break;
                }
                break;

            case KEYPAD_OK_MENU_HOLD:
                if (dmm_mode != 9)
                {
                    beep_short();
                }
                dmm_hold = !dmm_hold;
                dmm_send(0x10);
                break;

            case KEYPAD_RANGE_REL:
                beep_short();
                dmm_send(0x20);
                break;

            case KEYPAD_RETURN:
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
                        dmm_stat_reset();
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
            case KEYPAD_POWER:
                /* power off */
#ifdef AT32F403ACGT7
                gpio_bits_write(GPIOB, GPIO_PINS_12, FALSE);
#endif
                break;

            case KEYPAD_UP:
                beep_short();
                //tmr_counter_enable(TMR5, 1);
                //tester_enter();
                break;

            case KEYPAD_RANGE_REL:
                beep_short();
                dmm_send(0x30);
                break;

            case KEYPAD_OK_MENU_HOLD:
                beep_short();
                //tmr_counter_enable(&TMR5,1);
                //DAT_200003fc = 3;
                //setup_draw();
                break;

            case KEYPAD_RETURN:
            case KEYPAD_DOWN:
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

static void measurement_format(float x, char dst[static 8])
{
    float xabs = fabsf(x);
    if (xabs < 2.49995f)
    {
        sprintf(dst, "%0.4f", x);
    }
    else if (xabs < 24.9995f)
    {
        sprintf(dst, "%0.3f", x);
    }
    else if (xabs < 249.995f)
    {
        sprintf(dst, "%0.2f", x);
    }
    else if (xabs < 2499.95f)
    {
        sprintf(dst, "%0.1f", x);
    }
    else if (xabs < 24999.498f)
    {
        sprintf(dst, "%0.0f", x);
    }
    else
    {
        sprintf(dst, "------");
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
    keypad_init();
    beep_init();
    delay_init();

#ifdef AT32F403ACGT7
    /* only pull power high if power button has been pressed >1s */
    delay_ms(1000);
    gpio_bits_write(GPIOB, GPIO_PINS_12, TRUE);

    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    systick_interrupt_config(system_core_clock / 100);
#endif

    lv_init();

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

    lv_obj_t *batt = lv_label_create(top_row);
    lv_label_set_text_static(batt, "BATT"); /* TODO */


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


    lv_obj_t *stat_row = lv_obj_create(rows);
    lv_obj_set_size(stat_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(stat_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(stat_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START);

    lv_obj_t *stat_max = label_create_fixed(stat_row, "Max:-2.4999");

    lv_obj_t *stat_min = label_create_fixed(stat_row, "Min:-2.4999");

    lv_obj_t *stat_avg = label_create_fixed(stat_row, "Avg:-2.4999");


    #define CHART_Y_DIVISION 5
    #define CHART_Y_SCALE (1 << 16)

    lv_obj_t *chart_wrapper = lv_obj_create(rows);
    lv_obj_set_width(chart_wrapper, lv_pct(100));
    lv_obj_set_flex_grow(chart_wrapper, 1);
    lv_obj_set_flex_flow(chart_wrapper, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(chart_wrapper, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *scale = lv_scale_create(chart_wrapper);
    lv_scale_set_mode(scale, LV_SCALE_MODE_VERTICAL_LEFT);
    lv_obj_set_size(scale, lv_pct(19), lv_pct(90));
    lv_scale_set_total_tick_count(scale, CHART_Y_DIVISION);
    lv_scale_set_label_show(scale, true);
    lv_scale_set_major_tick_every(scale, 1);
    lv_scale_set_range(scale, 0, CHART_Y_SCALE);

    lv_obj_t *chart = lv_chart_create(chart_wrapper);
    lv_obj_set_size(chart, lv_pct(81), lv_pct(90));
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
    lv_chart_set_point_count(chart, DMM_STAT_MAX_NB);
    lv_chart_set_div_line_count(chart, CHART_Y_DIVISION, 7);
    lv_chart_set_axis_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, CHART_Y_SCALE);
    lv_chart_series_t *chart_ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_Y);
    int32_t *chart_ser_y_points = lv_chart_get_series_y_array(chart, chart_ser);

#ifdef AT32F403ACGT7
    /* wait for power button to be released before setting up LVGL keypad handling */
    while (!gpio_input_data_bit_read(GPIOA, GPIO_PINS_1))
    {
        wdt_counter_reload();
    }
#endif

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev, keypad_read);
    lv_indev_add_event_cb(indev, key_event_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_indev_add_event_cb(indev, key_event_cb, LV_EVENT_LONG_PRESSED, NULL);

    dmm_init();

    beep_short();

    while (true)
    {
#ifdef AT32F403ACGT7
        wdt_counter_reload();
#endif
        lv_timer_handler();
        delay_ms(20);

        dmm_result_t result;
        if (dmm_get(&result))
        {
#ifdef AT32F403ACGT7
            exint_interrupt_enable(EXINT_LINE_13, result.continuity);
#endif

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

            char stat_s[8];
            measurement_format(result.stat_max, stat_s);
            lv_label_set_text_fmt(stat_max, "Max:%s", stat_s);
            measurement_format(result.stat_min, stat_s);
            lv_label_set_text_fmt(stat_min, "Min:%s", stat_s);
            measurement_format(result.stat_avg, stat_s);
            lv_label_set_text_fmt(stat_avg, "Avg:%s", stat_s);

            uint_fast8_t i = 0;
            if ((result.stat_nb > 1) && (result.stat_max != result.stat_min))
            {
                float scale = CHART_Y_SCALE / (result.stat_max - result.stat_min);
                for (; i < result.stat_nb; i++)
                {
                    chart_ser_y_points[i] = (result.stat_values[i] - result.stat_min) * scale;
                }
            }
            for (; i < DMM_STAT_MAX_NB; i++)
            {
                chart_ser_y_points[i] = LV_CHART_POINT_NONE;
            }
            lv_chart_refresh(chart);

            static char scale_label[CHART_Y_DIVISION][8];
            static const char *scale_label_p[CHART_Y_DIVISION + 1] = { NULL };
            for (i = 0; i < CHART_Y_DIVISION; i++)
            {
                measurement_format(result.stat_min + (result.stat_max - result.stat_min) * i / (CHART_Y_DIVISION - 1), scale_label[i]);
                scale_label_p[i] = scale_label[i];
            }
            lv_scale_set_text_src(scale, scale_label_p);
        }
    }
    return 0;
}

void SysTick_Handler(void)
{
    lv_tick_inc(10);
}
