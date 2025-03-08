#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <WiFi.h>
// #include "Thrive_Logo.h"
// #if LV_USE_FLEX
// #if LV_USE_IMG

#define DEFAULT_SCREEN_TIMEOUT 30 * 1000
#define HOME_SCREEN_TIMEOUT 10 * 1000
#define DEFAULT_COLOR (lv_color_make(252, 218, 72))



// LV_IMG_DECLARE(clock_face);
// LV_IMG_DECLARE(clock_hour_hand);
// LV_IMG_DECLARE(clock_minute_hand);
// LV_IMG_DECLARE(clock_second_hand);

// LV_IMG_DECLARE(watch_if);
// LV_IMG_DECLARE(watch_bg);
// LV_IMG_DECLARE(watch_if_hour);
// LV_IMG_DECLARE(watch_if_min);
// LV_IMG_DECLARE(watch_if_sec);

// LV_IMG_DECLARE(watch_if_bg2);
// LV_IMG_DECLARE(watch_if_hour2);
// LV_IMG_DECLARE(watch_if_min2);
// LV_IMG_DECLARE(watch_if_sec2);

// LV_FONT_DECLARE(font_siegra);
// LV_FONT_DECLARE(font_sandbox);
// LV_FONT_DECLARE(font_jetBrainsMono);
// LV_FONT_DECLARE(font_firacode_60);
// LV_FONT_DECLARE(font_ununtu_18);


// LV_IMG_DECLARE(img_usb_plug);

// LV_IMG_DECLARE(charge_done_battery);

// LV_IMG_DECLARE(watch_if_5);
// LV_IMG_DECLARE(watch_if_6);
// LV_IMG_DECLARE(watch_if_8);

LV_IMG_DECLARE(recycle_symbol);
LV_IMG_DECLARE(carbon_symbol);
LV_IMG_DECLARE(speaking_symbol);


#define LV_COLOR_WHITE LV_COLOR_MAKE(0xFF, 0xFF, 0xFF)
#define LV_COLOR_SILVER LV_COLOR_MAKE(0xC0, 0xC0, 0xC0)
#define LV_COLOR_GRAY LV_COLOR_MAKE(0x80, 0x80, 0x80)
#define LV_COLOR_BLACK LV_COLOR_MAKE(0x00, 0x00, 0x00)
#define LV_COLOR_RED LV_COLOR_MAKE(0xFF, 0x00, 0x00)
#define LV_COLOR_MAROON LV_COLOR_MAKE(0x80, 0x00, 0x00)
#define LV_COLOR_YELLOW LV_COLOR_MAKE(0xFF, 0xFF, 0x00)
#define LV_COLOR_OLIVE LV_COLOR_MAKE(0x80, 0x80, 0x00)
#define LV_COLOR_LIME LV_COLOR_MAKE(0x00, 0xFF, 0x00)
#define LV_COLOR_GREEN LV_COLOR_MAKE(0x00, 0x80, 0x00)
#define LV_COLOR_CYAN LV_COLOR_MAKE(0x00, 0xFF, 0xFF)
#define LV_COLOR_AQUA LV_COLOR_CYAN
#define LV_COLOR_TEAL LV_COLOR_MAKE(0x00, 0x80, 0x80)
#define LV_COLOR_BLUE LV_COLOR_MAKE(0x00, 0x00, 0xFF)
#define LV_COLOR_NAVY LV_COLOR_MAKE(0x00, 0x00, 0x80)
#define LV_COLOR_MAGENTA LV_COLOR_MAKE(0xFF, 0x00, 0xFF)
#define LV_COLOR_PURPLE LV_COLOR_MAKE(0x80, 0x00, 0x80)
#define LV_COLOR_ORANGE LV_COLOR_MAKE(0xFF, 0xA5, 0x00)



static uint8_t pageId = 0;
static bool canScreenOff = true;
static bool usbPlugIn = false;
static bool isScreenInActive = false;
static uint8_t inactive_time = 0;
static RTC_DATA_ATTR int brightnessLevel = 0;

static int battery_percentage;
lv_obj_t *battery_percent;




lv_obj_t *time_label = NULL;
lv_obj_t *time_sec = NULL;

time_t now;
struct tm timeinfo;
lv_timer_t *clockTimer;

void main_ui(void);
// void battery_bar(lv_obj_t *parent);
void check_battery_cb(lv_timer_t *t);
extern "C" {
  void cta_block(lv_obj_t *parent, lv_coord_t arc_size, uint16_t img_zoom);
};

static lv_obj_t *view = NULL;
static lv_obj_t *home_tile = NULL;
static lv_obj_t *cta_tile = NULL;


