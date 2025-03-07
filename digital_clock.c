#include <lvgl.h>

#if LV_USE_IMG

LV_IMG_DECLARE(watch_if_5);

void digitalClock(lv_obj_t *parent)
{
    // const void *clock_filename = &watch_if_5;
    // lv_obj_t *clock_if =  lv_img_create(parent);
    // lv_img_set_src(clock_if, clock_filename);
    // lv_obj_set_size(clock_if, 240, 240);
    // lv_obj_center(clock_if);

    // label_datetime = lv_label_create(parent);
    // lv_label_set_text(label_datetime, "00:00");
    // lv_obj_set_style_text_font(label_datetime, &font_firacode_60, LV_PART_MAIN);
    // lv_obj_set_style_text_color(label_datetime, lv_color_white(), LV_PART_MAIN);
    // lv_obj_align(label_datetime, LV_ALIGN_CENTER, 0, 50);

    // lv_timer_create([](lv_timer_t *timer) {
    //     time_t now;
    //     struct tm  timeinfo;
    //     time(&now);
    //     localtime_r(&now, &timeinfo);
    //     static  bool rever = false;
    //     lv_label_set_text_fmt(label_datetime, rever ? "%02d:%02d" : "%02d %02d", timeinfo.tm_hour, timeinfo.tm_min);
    //     rever = !rever;
    // },
    // 1000, NULL);
}

#endif