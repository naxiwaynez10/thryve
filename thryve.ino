#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <WiFi.h>
// #if LV_USE_FLEX
// #if LV_USE_IMG

#define DEFAULT_SCREEN_TIMEOUT 15 * 1000
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

// LV_IMG_DECLARE(recycle_symbol);
// LV_IMG_DECLARE(carbon_symbol);
// LV_IMG_DECLARE(speaking_symbol);


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

static int battery_percent;

lv_obj_t *hour = NULL;
lv_obj_t *minute = NULL;
lv_obj_t *second = NULL;
lv_obj_t *year = NULL;
lv_obj_t *month = NULL;
lv_obj_t *week = NULL;
lv_obj_t *day = NULL;
lv_obj_t *state = NULL;



static lv_timer_t *clockTimer;
static lv_timer_t *transmitTask;


static uint8_t pageId = 0;
static bool canScreenOff = true;
static bool usbPlugIn = false;
static RTC_DATA_ATTR int brightnessLevel = 0;


typedef struct _lv_datetime {
  lv_obj_t *obj;
  const char *name;
  uint16_t minVal;
  uint16_t maxVal;
  uint16_t defaultVal;
  uint8_t digitFormat;
} lv_datetime_t;

static lv_datetime_t lv_datetime[] = {
  { NULL, "Year", 2023, 2099, 2025, 4 },
  { NULL, "Mon", 1, 12, 4, 2 },
  { NULL, "Day", 1, 30, 12, 2 },
  { NULL, "Hour", 0, 24, 22, 2 },
  { NULL, "Min", 0, 59, 30, 2 },
  { NULL, "Sec", 0, 59, 0, 2 }
};


extern "C" {
  void cta_block(lv_obj_t *parent, lv_coord_t arc_w, lv_coord_t arc_h, uint32_t arc_color, uint16_t img_zoom);
  void battery_bar(lv_obj_t *parent, int battery_percentage);
};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  watch.begin(&Serial);
  beginLvglHelper();

  main_ui();
  usbPlugIn = watch.isVbusIn();
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_task_handler();
  delay(5);
  battery_percent = watch.getBatteryPercent();
  Serial.println(usbPlugIn, battery_percent);
}