void tick_time_cb(lv_timer_t *t) {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  lv_label_set_text_fmt(time_sec, "%02d%", timeinfo.tm_sec);
  lv_label_set_text_fmt(time_label, "%02d%:%02d%",
                        timeinfo.tm_hour,
                        timeinfo.tm_min);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  watch.begin();
  // watch.fillScreen(0x02022B);
  // watch.pushImage(0, 0, THRIVE_LOGO_WIDTH, THRIVE_LOGO_HEIGHT, thrive_logo);
  // delay(3000);
  beginLvglHelper();
  time(&now);
  localtime_r(&now, &timeinfo);
  battery_percentage = watch.getBatteryPercent();
  main_ui();
  usbPlugIn = watch.isVbusIn();
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_task_handler();
  delay(5);

  bool screenTimeout = lv_disp_get_inactive_time(NULL) > DEFAULT_SCREEN_TIMEOUT;
  bool ideal = lv_disp_get_inactive_time(NULL) > HOME_SCREEN_TIMEOUT;

  if (ideal) lv_obj_set_tile(view, cta_tile, LV_ANIM_ON);

  while (screenTimeout && !watch.getTouched()) {
    int b = watch.getBrightness();
    brightnessLevel = b > 0 ? b : brightnessLevel;
    watch.decrementBrightness(0);
  }
  if (watch.getTouched()) lv_disp_trig_activity(NULL);
  watch.incrementalBrightness(brightnessLevel);
}




