#include <LilyGoLib.h>
#include <LV_Helper.h>
#include <WiFi.h>
#include "Thrive_Logo.h"
// #if LV_USE_FLEX
// #if LV_USE_IMG

#define DEFAULT_SCREEN_TIMEOUT 30 * 1000
#define HOME_SCREEN_TIMEOUT 10 * 1000
#define DEFAULT_COLOR (lv_color_make(252, 218, 72))

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
lv_obj_t *charging_icon;
lv_obj_t *wifi_label;



lv_obj_t *time_label = NULL;
lv_obj_t *time_sec = NULL;
lv_obj_t *date_label = NULL;
const char *weekDays[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };

LV_IMG_DECLARE(recycle_symbol);
LV_IMG_DECLARE(carbon_symbol);




LV_IMG_DECLARE(settings_icon);
LV_IMG_DECLARE(wifi_icon);
LV_IMG_DECLARE(calendar_icon);
LV_IMG_DECLARE(bluetooth_icon);


typedef struct {
  const uint32_t index;
  const lv_img_dsc_t *img_src;
  const uint32_t bg_color;
} Menu;

static Menu menus[] = {
  { 0, &calendar_icon, 0xFF3493 },
  { 1, &settings_icon, 0xF0F002 },
  { 2, &bluetooth_icon, 0xFFD9DA },
  { 3, &wifi_icon, 0xDBF4AD }
};

static const int MENU_SIZE = sizeof(menus) / sizeof(menus[0]);

time_t now;
struct tm timeinfo;
lv_timer_t *clockTimer;

static lv_obj_t *hour;
static lv_obj_t *minute;
static lv_obj_t *box;
const char *hour_opts = "1\n2\n3\n4\n5\n6\n7\n8\n9\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24";

const char *minute_opts =
  "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n"
  "10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n"
  "20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n"
  "30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n"
  "40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n"
  "50\n51\n52\n53\n54\n55\n56\n57\n58\n59";

const char *day_names[7] = { "S", "M", "T", "W", "T", "F", "S" };
static lv_calendar_date_t selected_days[3];


// typedef struct {
//   const uint32_t hour;
//   const uint32_t minute;
// } DateTime;

static tm set_time;



void main_ui(void);
// void battery_bar(lv_obj_t *parent);
void check_battery_cb(lv_timer_t *t);
void menu_page(void);
void menu_clicked_cb(lv_event_t *e);
void save_date_time_cb(lv_event_t *);

extern "C" {
  void cta_block(lv_obj_t *parent, lv_coord_t arc_size, uint16_t img_zoom);
  // void date_time_settings(lv_obj_t *parent);
};

static lv_obj_t *view = NULL;
static lv_obj_t *home_tile = NULL;
static lv_obj_t *menu_tile = NULL;
static lv_obj_t *date_tile = NULL;
static lv_obj_t *wifi_tile = NULL;
static lv_obj_t *settings_tile = NULL;
lv_obj_t *home_main_col = NULL;


