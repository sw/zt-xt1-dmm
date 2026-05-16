#include <stdio.h>

#include "screen.h"

extern const lv_image_dsc_t img_probe1;
extern const lv_image_dsc_t img_probe2;
extern const lv_image_dsc_t img_probe3;
const lv_image_dsc_t *img_probes[] = { &img_probe1, &img_probe2, &img_probe3 };

void resistor_fmt(lv_obj_t *label, float resistance)
{
    if (resistance < 0.0f)
    {
        return;
    }

    if (resistance < 10.0f)
    {
        lv_label_set_text_fmt(label, "%0.2fΩ", resistance);
    }
    else if (resistance < 1e3f)
    {
        lv_label_set_text_fmt(label, "%0.1fΩ", resistance);
    }
    else if (resistance < 10e3f)
    {
        lv_label_set_text_fmt(label, "%0.3fkΩ", resistance / 1e3f);
    }
    else if (resistance < 100e3f)
    {
        lv_label_set_text_fmt(label, "%0.2fkΩ", resistance / 1e3f);
    }
    else if (resistance < 1e6f)
    {
        lv_label_set_text_fmt(label, "%0.1fkΩ", resistance / 1e3f);
    }
    else if (resistance < 10e6f)
    {
        lv_label_set_text_fmt(label, "%0.2fMΩ", resistance / 1e6f);
    }
    else if (resistance < 100e6f)
    {
        lv_label_set_text_fmt(label, "%0.1fMΩ", resistance / 1e6f);
    }
}

void inductor_fmt(lv_obj_t *label, float inductance_uH, float resistance)
{
    char r_s[16] = { 0 };
    if (resistance < 1e3f)
    {
        sprintf(r_s, "\nR = %0.1fΩ", resistance);
    }

    if (inductance_uH < 100e3f)
    {
        lv_label_set_text_fmt(label, "%0.2fmH%s", inductance_uH / 1e3f, r_s);
    }
    else
    {
        lv_label_set_text_fmt(label, "%0.2fH%s", inductance_uH / 1e6f, r_s);
    }
}