void main_ui() {

  lv_obj_t *screen = lv_obj_create(lv_scr_act());
  lv_obj_set_size(screen, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(0x02022B), NULL);
  lv_obj_set_style_bg_color(screen, lv_color_hex(0x02022B), NULL);
  lv_obj_set_style_text_color(screen, lv_color_white(), NULL);
  lv_obj_set_style_pad_all(screen, 0, NULL);
  lv_obj_set_style_border_opa(screen, 0, NULL);



  static lv_style_t style_scrolled;
  lv_style_init(&style_scrolled);
  lv_style_set_width(&style_scrolled, 0);
  lv_style_set_bg_opa(&style_scrolled, 0);

  view = lv_tileview_create(screen);
  lv_obj_set_style_bg_opa(view, LV_OPA_TRANSP, NULL);
  lv_obj_add_style(view, &style_scrolled, LV_PART_SCROLLBAR | LV_STATE_SCROLLED);



  /* All Tiles */
  cta_tile = lv_tileview_add_tile(view, 0, 0, LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(cta_tile, LV_OPA_TRANSP, NULL);

  home_tile = lv_tileview_add_tile(view, 1, 0, LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(home_tile, LV_OPA_TRANSP, NULL);




  /* Items in CTA Tiles */
  lv_obj_t *cta_box = lv_obj_create(cta_tile);
  lv_obj_remove_style_all(cta_box);
  lv_obj_set_style_bg_opa(cta_box, LV_OPA_TRANSP, NULL);
  lv_obj_set_size(cta_box, lv_pct(100), lv_pct(100));
  lv_obj_set_style_pad_all(cta_box, 10, NULL);
  lv_obj_set_flex_flow(cta_box, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(cta_box, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  /* Items in HOME Tiles */

  /*Create a container with COLUMN flex direction*/
  lv_obj_t *home_main_col = lv_obj_create(home_tile);
  lv_obj_remove_style_all(home_main_col);
  lv_obj_set_size(home_main_col, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_hor(home_main_col, 10, NULL);
  lv_obj_align_to(home_main_col, home_tile, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_flex_flow(home_main_col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_opa(home_main_col, LV_OPA_TRANSP, NULL);

  lv_obj_t *header = lv_obj_create(home_main_col);
  lv_obj_remove_style_all(header);
  lv_obj_set_size(header, LV_PCT(100), 40);
  lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, NULL);
  // battery_bar(header);

  /* Battery. */

  lv_obj_t *cap = lv_obj_create(header);
  lv_obj_set_size(cap, 4, 8);
  lv_obj_set_style_bg_color(cap, lv_palette_main(LV_PALETTE_BLUE), NULL);
  lv_obj_set_style_border_color(cap, lv_palette_main(LV_PALETTE_BLUE), NULL);
  lv_obj_set_style_radius(cap, 2, NULL);
  lv_obj_align_to(cap, header, LV_ALIGN_RIGHT_MID, 0, 0);


  static lv_style_t style_bg;
  static lv_style_t style_indic;
  lv_style_init(&style_bg);
  lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_border_width(&style_bg, 2);
  lv_style_set_pad_all(&style_bg, 3); /*To make the indicator smaller*/
  lv_style_set_radius(&style_bg, 6);

  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_radius(&style_indic, 4);
  lv_style_set_border_side(&style_indic, LV_BORDER_SIDE_LEFT);

  battery_percent = lv_bar_create(header);
  lv_obj_remove_style_all(battery_percent); /*To have a clean start*/
  lv_obj_add_style(battery_percent, &style_bg, NULL);
  lv_obj_add_style(battery_percent, &style_indic, LV_PART_INDICATOR);
  lv_obj_set_size(battery_percent, 40, 18);
  lv_bar_set_range(battery_percent, 0, 100);
  lv_bar_set_value(battery_percent, battery_percentage, LV_ANIM_OFF);
  lv_obj_align_to(battery_percent, cap, LV_ALIGN_OUT_LEFT_MID, 0, 0);

  lv_timer_create(check_battery_cb, 1000, NULL);









  lv_obj_t *home_row1 = lv_obj_create(home_main_col);
  lv_obj_remove_style_all(home_row1);
  lv_obj_set_size(home_row1, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_align(home_row1, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_flex_flow(home_row1, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(home_row1, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(home_row1, LV_OPA_TRANSP, NULL);
  lv_obj_set_style_pad_hor(home_row1, 10, NULL);
  lv_obj_set_style_border_opa(home_row1, 0, NULL);



  /* The time text */
  static lv_style_t timeStyle;
  lv_style_init(&timeStyle);
  lv_style_set_text_color(&timeStyle, lv_color_hex(0x00B4D8));
  lv_style_set_pad_all(&timeStyle, 0);
  time_label = lv_label_create(home_row1);
  lv_obj_remove_style_all(time_label);
  lv_obj_add_style(time_label, &timeStyle, NULL);
  lv_obj_set_style_text_font(time_label, &lv_font_montserrat_38, NULL);
  lv_label_set_text_fmt(time_label, "%02d%:%02d%",
                        timeinfo.tm_hour,
                        timeinfo.tm_min);

  time_sec = lv_label_create(home_row1);
  lv_obj_add_style(time_sec, &timeStyle, NULL);
  lv_obj_set_style_pad_right(time_sec, 5, NULL);
  lv_obj_set_style_pad_bottom(time_sec, 5, NULL);
  lv_obj_set_style_text_font(time_sec, &lv_font_montserrat_18, NULL);
  lv_label_set_text_fmt(time_label, "%02d%", timeinfo.tm_sec);
  lv_obj_align_to(time_sec, time_label, LV_ALIGN_OUT_RIGHT_BOTTOM, -10, 0);


  static lv_style_t dateStyle;
  lv_style_init(&dateStyle);
  lv_style_set_text_color(&dateStyle, lv_color_hex(0xE27E13));
  lv_style_set_text_font(&dateStyle, &lv_font_montserrat_20);
  lv_obj_t *date_label = lv_label_create(home_row1);
  lv_obj_set_style_pad_bottom(date_label, 5, NULL);
  lv_obj_add_style(date_label, &dateStyle, NULL);
  lv_label_set_text(date_label, "07/17");

  lv_obj_t *home_row2 = lv_obj_create(home_main_col);
  lv_obj_remove_style_all(home_row2);
  lv_obj_set_size(home_row2, LV_PCT(100), LV_PCT(65));
  lv_obj_set_flex_flow(home_row2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(home_row2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  cta_block(home_row2, 120, 2);
  cta_block(cta_box, 180, 3);



  lv_obj_set_tile(view, home_tile, LV_ANIM_OFF);

  lv_timer_create(tick_time_cb, 1000, NULL);
}


void check_battery_cb(lv_timer_t *t) {
  battery_percentage = watch.getBatteryPercent();
  lv_bar_set_value(battery_percent, battery_percentage, LV_ANIM_ON);
}

// void battery_bar(lv_obj_t *parent) {
//   lv_style_t style_bg;
//   lv_style_t style_indic;

//   lv_style_init(&style_bg);
//   lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
//   lv_style_set_border_width(&style_bg, 2);
//   lv_style_set_pad_all(&style_bg, 4); /*To make the indicator smaller*/
//   lv_style_set_radius(&style_bg, 6);
//   // lv_style_set_anim_time(&style_bg, 1000);

//   lv_style_init(&style_indic);
//   lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
//   lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
//   lv_style_set_radius(&style_indic, 3);

//   lv_obj_t *bar = lv_obj_create(parent);
//   lv_obj_remove_style_all(bar); /*To have a clean start*/
//   lv_obj_add_style(bar, &style_bg, NULL);
//   lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

//   lv_obj_set_size(bar, 50, 20);
//   lv_obj_center(bar);
//   // lv_bar_set_value(bar, 80, LV_ANIM_OFF);
// }