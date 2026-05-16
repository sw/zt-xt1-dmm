#include "beep.h"
#include "screen.h"
#include "tester.h"

lv_obj_t *screen_tool;
static lv_group_t *group;
static lv_obj_t *info;
static lv_obj_t *probes[2];
static lv_obj_t *r_l;

typedef struct
{
    const char *text;
    tool_t tool;
    void (*update_cb)(const tester_result_t *result);
} item_t;

static void tool_resistor(const tester_result_t *result)
{
    if (result->component == COMPONENT_RESISTOR)
    {
        lv_image_set_src(probes[0], img_probes[result->probes[0]]);
        lv_image_set_src(probes[1], img_probes[result->probes[2]]);
        lv_image_set_src(r_l, &img_resistor);
        resistor_fmt(info, result->resistance);
    }
}

static void tool_inductor(const tester_result_t *result)
{
    if (result->component == COMPONENT_INDUCTOR)
    {
        lv_image_set_src(probes[0], img_probes[result->probes[0]]);
        lv_image_set_src(probes[1], img_probes[result->probes[2]]);
        lv_image_set_src(r_l, &img_inductor);
        inductor_fmt(info, result->inductance_uH, result->resistance);
    }
}

static void tool_ds18b20(const tester_result_t *result)
{
    if (result->component == COMPONENT_DS18B20)
    {
        lv_label_set_text_fmt(info,
            "%0.1f °C\n"
            "%u:GND\n%u:DQ\n%u:VDD\n"
            "ID:%" PRIX64,
            result->temperature,
            result->probes[0] + 1, result->probes[1] + 1, result->probes[2] + 1,
            *(uint64_t *)result->ds18b20_rom_code);
    }
}

static void tool_dht11(const tester_result_t *result)
{
    if (result->component == COMPONENT_DHT11)
    {
        lv_label_set_text_fmt(info,
            "%0.1f °C  %0.0f%%\n"
            "%u : 1VDD\n%u : 2DATA\n%u : 4GND",
            result->temperature, result->humidity,
            result->probes[0] + 1, result->probes[1] + 1, result->probes[2] + 1);
    }
}

static void tool_infrared(const tester_result_t *result)
{
    if (result->component == COMPONENT_INFRARED)
    {
        lv_label_set_text_fmt(info,
            "UserCode : %04X\n"
            "DataCode : %04X\n"
            "%u : 1OUT\n%u : 2GND\n%u : 3VCC",
            (unsigned int)result->ir_a[1],
            (unsigned int)result->ir_a[2],
            result->probes[0] + 1, result->probes[1] + 1, result->probes[2] + 1);
    }
}

static const item_t items[] =
{
    { .text = "Resistor",  .tool = TOOL_RESISTOR,       .update_cb = tool_resistor },
    { .text = "Inductor",  .tool = TOOL_INDUCTOR,       .update_cb = tool_inductor },
    { .text = "DS18B20" ,  .tool = TOOL_TEMP_DS18B20,   .update_cb = tool_ds18b20  },
    { .text = "DHT11",     .tool = TOOL_TEMP_HUM_DHT11, .update_cb = tool_dht11    },
    { .text = "IR Decode", .tool = TOOL_INFRARED,       .update_cb = tool_infrared },
};

static void screen_clear(void)
{
    lv_label_set_text_static(info, "");
    lv_image_set_src(r_l, NULL);
    lv_image_set_src(probes[0], NULL);
    lv_image_set_src(probes[1], NULL);
}

static void screen_tool_enter(void)
{
    const item_t *item_focus = lv_obj_get_user_data(lv_group_get_focused(group));
    beep_short();
    screen_clear();
    tester_send(3, item_focus->tool);
}

static lv_obj_t *screen_tool_handle_key(lv_event_code_t event_code, keypad_t key)
{
    if (event_code == LV_EVENT_SHORT_CLICKED)
    {
        /* do prev/next manually instead of assigning an indev to avoid long-press repeats */
        switch (key)
        {
            case KEYPAD_UP:
                lv_group_focus_prev(group);
                break;

            case KEYPAD_DOWN:
                lv_group_focus_next(group);
                break;

            case KEYPAD_RETURN:
                beep_short();
                tester_send(3, TOOL_NONE);
                return screen_tester;

            case KEYPAD_OK_MENU_HOLD:
                beep_short();
                /* re-request current tool */
                const item_t *item_focus = lv_obj_get_user_data(lv_group_get_focused(group));
                tester_send(3, item_focus->tool);
                break;
        }
    }
    else if (event_code == LV_EVENT_LONG_PRESSED)
    {
        switch (key)
        {
            case KEYPAD_UP:
                beep_short();
                tester_send(3, TOOL_NONE);
                return screen_dmm;

            case KEYPAD_OK_MENU_HOLD:
                beep_short();
                tester_send(3, TOOL_NONE);
                //return screen_setup;
                break;
        }
    }
    return screen_tool;
}

static void screen_tool_update(void)
{
    tester_result_t result;
    if (!tester_get(&result))
    {
        return;
    }
    screen_clear();
    const item_t *item_focus = lv_obj_get_user_data(lv_group_get_focused(group));
    item_focus->update_cb(&result);
}

static void group_focus_cb(lv_group_t *group)
{
    const item_t *item_focus = lv_obj_get_user_data(lv_group_get_focused(group));
    beep_short();
    tester_send(3, item_focus->tool);
}

void screen_tool_create(void)
{
    screen_tool = lv_obj_create(NULL);

    lv_obj_t *rows = lv_obj_create(screen_tool);
    lv_obj_set_size(rows, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(rows, LV_FLEX_FLOW_COLUMN);

    lv_obj_t *title = lv_label_create(rows);
    lv_label_set_text_static(title, "Tools");
    lv_obj_set_width(title, LV_PCT(100));

    lv_obj_t *columns = lv_obj_create(rows);
    lv_obj_set_width(columns, LV_PCT(100));
    lv_obj_set_flex_grow(columns, 1);
    lv_obj_set_flex_flow(columns, LV_FLEX_FLOW_ROW);

    group = lv_group_create();

    lv_obj_t *list = lv_list_create(columns);
    lv_obj_set_size(list, lv_pct(33), lv_pct(100));

    for (unsigned int i = 0; i < sizeof(items) / sizeof(items[0]); i++)
    {
        lv_obj_t *btn = lv_list_add_button(list, NULL, items[i].text);
        lv_obj_set_flex_grow(btn, 1);
        lv_obj_set_flex_align(btn, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
        lv_obj_set_user_data(btn, (void *)&items[i]);
        lv_group_add_obj(group, btn);
    }

    lv_group_set_focus_cb(group, group_focus_cb);

    info = lv_label_create(columns);
    lv_obj_set_height(info, lv_pct(50));
    lv_obj_set_flex_grow(info, 1);

    probes[0] = lv_image_create(screen_tool);
    lv_obj_set_pos(probes[0], 140, 190);
    probes[1] = lv_image_create(screen_tool);
    lv_obj_set_pos(probes[1], 260, 190);
    r_l = lv_image_create(screen_tool);
    lv_obj_set_pos(r_l, 190, 197);

    static screen_user_data_t ud =
    {
        .enter      = screen_tool_enter,
        .handle_key = screen_tool_handle_key,
        .update     = screen_tool_update,
    };
    lv_obj_set_user_data(screen_tool, &ud);
}