void main_ui() {

  // static lv_style_t style;
  //   lv_style_init(&style);
  //   lv_style_set_flex_flow(&style, LV_FLEX_FLOW_ROW_WRAP);
  //   lv_style_set_flex_main_place(&style, LV_FLEX_ALIGN_SPACE_EVENLY);
  //   lv_style_set_layout(&style, LV_LAYOUT_FLEX);




  static lv_style_t bgStyle;
  lv_style_init(&bgStyle);
  lv_style_set_bg_color(&bgStyle, lv_color_hex(0x02022B));
  lv_style_set_text_color(&bgStyle, lv_color_white());




  lv_obj_t *screen = lv_obj_create(lv_scr_act());
  lv_obj_remove_style_all(screen);
  lv_obj_set_size(screen, lv_disp_get_hor_res(NULL), lv_disp_get_ver_res(NULL));
  lv_obj_add_style(screen, &bgStyle, NULL);
  

  lv_obj_t *view = lv_tileview_create(screen);
  lv_obj_remove_style_all(view);
  lv_obj_set_style_bg_opa(view, LV_OPA_TRANSP, NULL);
  lv_obj_set_size(view, LV_PCT(100), LV_PCT(100));


  /* For the CTA when the watch is idile */
  lv_obj_t *cta_tile = lv_tileview_add_tile(view, 0, 0, LV_DIR_RIGHT);
  lv_obj_remove_style_all(cta_tile);
  lv_obj_set_size(cta_tile, LV_PCT(100), LV_SIZE_CONTENT);


  lv_obj_t *cta_view_tile = lv_tileview_create(cta_tile);
  lv_obj_remove_style_all(cta_view_tile);
  lv_obj_set_style_bg_opa(cta_view_tile, LV_OPA_TRANSP, NULL);
  lv_obj_set_size(cta_view_tile, LV_PCT(100), LV_SIZE_CONTENT);

  lv_obj_t *cta_tile1 = lv_tileview_add_tile(cta_view_tile, 0, 0, LV_DIR_VER);
  lv_obj_t *c_list = lv_list_create(cta_tile1);
  lv_obj_set_size(c_list, LV_PCT(100), 240);
  lv_obj_set_style_text_color(c_list, lv_color_white(), NULL);

  lv_list_add_btn(c_list, NULL, "One");
  lv_list_add_btn(c_list, NULL, "Two");
  lv_list_add_btn(c_list, NULL, "Three");
  lv_list_add_btn(c_list, NULL, "Four");
  lv_list_add_btn(c_list, NULL, "Five");



  /* The main watch tile title */
  lv_obj_t *home_tileview = lv_tileview_add_tile(view, 1, 0, LV_DIR_LEFT);
  lv_obj_remove_style_all(home_tileview);
  lv_obj_set_size(home_tileview, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_flex_flow(home_tileview, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(home_tileview, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);



  // lv_obj_t *hometile = lv_tileview_create(home_tileview);
  // lv_obj_remove_style_all(hometile);
  // lv_obj_set_style_bg_opa(hometile, LV_OPA_TRANSP, NULL);
  // lv_obj_set_size(hometile, lv_pct(100), LV_SIZE_CONTENT);


  lv_obj_t *header = lv_obj_create(home_tileview);
  lv_obj_remove_style_all(header);
  lv_obj_set_size(header, LV_PCT(100), 30);
  lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(header, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_END, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, NULL);
  battery_bar(header, battery_percent);




  lv_obj_t *tileview = lv_tileview_create(home_tileview);
  lv_obj_remove_style_all(tileview);
  lv_obj_set_style_bg_opa(tileview, LV_OPA_TRANSP, NULL);
  lv_obj_set_size(tileview, lv_pct(100), LV_SIZE_CONTENT);
  // lv_point_t valid_pos_array[] = {{0,0}, {1,0}, {1,1}, {{LV_COORD_MIN, LV_COORD_MIN}}
  // lv_tileview_set_valid_positions(tileview, valid_pos_array, array_len)
  /*Tile1: just a label*/



  // lv_style_set_bg_opa(&bgStyle, LV_OPA_COVER);
  lv_obj_t *home_tile = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_RIGHT);
  lv_obj_remove_style_all(home_tile);
  lv_obj_set_size(home_tile, LV_PCT(100), 210);



  /*Create a container with COLUMN flex direction*/

  lv_obj_t *home_main_col = lv_obj_create(home_tile);
  lv_obj_remove_style_all(home_main_col);
  lv_obj_set_size(home_main_col, LV_PCT(100), LV_PCT(100));
  lv_obj_align_to(home_main_col, home_tile, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_flex_flow(home_main_col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_opa(home_main_col, LV_OPA_TRANSP, NULL);


  lv_obj_t *home_row1 = lv_obj_create(home_main_col);
  lv_obj_remove_style_all(home_row1);
  lv_obj_set_size(home_row1, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_align(home_row1, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_flex_flow(home_row1, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(home_row1, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(home_row1, LV_OPA_TRANSP, NULL);
  lv_obj_set_style_pad_bottom(home_row1, 10, NULL);


  /* The time text */
  static lv_style_t timeStyle;
  static lv_style_t dateStyle;
  lv_style_init(&timeStyle);
  lv_style_init(&dateStyle);
  lv_style_set_text_color(&timeStyle, lv_color_hex(0x00B4D8));
  lv_style_set_text_font(&timeStyle, &lv_font_montserrat_38);
  lv_style_set_pad_right(&timeStyle, 10);

  lv_style_set_text_color(&dateStyle, lv_color_hex(0xE27E13));
  lv_style_set_text_font(&dateStyle, &lv_font_montserrat_20);

  lv_obj_t *time_label = lv_label_create(home_row1);
  lv_label_set_text(time_label, "10 : 14");
  lv_obj_add_style(time_label, &timeStyle, NULL);

  lv_obj_t *date_label = lv_label_create(home_row1);
  lv_label_set_text(date_label, "07/17");
  lv_obj_add_style(date_label, &dateStyle, NULL);
  lv_obj_align_to(date_label, home_row1, LV_ALIGN_BOTTOM_MID, 0, 0);



  lv_obj_t *home_row2 = lv_obj_create(home_main_col);
  lv_obj_remove_style_all(home_row2);
  lv_obj_set_size(home_row2, LV_PCT(100), LV_PCT(70));
  lv_obj_set_flex_flow(home_row2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(home_row2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


  cta_block(home_row2, 120, 120, 0xE81DA7, 2);



  /*Tile2: a button*/
  lv_obj_t *tile2 = lv_tileview_add_tile(tileview, 1, 0, LV_DIR_HOR);

  lv_obj_t *btn = lv_btn_create(tile2);
  lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
  lv_obj_set_style_bg_color(btn, lv_color_hex(0x00ff03), NULL);
  lv_obj_center(btn);

  lv_obj_t *label = lv_label_create(btn);
  lv_label_set_text(label, "Scroll up or right");



  /*Tile3: a list*/
  lv_obj_t *tile3 = lv_tileview_add_tile(tileview, 1, 1, LV_DIR_LEFT);
  lv_obj_t *list = lv_list_create(tile3);
  lv_obj_set_size(list, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_text_color(list, lv_color_white(), NULL);

  lv_list_add_btn(list, NULL, "One");
  lv_list_add_btn(list, NULL, "Two");
  lv_list_add_btn(list, NULL, "Three");
  lv_list_add_btn(list, NULL, "Four");
  lv_list_add_btn(list, NULL, "Five");
  lv_list_add_btn(list, NULL, "Six");
  lv_list_add_btn(list, NULL, "Seven");
  lv_list_add_btn(list, NULL, "Eight");
  lv_list_add_btn(list, NULL, "Nine");
  lv_list_add_btn(list, NULL, "Ten");
}



void lv_battery_indicator(lv_obj_t *parent) {
  static lv_style_t style_bg;
  static lv_style_t style_indic;

  lv_style_init(&style_bg);
  lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_border_width(&style_bg, 2);
  lv_style_set_pad_all(&style_bg, 6); /*To make the indicator smaller*/
  lv_style_set_radius(&style_bg, 6);
  // lv_style_set_anim_duration(&style_bg, 1000);

  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));
  lv_style_set_radius(&style_indic, 3);

  lv_obj_t *bar = lv_bar_create(parent);
  lv_obj_remove_style_all(bar); /*To have a clean start*/
  lv_obj_add_style(bar, &style_bg, 0);
  lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);

  lv_obj_set_size(bar, 200, 20);
  lv_obj_center(bar);
  lv_bar_set_value(bar, 100, LV_ANIM_ON);
}





void tileview_change_cb(lv_event_t *e) {}
