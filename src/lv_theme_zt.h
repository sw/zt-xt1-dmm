#pragma once

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**
 * Initialize the theme
 * @param disp pointer to display
 * @return a pointer to reference this theme later
 */
lv_theme_t * lv_theme_zt_init(lv_display_t * disp);

/**
* Check if the theme is initialized
* @return true if default theme is initialized, false otherwise
*/
bool lv_theme_zt_is_inited(void);

/**
 * Get zt theme
 * @return a pointer to zt theme, or NULL if this is not initialized
 */
lv_theme_t * lv_theme_zt_get(void);

/**
 * Deinitialize the zt theme
 */
void lv_theme_zt_deinit(void);