void do_tick() {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  // lv_label_set_text_fmt(time_sec, "%02d%", timeinfo.tm_sec);
  lv_label_set_text_fmt(time_label, "%02d%:%02d%",
                        timeinfo.tm_hour,
                        timeinfo.tm_min);

  lv_label_set_text_fmt(date_label, "  %02d/%02d %s",
                        timeinfo.tm_mon,
                        timeinfo.tm_mday,
                        weekDays[timeinfo.tm_wday]);

  battery_percentage = watch.getBatteryPercent();
  usbPlugIn = watch.isVbusIn();
  if (usbPlugIn) {
    lv_obj_clear_flag(charging_icon, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(charging_icon, LV_OBJ_FLAG_HIDDEN);
  }
}

void tick_time_cb(lv_timer_t *t) {
  do_tick();
}


void check_battery_cb(lv_timer_t *t) {
  battery_percentage = watch.getBatteryPercent();
  lv_bar_set_value(battery_percent, battery_percentage, LV_ANIM_ON);
}

static void scroll_event_cb(lv_event_t *e) {
  lv_obj_t *cont = lv_event_get_target(e);

  lv_area_t cont_a;
  lv_obj_get_coords(cont, &cont_a);
  lv_coord_t cont_y_center = cont_a.y1 + lv_area_get_height(&cont_a) / 2;

  lv_coord_t r = lv_obj_get_height(cont) * 7 / 10;
  uint32_t i;
  uint32_t child_cnt = lv_obj_get_child_cnt(cont);
  for (i = 0; i < child_cnt; i++) {
    lv_obj_t *child = lv_obj_get_child(cont, i);
    lv_area_t child_a;
    lv_obj_get_coords(child, &child_a);

    lv_coord_t child_y_center = child_a.y1 + lv_area_get_height(&child_a) / 2;

    lv_coord_t diff_y = child_y_center - cont_y_center;
    diff_y = LV_ABS(diff_y);

    /*Get the x of diff_y on a circle.*/
    lv_coord_t x;
    /*If diff_y is out of the circle use the last point of the circle (the radius)*/
    if (diff_y >= r) {
      x = r;
    } else {
      /*Use Pythagoras theorem to get x from radius and y*/
      uint32_t x_sqr = r * r - diff_y * diff_y;
      lv_sqrt_res_t res;
      lv_sqrt(x_sqr, &res, 0x8000); /*Use lvgl's built in sqrt root function*/
      x = r - res.i;
    }

    /*Translate the item by the calculated X coordinate*/
    lv_obj_set_style_translate_x(child, x, 0);

    /*Use some opacity with larger translations*/
    lv_opa_t opa = lv_map(x, 0, r, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_obj_set_style_opa(child, LV_OPA_COVER - opa, 0);
  }
}

void menu_clicked_cb(lv_event_t *e) {
  uint32_t id = (uint32_t)lv_event_get_user_data(e);
  lv_obj_set_tile_id(view, 2, id, LV_ANIM_ON);
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  watch.begin();
  watch.fillScreen(0x02022B);
  watch.pushImage(0, 0, THRIVE_LOGO_WIDTH, THRIVE_LOGO_HEIGHT, thrive_logo);
  delay(3000);
  beginLvglHelper();
  time(&now);
  localtime_r(&now, &timeinfo);
  main_ui();
  do_tick();
}

void loop() {
  // put your main code here, to run repeatedly:
  lv_task_handler();
  delay(5);

  bool screenTimeout = lv_disp_get_inactive_time(NULL) > DEFAULT_SCREEN_TIMEOUT;

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
  lv_obj_set_scrollbar_mode(view, LV_SCROLLBAR_MODE_OFF);


  /* All Tiles */

  home_tile = lv_tileview_add_tile(view, 0, 0, LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(home_tile, LV_OPA_TRANSP, NULL);

  menu_tile = lv_tileview_add_tile(view, 1, 0, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(menu_tile, LV_OPA_TRANSP, NULL);

  menu_page();



  /* All should be to the right so its not swippable */
  date_tile = lv_tileview_add_tile(view, 2, 0, LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(date_tile, LV_OPA_TRANSP, NULL);
  date_ui();


  settings_tile = lv_tileview_add_tile(view, 2, 1, LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(settings_tile, LV_OPA_TRANSP, NULL);

  wifi_tile = lv_tileview_add_tile(view, 2, 2, LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(wifi_tile, LV_OPA_TRANSP, NULL);


  /* Items in HOME Tiles */

  /*Create a container with COLUMN flex direction*/
  home_main_col = lv_obj_create(home_tile);
  lv_obj_remove_style_all(home_main_col);
  lv_obj_set_size(home_main_col, LV_PCT(100), LV_PCT(100));
  lv_obj_set_style_pad_hor(home_main_col, 5, NULL);
  lv_obj_align_to(home_main_col, home_tile, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_flex_flow(home_main_col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_style_bg_opa(home_main_col, LV_OPA_TRANSP, NULL);

  header_ui();


  lv_timer_create(check_battery_cb, 1000, NULL);



  lv_obj_t *home_row1 = lv_obj_create(home_main_col);
  // lv_obj_remove_style_all(home_row1);
  lv_obj_set_size(home_row1, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_align(home_row1, LV_ALIGN_TOP_MID, 0, 0);
  lv_obj_set_flex_flow(home_row1, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(home_row1, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
  lv_obj_set_style_bg_opa(home_row1, LV_OPA_TRANSP, NULL);
  lv_obj_set_style_pad_top(home_row1, 0, NULL);
  lv_obj_set_style_border_opa(home_row1, 0, NULL);
  lv_obj_set_scrollbar_mode(home_row1, LV_SCROLLBAR_MODE_OFF);


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

  static lv_style_t dateStyle;
  lv_style_init(&dateStyle);
  lv_style_set_text_color(&dateStyle, lv_color_hex(0xE27E13));
  lv_style_set_text_font(&dateStyle, &lv_font_montserrat_18);

  date_label = lv_label_create(home_row1);
  lv_obj_remove_style_all(date_label);
  lv_obj_align(date_label, LV_ALIGN_RIGHT_MID, 0, 0);
  lv_obj_add_style(date_label, &dateStyle, NULL);
  lv_label_set_text(date_label, "07/17 SAT");





  lv_obj_t *home_row2 = lv_obj_create(home_main_col);
  lv_obj_remove_style_all(home_row2);
  lv_obj_set_size(home_row2, LV_PCT(100), LV_PCT(65));
  lv_obj_set_flex_flow(home_row2, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(home_row2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  // cta_block(cta_box, 180, 3);
  cta_block(home_row2, 120, 2);



  // lv_obj_set_tile(view, home_tile, LV_ANIM_OFF);
  // lv_obj_set_tile_id(view, 2, 0, LV_ANIM_OFF);

  lv_timer_create(tick_time_cb, 1000, NULL);
}



void menu_page() {
  lv_obj_t *cont = lv_obj_create(menu_tile);
  lv_obj_set_size(cont, lv_pct(100), lv_pct(100));
  lv_obj_center(cont);
  lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, NULL);
  lv_obj_set_style_pad_left(cont, 50, NULL);
  lv_obj_set_style_border_opa(cont, 0, NULL);
  lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
  lv_obj_add_event_cb(cont, scroll_event_cb, LV_EVENT_SCROLL, NULL);
  // lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
  lv_obj_set_style_clip_corner(cont, false, 0);
  lv_obj_set_scroll_dir(cont, LV_DIR_VER);
  lv_obj_set_scroll_snap_y(cont, LV_SCROLL_SNAP_CENTER);
  lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
  lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLL_ONE);

  uint32_t i;
  for (i = 0; i < MENU_SIZE; i++) {
    lv_obj_t *btn = lv_btn_create(cont);
    // lv_obj_remove_style_all(btn);
    lv_obj_set_style_pad_all(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(menus[i].bg_color), 0);
    lv_obj_set_size(btn, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_radius(btn, lv_pct(100), NULL);

    lv_obj_t *img = lv_img_create(btn);
    lv_img_set_src(img, menus[i].img_src);
    lv_img_set_zoom(img, (0.6 * 256));
    lv_obj_center(img);
    lv_obj_add_event_cb(btn, menu_clicked_cb, LV_EVENT_CLICKED, (void *)menus[i].index);

    // lv_obj_t *label = lv_label_create(btn);
    // lv_obj_align(label, LV_ALIGN_OUT_RIGHT_MID, 50, 25);
    // lv_label_set_text_fmt(label, "Button %" LV_PRIu32, i);
    // lv_obj_move_foreground(label);
  }

  /*Update the buttons position manually for first*/
  lv_event_send(cont, LV_EVENT_SCROLL, NULL);

  /*Be sure the fist button is in the middle*/
  lv_obj_scroll_to_view(lv_obj_get_child(cont, 0), LV_ANIM_ON);
}


void header_ui() {
  lv_obj_t *header = lv_obj_create(home_main_col);
  lv_obj_remove_style_all(header);
  lv_obj_set_size(header, LV_PCT(100), 25);
  lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, NULL);
  // battery_bar(header);


  /* Wifi */
  wifi_label = lv_label_create(header);
  lv_obj_align_to(wifi_label, header, LV_ALIGN_LEFT_MID, 10, 0);
  lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
  lv_obj_set_style_text_color(wifi_label, lv_color_white(), NULL);


  /* Battery. */
  lv_obj_t *cap = lv_obj_create(header);
  lv_obj_set_size(cap, 4, 8);
  lv_obj_set_style_bg_color(cap, lv_color_white(), NULL);
  lv_obj_set_style_border_color(cap, lv_color_white(), NULL);
  lv_obj_set_style_radius(cap, 2, NULL);
  lv_obj_align_to(cap, header, LV_ALIGN_RIGHT_MID, -10, 0);


  static lv_style_t style_bg;
  static lv_style_t style_indic;
  lv_style_init(&style_bg);
  lv_style_set_border_color(&style_bg, lv_color_white());
  lv_style_set_border_width(&style_bg, 2);
  lv_style_set_pad_all(&style_bg, 3); /*To make the indicator smaller*/
  lv_style_set_radius(&style_bg, 6);

  lv_style_init(&style_indic);
  lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);
  lv_style_set_bg_color(&style_indic, lv_color_white());
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

  /* Charging */
  charging_icon = lv_label_create(battery_percent);
  lv_obj_center(charging_icon);
  lv_obj_set_size(charging_icon, LV_SIZE_CONTENT, 12);
  lv_label_set_text(charging_icon, LV_SYMBOL_CHARGE);
  lv_obj_set_style_text_color(charging_icon, lv_color_black(), NULL);
}

void date_ui() {
  lv_style_t style;
  lv_style_init(&style);
  lv_style_set_pad_column(&style, 30);
  lv_obj_t *col = lv_obj_create(date_tile);
  lv_obj_remove_style_all(col);
  lv_obj_set_size(col, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_all(col, 10, NULL);
  lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, NULL);
  lv_obj_set_flex_flow(col, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(col, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
  lv_obj_add_style(col, &style, NULL);
  lv_obj_set_scrollbar_mode(col, LV_SCROLLBAR_MODE_OFF);
  back_btn(col);
  date_time_settings(col);
}


static void time_settings_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    if (obj == hour) {
      set_time.tm_hour = (int)buf;
    }
    if (obj == minute) {
      set_time.tm_min = (int)buf;
    }
    LV_LOG_USER("Selected value: %s", buf);
  }
}


void date_time_settings(lv_obj_t *parent) {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);
  set_time.tm_hour = timeinfo.tm_hour;
  set_time.tm_min = timeinfo.tm_min;
  set_time.tm_sec = timeinfo.tm_sec;

  static lv_style_t style_sel;
  lv_style_init(&style_sel);
  lv_style_set_text_font(&style_sel, &lv_font_montserrat_22);
  lv_style_set_text_color(&style_sel, lv_color_white());
  lv_style_set_pad_ver(&style_sel, 30);

  lv_obj_t *time_heading_label = lv_label_create(parent);
  lv_label_set_text(time_heading_label, "Set the time");
  lv_obj_add_style(time_heading_label, &style_sel, LV_PART_MAIN);

  static lv_style_t s;
  lv_style_init(&s);
  lv_style_set_text_color(&s, lv_color_white());
  lv_style_set_pad_ver(&s, 5);
  lv_style_set_pad_hor(&s, 20);

  lv_obj_t *box2 = lv_obj_create(parent);
  lv_obj_remove_style_all(box2);
  lv_obj_set_style_bg_opa(box2, LV_OPA_TRANSP, NULL);
  lv_obj_set_size(box2, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_center(box2);
  lv_obj_set_flex_flow(box2, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(box2, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *hour_label = lv_label_create(box2);
  lv_label_set_text(hour_label, "HOUR");
  lv_obj_add_style(hour_label, &s, 0);


  lv_obj_t *min_label = lv_label_create(box2);
  lv_label_set_text(min_label, "MINUTE");
  lv_obj_add_style(min_label, &s, 0);




  box = lv_obj_create(parent);
  lv_obj_set_style_border_opa(box, 0, NULL);
  lv_obj_set_size(box, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(box, LV_OPA_TRANSP, NULL);
  lv_obj_center(box);
  lv_obj_set_flex_flow(box, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(box, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


  hour = lv_roller_create(box);
  lv_roller_set_options(hour, hour_opts, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(hour, 3);
  lv_obj_add_style(hour, &style_sel, LV_PART_SELECTED);
  lv_obj_add_event_cb(hour, time_settings_cb, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(hour, timeinfo.tm_hour, LV_ANIM_OFF);

  lv_obj_t *separator_label = lv_label_create(box);
  lv_label_set_text(separator_label, ":");

  minute = lv_roller_create(box);
  lv_roller_set_options(minute, minute_opts, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(minute, 3);
  lv_obj_add_style(minute, &style_sel, LV_PART_SELECTED);
  lv_obj_add_event_cb(minute, time_settings_cb, LV_EVENT_ALL, NULL);
  lv_roller_set_selected(minute, timeinfo.tm_min, LV_ANIM_OFF);



  lv_obj_t *date_heading_label = lv_label_create(parent);
  lv_label_set_text(date_heading_label, "Set the Date");
  lv_obj_add_style(date_heading_label, &style_sel, LV_PART_MAIN);
  lv_obj_set_style_pad_top(date_heading_label, 50, LV_PART_MAIN);

  calender_ui(parent);

  lv_obj_t *x = lv_label_create(parent);
  lv_obj_remove_style_all(x);
  lv_obj_set_style_bg_opa(x, LV_OPA_TRANSP, NULL);
  lv_obj_set_size(x, lv_pct(100), 40);  /// Just to give space

  lv_obj_t *btn = lv_btn_create(parent);
  lv_obj_set_size(btn, lv_pct(100), 50);
  lv_obj_add_event_cb(btn, save_date_time_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *text = lv_label_create(btn);
  lv_label_set_text(text, "Save settings");
  lv_obj_center(text);
}




static void event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_current_target(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    lv_calendar_date_t date;
    if (lv_calendar_get_pressed_date(obj, &date)) {
      selected_days[0].year = date.year;
      selected_days[0].month = date.month;
      selected_days[0].day = date.day;
      lv_calendar_set_highlighted_dates(obj, selected_days, 1);
      LV_LOG_USER("Clicked date: %02d.%02d.%d", date.day, date.month, date.year);
    }
  }
}



void calender_ui(lv_obj_t *parent) {
  time_t now;
  struct tm timeinfo;
  time(&now);
  localtime_r(&now, &timeinfo);

  lv_obj_t *calendar = lv_calendar_create(parent);
  lv_obj_set_size(calendar, lv_pct(100), 200);
  lv_obj_add_event_cb(calendar, event_handler, LV_EVENT_ALL, NULL);
  lv_calendar_set_day_names(calendar, day_names);

  lv_calendar_set_today_date(calendar, (timeinfo.tm_year + 1900), timeinfo.tm_mday, timeinfo.tm_wday);
  lv_calendar_set_showed_date(calendar, (timeinfo.tm_year + 1900), timeinfo.tm_mday);

  selected_days[0].year = (timeinfo.tm_year + 1900);
  selected_days[0].month = timeinfo.tm_mday;
  selected_days[0].day = timeinfo.tm_wday;
  lv_calendar_set_highlighted_dates(calendar, selected_days, 1);

#if LV_USE_CALENDAR_HEADER_DROPDOWN
  lv_calendar_header_dropdown_create(calendar);
#elif LV_USE_CALENDAR_HEADER_ARROW
  lv_calendar_header_arrow_create(calendar);
#endif
}


void save_date_time_cb(lv_event_t *) {

  watch.setDateTime(selected_days[0].year, selected_days[0].month, selected_days[0].day, set_time.tm_hour, set_time.tm_min, set_time.tm_sec);
  // Reading time synchronization from RTC to system time
  watch.hwClockRead();
}



void go_back_cb(lv_event_t *e) {
  lv_obj_t *current_tile = lv_tileview_get_tile_act(view);
  if (current_tile != menu_tile) lv_obj_set_tile(view, menu_tile, LV_ANIM_ON);
}


void back_btn(lv_obj_t *parent) {
  lv_obj_t *con = lv_obj_create(parent);
  lv_obj_remove_style_all(con);
  lv_obj_set_size(con, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_set_style_border_opa(con, 0, NULL);
  lv_obj_set_style_bg_opa(con, LV_OPA_TRANSP, 0);

  lv_obj_t *btn = lv_btn_create(con);
  lv_obj_align(btn, LV_ALIGN_LEFT_MID, 0, 0);
  lv_obj_set_size(btn, 30, 30);
  lv_obj_add_event_cb(btn, go_back_cb, LV_EVENT_CLICKED, NULL);

  lv_obj_t *img = lv_label_create(btn);
  lv_label_set_text(img, LV_SYMBOL_LEFT);
  lv_obj_center(img);
}