#include "beep.h"
#include "screen.h"
#include "tester.h"

lv_obj_t *screen_tester;
static lv_obj_t *component;
static lv_obj_t *subtype;
static lv_obj_t *symbol;
static lv_obj_t *probes[3];
static lv_obj_t *values;

extern const lv_image_dsc_t img_bjt_npn;
extern const lv_image_dsc_t img_bjt_pnp;

extern const lv_image_dsc_t img_probe1;
extern const lv_image_dsc_t img_probe2;
extern const lv_image_dsc_t img_probe3;

static const lv_image_dsc_t *img_probes[] = { &img_probe1, &img_probe2, &img_probe3 };

static void screen_tester_enter(void)
{
    tester_init();
}

static lv_obj_t *screen_tester_handle_key(lv_event_code_t event_code, keypad_t key)
{
    if (event_code == LV_EVENT_SHORT_CLICKED)
    {
        switch (key)
        {
            case KEYPAD_UP:
                beep_short();
                tester_send(1, 0/* tester_zener_enable */);
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

static void component_bjt(const tester_result_t *result)
{
    lv_label_set_text_static(component, "BJT");
    lv_label_set_text_static(subtype, result->junction == JUNCTION_NPN ? "NPN" : "PNP");

    lv_obj_set_pos(symbol, 57, 115);
    lv_image_set_src(symbol, result->junction == JUNCTION_NPN ? &img_bjt_npn : &img_bjt_pnp);

    lv_obj_set_pos(probes[0], 23, 143);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[1], 133, 103);
    lv_image_set_src(probes[1], img_probes[result->probes[1]]);
    lv_obj_set_pos(probes[2], 133, 183);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    lv_label_set_text_fmt(values,
        "hFE = %.0f\n"
        "Vbe = %.2fV\n"
        "Ic = %.2fμA",
        result->hfe,
        result->bjt_ube,
        result->ic_mA * 1000.0f);
    /* TODO: Vf */
}

static void screen_tester_update(void)
{
    tester_result_t result;
    if (!tester_get(&result))
    {
        return;
    }

    if (result.component != COMPONENT_TESTING)
    {
        beep_short();
    }

    switch (result.component)
    {
        default:
        case COMPONENT_NONE:
            lv_label_set_text_static(component, "Unknown or damaged part");
            break;
        case COMPONENT_TESTING:
            lv_label_set_text_static(component, "Testing...");
            break;
        case COMPONENT_JFET:
            lv_label_set_text_static(component, "JFET");
            break;
        case COMPONENT_DMOS:
            lv_label_set_text_static(component, "DMOS");
            break;
        case COMPONENT_BJT:        component_bjt(&result); break;
        case COMPONENT_DARLINGTON:
            lv_label_set_text_static(component, "Darlington");
            break;
        case COMPONENT_UJT:
            lv_label_set_text_static(component, "UJT");
            break;
        case COMPONENT_EMOS:
            lv_label_set_text_static(component, "EMOS");
            break;
        case COMPONENT_IGBT:
            lv_label_set_text_static(component, "IGBT");
            break;
        case COMPONENT_THYRISTOR:
            lv_label_set_text_static(component, "Thyristor");
            break;
        case COMPONENT_TRIAC:
            lv_label_set_text_static(component, "TRIAC");
            break;
        case COMPONENT_DIODE:
            lv_label_set_text_static(component, "Diode");
            break;
        case COMPONENT_2DIODE:
            lv_label_set_text_static(component, "Dual Diode");
            break;
        case COMPONENT_BATTERY:
            lv_label_set_text_static(component, "Battery");
            break;
        case COMPONENT_CAP:
            lv_label_set_text_static(component, "Capacitor");
            break;
        case COMPONENT_RESISTOR:
            lv_label_set_text_static(component, "Resistor");
            break;
        case COMPONENT_INDUCTOR:
            lv_label_set_text_static(component, "Inductor");
            break;
        case COMPONENT_ZENER:
            lv_label_set_text_static(component, "Zener Diode");
            break;
    }
}

void screen_tester_create(void)
{
    screen_tester = lv_obj_create(NULL);

    lv_obj_t *rows = lv_obj_create(screen_tester);
    lv_obj_set_size(rows, lv_pct(100), lv_pct(100));
    lv_obj_set_flex_flow(rows, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rows, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_START);

    lv_obj_t *title = lv_label_create(rows);
    lv_label_set_text_static(title, "Tester");
    lv_obj_set_width(title, LV_PCT(100));

    component = lv_label_create(rows);
    lv_label_set_text_static(component, "");
    lv_obj_set_width(component, LV_PCT(100));

    subtype = lv_label_create(rows);
    lv_label_set_text_static(subtype, "");
    lv_obj_set_width(subtype, LV_PCT(100));

    symbol = lv_image_create(screen_tester);
    probes[0] = lv_image_create(screen_tester);
    probes[1] = lv_image_create(screen_tester);
    probes[2] = lv_image_create(screen_tester);

    /* TODO: we just want the value texts vertically centered, is there an easier way? */
    lv_obj_t *values_container = lv_obj_create(rows);
    lv_obj_set_width(values_container, lv_pct(33));
    lv_obj_set_flex_grow(values_container, 1);
    lv_obj_set_flex_flow(values_container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(values_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    values = lv_label_create(values_container);
    lv_label_set_text_static(values, "");
    lv_obj_set_width(values, lv_pct(100));
    lv_obj_set_style_text_align(values, LV_TEXT_ALIGN_LEFT, 0);

    static screen_user_data_t ud =
    {
        .enter      = screen_tester_enter,
        .handle_key = screen_tester_handle_key,
        .update     = screen_tester_update,
    };
    lv_obj_set_user_data(screen_tester, &ud);
}
