#include <stdio.h>

#include "beep.h"
#include "misc.h"
#include "screen.h"
#include "tester.h"

lv_obj_t *screen_tester;
static lv_obj_t *component;
static lv_obj_t *subtype;
static lv_obj_t *symbol[2];
static lv_obj_t *probes[3];
static lv_obj_t *values;

static bool zener_enabled;

extern const lv_image_dsc_t img_njfet;
extern const lv_image_dsc_t img_pjfet;

extern const lv_image_dsc_t img_nmos;
extern const lv_image_dsc_t img_pmos;

extern const lv_image_dsc_t img_bjt_npn;
extern const lv_image_dsc_t img_bjt_pnp;

extern const lv_image_dsc_t img_igbt;
extern const lv_image_dsc_t img_thyristor;
extern const lv_image_dsc_t img_triac;

extern const lv_image_dsc_t img_diode;
extern const lv_image_dsc_t img_edoid;
extern const lv_image_dsc_t img_zener;
extern const lv_image_dsc_t img_capacitor;
extern const lv_image_dsc_t img_resistor;
extern const lv_image_dsc_t img_inductor;

extern const lv_image_dsc_t img_probe1;
extern const lv_image_dsc_t img_probe2;
extern const lv_image_dsc_t img_probe3;
extern const lv_image_dsc_t img_probea;
extern const lv_image_dsc_t img_probek;

static const lv_image_dsc_t *img_probes[] = { &img_probe1, &img_probe2, &img_probe3 };

static void screen_clear(void)
{
    lv_label_set_text_static(component, "");
    lv_label_set_text_static(subtype, "");
    lv_label_set_text_static(values, "");
    lv_image_set_src(symbol[0], NULL);
    if (zener_enabled)
    {
        lv_obj_set_pos(symbol[1], 130, 202);
        lv_image_set_src(symbol[1], &img_zener);
    }
    else
    {
        lv_image_set_src(symbol[1], NULL);
    }
    lv_image_set_src(probes[0], NULL);
    lv_image_set_src(probes[1], NULL);
    lv_image_set_src(probes[2], NULL);
}

static void screen_tester_enter(void)
{
    tester_init();
    screen_clear();
}

static lv_obj_t *screen_tester_handle_key(lv_event_code_t event_code, keypad_t key)
{
    if (event_code == LV_EVENT_SHORT_CLICKED)
    {
        switch (key)
        {
            case KEYPAD_UP:
                beep_short();
                tester_send(1, zener_enabled);
                break;

            case KEYPAD_DOWN:
                beep_short();
                zener_enabled = false;
                tester_zener_enable(zener_enabled);
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
                zener_enabled = false;
                tester_zener_enable(zener_enabled);
                return screen_dmm;

            case KEYPAD_DOWN:
                beep_short();
                zener_enabled = !zener_enabled;
                tester_zener_enable(zener_enabled);
                screen_clear();
                break;

            case KEYPAD_OK_MENU_HOLD:
                beep_short();
                zener_enabled = false;
                tester_zener_enable(zener_enabled);
                //DAT_200003fc = 1;
                //return screen_setup;
                break;
        }
    }
    return screen_tester;
}

static void component_jfet(const tester_result_t *result)
{
    lv_label_set_text_static(component, "JFET");
    lv_label_set_text_static(subtype, result->channel == CHANNEL_P ? "P-channel" : "N-channel");

    lv_obj_set_pos(symbol[0], 57, 115);
    lv_image_set_src(symbol[0], result->channel == CHANNEL_P ? &img_pjfet : &img_njfet);

    lv_obj_set_pos(probes[0], 23, 143);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[1], 133, 103);
    lv_image_set_src(probes[1], img_probes[result->probes[1]]);
    lv_obj_set_pos(probes[2], 133, 183);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    lv_label_set_text_fmt(values,
        "I = %0.1fmA\nUg = %0.2fV",
        result->current_mA,
        result->jfet_ug);
}

