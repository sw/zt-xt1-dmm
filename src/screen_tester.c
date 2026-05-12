#include "beep.h"
#include "screen.h"

lv_obj_t *screen_tester;

static void screen_tester_enter(void)
{
}

static lv_obj_t *screen_tester_handle_key(lv_event_code_t event_code, keypad_t key)
{
    if (event_code == LV_EVENT_SHORT_CLICKED)
    {
        switch (key)
        {
            case KEYPAD_UP:
                beep_short();
                //tester_tx.app.data[0] = tester_zener_enable;
                //tester_send(1,1);
                break;

            case KEYPAD_DOWN:
                beep_short();
                //tester_zener_enable = 0;
                //gpio_bits_write((uint *)&GPIOB,2,0);
                //return screen_tool;
                break;
        }
    }
    else if (event_code == LV_EVENT_LONG_PRESSED)
    {
        switch (key)
        {
            case KEYPAD_UP:
                beep_short();
                //tester_zener_enable = 0;
                //gpio_bits_write((uint *)&GPIOB,2,0);
                return screen_dmm;

            case KEYPAD_DOWN:
                beep_short();
                //tester_zener_enable = tester_zener_enable == 0;
                //gpio_bits_write((uint *)&GPIOB,2,(uint)tester_zener_enable);
                //tester_switchcase();
                break;

            case KEYPAD_OK_MENU_HOLD:
                beep_short();
                //tester_zener_enable = 0;
                //gpio_bits_write((uint *)&GPIOB,2,0);
                //DAT_200003fc = 1;
                //return screen_setup;
                break;
        }
    }
    return screen_tester;
}

static void screen_tester_update(void)
{
}

void screen_tester_create(void)
{
    screen_tester = lv_obj_create(NULL);

    lv_obj_t *title = lv_label_create(screen_tester);
    lv_label_set_text_static(title, "Tester");
    lv_obj_set_width(title, LV_PCT(100));

    static screen_user_data_t ud =
    {
        .enter      = screen_tester_enter,
        .handle_key = screen_tester_handle_key,
        .update     = screen_tester_update,
    };
    lv_obj_set_user_data(screen_tester, &ud);
}
