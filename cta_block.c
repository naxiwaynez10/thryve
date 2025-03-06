#include <lvgl.h>

#if LV_USE_ARC

LV_IMG_DECLARE(recycle_symbol);
LV_IMG_DECLARE(carbon_symbol);
LV_IMG_DECLARE(speaking_symbol);


static void set_angle(void *obj, int32_t v) {
  lv_arc_set_value(obj, v);
}

void cta_block(lv_obj_t *parent, lv_coord_t arc_w, lv_coord_t arc_h, uint32_t arc_color, uint16_t img_zoom) {
  lv_style_t *noPadding;
  lv_style_init(&noPadding);
  lv_style_set_pad_all(&noPadding, 0);
  lv_style_set_bg_opa(&noPadding, LV_OPA_COVER);
  lv_obj_t *arc = lv_arc_create(parent);
  lv_obj_set_size(arc, arc_w, arc_h);
  lv_arc_set_rotation(arc, 270);
  lv_arc_set_bg_angles(arc, 0, 360);
  lv_obj_remove_style(arc, NULL, LV_PART_KNOB);  /*Be sure the knob is not displayed*/
  lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE); /*To not allow adjusting by click*/
  lv_obj_set_style_arc_color(arc, lv_color_hex(arc_color), LV_PART_INDICATOR | LV_STATE_DEFAULT);
  lv_obj_add_style(arc, &noPadding, LV_PART_INDICATOR | LV_PART_MAIN);
  lv_obj_set_style_arc_color(arc, lv_color_hex(0x440), LV_PART_MAIN | LV_STATE_DEFAULT);
  lv_obj_center(arc);


  lv_anim_t a;
  lv_anim_init(&a);
  lv_anim_set_var(&a, arc);
  lv_anim_set_exec_cb(&a, set_angle);
  lv_anim_set_time(&a, 60000);
  lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE); /*Just for the demo*/
  lv_anim_set_repeat_delay(&a, 500);
  lv_anim_set_values(&a, 0, 100);
  lv_anim_start(&a);

  // co2
  lv_obj_t *img1 = lv_img_create(arc);
  // lv_img_set_src(img1, &speaking_symbol);
  lv_img_set_src(img1, &recycle_symbol);
  // lv_img_set_src(img1, &carbon_symbol);
  lv_obj_align(img1, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_size(img1, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_img_set_zoom(img1, img_zoom * 256);


  lv_obj_t *label = lv_label_create(parent);
   lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL);
   lv_obj_set_width(label, lv_pct(100));
  lv_obj_set_style_text_color(label, lv_color_white(), NULL);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_20, NULL);
  lv_obj_align(label, LV_ALIGN_OUT_BOTTOM_MID, 0, 0);
  lv_label_set_text(label, "Stop bush burning in our area");
}

#endif