static void component_mosfet(const tester_result_t *result)
{
    lv_label_set_text_static(component, result->component == COMPONENT_DMOS ? "Depletion-mode MOSFET" : "Enhancement-mode MOSFET");
    /* TODO: different image for depletion-mode */
    lv_label_set_text_static(subtype, result->channel == CHANNEL_P ? "P-channel" : "N-channel");

    lv_obj_set_pos(symbol[0], 57, 115);
    lv_image_set_src(symbol[0], result->channel == CHANNEL_P ? &img_pmos : &img_nmos);

    lv_obj_set_pos(probes[0], 23, 168);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[1], 123, 93);
    lv_image_set_src(probes[1], img_probes[result->probes[1]]);
    lv_obj_set_pos(probes[2], 123, 189);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    char cap_s[16] = { 0 };
    if (result->capacitance_pF < 1e3f)
    {
        sprintf(cap_s, "\nCg = %0.0fpF", result->capacitance_pF);
    }
    else if (result->capacitance_pF < 1e6f)
    {
        sprintf(cap_s, "\nCg = %0.2fnF", result->capacitance_pF / 1e3f);
    }

    if (result->component == COMPONENT_DMOS)
    {
        lv_label_set_text_fmt(values,
            "UF = %0.2fV\nUgs = %0.2fV\nIdss = %0.0fmA\nRds = %0.2fΩ%s",
            result->diode_vf,
            result->dmos_ugs,
            result->current_mA,
            result->resistance,
            cap_s);
    }
    else
    {
        lv_label_set_text_fmt(values,
            "UF = %0.2fV\nUT = %0.2fV\nRds = %0.2fΩ%s",
            result->diode_vf,
            result->emos_uth,
            result->resistance,
            cap_s);
    }
}

static void component_bjt(const tester_result_t *result)
{
    lv_label_set_text_static(component, result->component == COMPONENT_DARLINGTON ? "Darlington" : "BJT");
    /* TODO: different image for Darlington */
    lv_label_set_text_static(subtype, result->junction == JUNCTION_PNP ? "PNP" : "NPN");

    lv_obj_set_pos(symbol[0], 57, 115);
    lv_image_set_src(symbol[0], result->junction == JUNCTION_PNP ? &img_bjt_pnp : &img_bjt_npn);

    lv_obj_set_pos(probes[0], 23, 143);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[1], 133, 103);
    lv_image_set_src(probes[1], img_probes[result->probes[1]]);
    lv_obj_set_pos(probes[2], 133, 183);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    char uf_s[16] = { 0 };
    if (result->bd)
    {
        sprintf(uf_s, "\nUf = %0.2fV", result->diode_vf);
    }

    lv_label_set_text_fmt(values,
        "hFE = %.0f\nUbe = %.2fV\nIc = %.2fμA%s",
        result->hfe,
        result->bjt_ube,
        result->current_mA * 1e3f,
        uf_s);
}

static void component_igbt(const tester_result_t *result)
{
    lv_label_set_text_static(component, "IGBT");

    lv_obj_set_pos(symbol[0], 57, 115);
    lv_image_set_src(symbol[0], &img_igbt);

    lv_obj_set_pos(probes[0], 23, 143);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[1], 133, 103);
    lv_image_set_src(probes[1], img_probes[result->probes[1]]);
    lv_obj_set_pos(probes[2], 133, 183);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    char cap_s[16] = { 0 };
    if (result->capacitance_pF < 1e3f)
    {
        sprintf(cap_s, "\nCg = %0.0fpF", result->capacitance_pF);
    }
    else if (result->capacitance_pF < 1e6f)
    {
        sprintf(cap_s, "\nCg = %0.2fnF", result->capacitance_pF / 1e3f);
    }

    char uf_s[16] = { 0 };
    if (result->bd)
    {
        sprintf(uf_s, "Uf = %0.2fV\n", result->diode_vf);
    }

    lv_label_set_text_fmt(values,
        "%sUT = %0.2fV\nIc = %0.1fμA%s",
        uf_s,
        result->emos_uth,
        result->current_mA * 1e3f,
        cap_s);
}

