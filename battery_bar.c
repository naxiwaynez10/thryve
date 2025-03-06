#include <lvgl.h>

#if LV_USE_BAR



void battery_bar(lv_obj_t *parent, int battery_percentage) {
  lv_obj_t *battery = lv_obj_create(parent);
  lv_obj_remove_style_all(battery);
  lv_obj_set_size(battery, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(battery, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(battery, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(battery, LV_OPA_TRANSP, NULL);
  // lv_obj_set_style_pad_right(battery, 10, NULL);
  lv_obj_align_to(battery, parent, LV_ALIGN_RIGHT_MID, 0, 0);

  static lv_style_t style_bg;
  static lv_style_t style_indic;

  lv_style_init(&style_bg);
  lv_style_set_border_color(&style_bg, lv_color_white());
  lv_style_set_border_width(&style_bg, 2);
  lv_style_set_pad_all(&style_bg, 0); /*To make the indicator smaller*/
  lv_style_set_radius(&style_bg, 6);
  lv_style_set_anim_time(&style_bg, 1000);

  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_color_white());
  lv_style_set_radius(&style_indic, 3);

  lv_obj_t *bar = lv_bar_create(battery);
  lv_obj_remove_style_all(bar); /*To have a clean start*/
  lv_obj_add_style(bar, &style_bg, 0);
  lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);
  lv_obj_set_size(bar, 45, 15);
  lv_obj_center(bar);
  lv_bar_set_value(bar, (int32_t)battery_percentage, LV_ANIM_ON);


  static lv_style_t br;
  lv_style_init(&br);
  lv_obj_t *cap = lv_obj_create(battery);
  lv_obj_set_size(cap, 3, 6);
  lv_obj_set_style_bg_color(cap, lv_color_white(), 0);
  lv_obj_set_style_border_width(cap, 0, NULL);
  lv_obj_set_style_radius(cap, 0, NULL);
}

#endif