static void component_thy_triac(const tester_result_t *result)
{
    lv_label_set_text_static(component, result->component == COMPONENT_TRIAC ? "TRIAC" : "Thyristor");

    lv_obj_set_pos(symbol[0], 57, 88 + 17 * (result->component == COMPONENT_THYRISTOR));
    lv_image_set_src(symbol[0], result->component == COMPONENT_TRIAC ? &img_triac : &img_thyristor);

    lv_obj_set_pos(probes[1], 23, 120);
    lv_image_set_src(probes[1], img_probes[result->probes[1]]);
    lv_obj_set_pos(probes[0], 156, 175);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[2], 177, 120);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    lv_label_set_text_fmt(values,
        "UG = %0.2fV\nUT = %0.2fV",
        result->thy_ug,
        result->diode_vf);
}

static void component_diode(const tester_result_t *result)
{
    lv_label_set_text_static(component, "Diode");

    lv_obj_set_pos(symbol[0], 75, 133);
    lv_image_set_src(symbol[0], result->probes[2] < result->probes[0] ? &img_diode : &img_edoid);

    lv_obj_set_pos(probes[0], 156, 126);
    lv_image_set_src(probes[0], img_probes[MAX(result->probes[0], result->probes[2])]);
    lv_obj_set_pos(probes[2], 20, 126);
    lv_image_set_src(probes[2], img_probes[MIN(result->probes[0], result->probes[2])]);

    char cap_s[16] = { 0 };
    if (result->capacitance_pF > 0.0f)
    {
        sprintf(cap_s, "\nC = %0.0fpF", result->capacitance_pF);
    }

    char ir_s[16] = { 0 };
    if (result->current_mA > 0.0f)
    {
        if (result->current_mA < 0.0001f)
        {
            sprintf(ir_s, "\nIr = %0.0fnA", result->current_mA * 1e6f);
        }
        else
        {
            sprintf(ir_s, "\nIr = %0.1fμA", result->current_mA * 1e3f);
        }
    }

    lv_label_set_text_fmt(values,
        "Uf = %0.2fV%s%s",
        result->diode_vf,
        cap_s,
        ir_s);
}

static void component_diode2(const tester_result_t *result)
{
    lv_label_set_text_static(component, "Dual Diode");

    lv_obj_set_pos(probes[0], 10, 126);
    lv_image_set_src(probes[0], img_probes[0]);

    lv_obj_set_pos(probes[1], 88, 126);
    lv_image_set_src(probes[1], img_probes[1]);

    lv_obj_set_pos(probes[2], 166, 126);
    lv_image_set_src(probes[2], img_probes[2]);

    char s[2][10] = { 0 };
    int i = 0;
    /* TODO: paint lines */
    if (result->diode_vf_a[0] < 4.8f)
    {
        lv_obj_set_pos(symbol[i], 35, 133);
        lv_image_set_src(symbol[i], &img_edoid);
        sprintf(s[i], "1-2 %.2fV", result->diode_vf_a[0]);
        i++;
    }
    if (result->diode_vf_a[1] < 4.8f)
    {
        lv_obj_set_pos(symbol[i], 40, 133);
        lv_image_set_src(symbol[i], &img_diode);
        sprintf(s[i], "2-1 %.2fV", result->diode_vf_a[1]);
        i++;
    }
    if (result->diode_vf_a[2] < 4.8f)
    {
        lv_obj_set_pos(symbol[i], 75, 96);
        lv_image_set_src(symbol[i], &img_edoid);
        sprintf(s[i], "1-3 %.2fV", result->diode_vf_a[2]);
        i++;
    }
    if (result->diode_vf_a[3] < 4.8f)
    {
        lv_obj_set_pos(symbol[i], 79, 96);
        lv_image_set_src(symbol[i], &img_diode);
        sprintf(s[i], "3-1 %.2fV", result->diode_vf_a[3]);
        i++;
    }
    if (result->diode_vf_a[4] < 4.8f)
    {
        lv_obj_set_pos(symbol[i], 115, 133);
        lv_image_set_src(symbol[i], &img_edoid);
        sprintf(s[i], "2-3 %.2fV", result->diode_vf_a[4]);
        i++;
    }
    if (result->diode_vf_a[5] < 4.8f)
    {
        lv_obj_set_pos(symbol[i], 118, 133);
        lv_image_set_src(symbol[i], &img_diode);
        sprintf(s[i], "3-2 %.2fV", result->diode_vf_a[5]);
        i++;
    }

    lv_label_set_text_fmt(values, "%s\n%s", s[0], s[1]);
}

static void component_capacitor(const tester_result_t *result)
{
    lv_label_set_text_static(component, "Capacitor");

    lv_obj_set_pos(symbol[0], 94, 128);
    lv_image_set_src(symbol[0], &img_capacitor);

    lv_obj_set_pos(probes[0], 20, 126);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[2], 156, 126);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    char cap_s[16] = { 0 };
    if (result->capacitance_pF < 1e3f)
    {
        sprintf(cap_s, "%0.1fpF", result->capacitance_pF);
    }
    else if (result->capacitance_pF < 10e3f)
    {
        sprintf(cap_s, "%0.0fpF", result->capacitance_pF);
    }
    else if (result->capacitance_pF < 1e6f)
    {
        sprintf(cap_s, "%0.1fnF", result->capacitance_pF / 1e3);
    }
    else if (result->capacitance_pF < 10e6f)
    {
        sprintf(cap_s, "%0.0fnF", result->capacitance_pF / 1e3);
    }
    else if (result->capacitance_pF < 1e9f)
    {
        sprintf(cap_s, "%0.1fμF", result->capacitance_pF / 1e6);
    }
    else if (result->capacitance_pF < 10e9f)
    {
        sprintf(cap_s, "%0.0fμF", result->capacitance_pF / 1e6);
    }
    else if (result->capacitance_pF < 100e9f)
    {
        sprintf(cap_s, "%0.2fmF", result->capacitance_pF / 1e9);
    }
    else if (result->capacitance_pF < 1e12f)
    {
        /* shouldn't happen? */
        sprintf(cap_s, "%0.2fF", result->capacitance_pF / 1e12);
    }

    char vloss_s[16] = { 0 };
    if (result->cap_vloss > 0.0f)
    {
        sprintf(vloss_s, "\nVloss = %0.1f%%", result->cap_vloss);
    }

    char esr_s[16] = { 0 };
    if (result->resistance < 1.0f)
    {
        sprintf(esr_s, "\nESR = %0.2fΩ", result->resistance);
    }
    else if (result->resistance < 100.0f)
    {
        sprintf(esr_s, "\nESR = %0.1fΩ", result->resistance);
    }

    lv_label_set_text_fmt(values, "%s%s%s", cap_s, vloss_s, esr_s);
}

static void component_resistor(const tester_result_t *result)
{
    lv_label_set_text_static(component, "Resistor");

    lv_obj_set_pos(symbol[0], 77, 133);
    lv_image_set_src(symbol[0], &img_resistor);

    lv_obj_set_pos(probes[0], 20, 126);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[2], 156, 126);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    if (result->resistance < 10.0f)
    {
        lv_label_set_text_fmt(values, "%0.2fΩ", result->resistance);
    }
    else if (result->resistance < 1e3f)
    {
        lv_label_set_text_fmt(values, "%0.1fΩ", result->resistance);
    }
    else if (result->resistance < 10e3f)
    {
        lv_label_set_text_fmt(values, "%0.0fΩ", result->resistance);
    }
    else if (result->resistance < 100e3f)
    {
        lv_label_set_text_fmt(values, "%0.2fkΩ", result->resistance / 1e3f);
    }
    else if (result->resistance < 1e6f)
    {
        lv_label_set_text_fmt(values, "%0.1fkΩ", result->resistance / 1e3f);
    }
    else if (result->resistance < 10e6f)
    {
        lv_label_set_text_fmt(values, "%0.2fMΩ", result->resistance / 1e6f);
    }
    else if (result->resistance < 100e6f)
    {
        lv_label_set_text_fmt(values, "%0.1fMΩ", result->resistance / 1e6f);
    }
}

static void component_inductor(const tester_result_t *result)
{
    lv_label_set_text_static(component, "Inductor");

    lv_obj_set_pos(symbol[0], 75, 137);
    lv_image_set_src(symbol[0], &img_inductor);

    lv_obj_set_pos(probes[0], 20, 126);
    lv_image_set_src(probes[0], img_probes[result->probes[0]]);
    lv_obj_set_pos(probes[2], 156, 126);
    lv_image_set_src(probes[2], img_probes[result->probes[2]]);

    char r_s[16] = { 0 };
    if (result->resistance < 1e3f)
    {
        sprintf(r_s, "\nR = %0.1fΩ", result->resistance);
    }

    if (result->inductance_uH < 100e3f)
    {
        lv_label_set_text_fmt(values, "%0.2fmH%s", result->inductance_uH / 1e3f, r_s);
    }
    else
    {
        lv_label_set_text_fmt(values, "%0.2fH%s", result->inductance_uH / 1e6f, r_s);
    }
}

static void component_zener(const tester_result_t *result)
{
    lv_label_set_text_static(component, "Zener");

    lv_obj_set_pos(symbol[0], 75, 124);
    lv_image_set_src(symbol[0], &img_zener);

    lv_obj_set_pos(probes[0], 20, 126);
    lv_image_set_src(probes[0], &img_probek);
    lv_obj_set_pos(probes[2], 156, 126);
    lv_image_set_src(probes[2], &img_probea);

    lv_label_set_text_fmt(values, "Uz = %0.1fV", result->diode_vf);
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

    screen_clear();

    switch (result.component)
    {
        default:
        case COMPONENT_NONE:
            lv_label_set_text_static(component, "Unknown or damaged part");
            break;
        case COMPONENT_TESTING:
            lv_label_set_text_static(component, "Testing...");
            break;
        case COMPONENT_JFET:       component_jfet(&result);      break;
        case COMPONENT_DMOS:
        case COMPONENT_EMOS:       component_mosfet(&result);    break;
        case COMPONENT_BJT:
        case COMPONENT_DARLINGTON: component_bjt(&result);       break;
        case COMPONENT_UJT:
            lv_label_set_text_static(component, "UJT");
            break;
        case COMPONENT_IGBT:       component_igbt(&result);      break;
        case COMPONENT_THYRISTOR:
        case COMPONENT_TRIAC:      component_thy_triac(&result); break;
        case COMPONENT_DIODE:      component_diode(&result);     break;
        case COMPONENT_2DIODE:     component_diode2(&result);    break;
        case COMPONENT_BATTERY:
            lv_label_set_text_static(component, "Battery");
            break;
        case COMPONENT_CAP:        component_capacitor(&result); break;
        case COMPONENT_RESISTOR:   component_resistor(&result);  break;
        case COMPONENT_INDUCTOR:   component_inductor(&result);  break;
        case COMPONENT_ZENER:      component_zener(&result);     break;
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

    symbol[0] = lv_image_create(screen_tester);
    symbol[1] = lv_image_create(screen_tester);
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
    lv_obj_set_style_text_line_space(values, 4, 0);

    static screen_user_data_t ud =
    {
        .enter      = screen_tester_enter,
        .handle_key = screen_tester_handle_key,
        .update     = screen_tester_update,
    };
    lv_obj_set_user_data(screen_tester, &ud);
}
