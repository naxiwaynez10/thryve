#include <Arduino_JSON.h>
#include <assert.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LilyGoLib.h>
#include <LV_Helper.h>
#include "Thrive_Logo.h"
#include <Preferences.h>
#include <time.h>
#include <sntp.h>
Preferences prefs;

// #if LV_USE_FLEX
// #if LV_USE_IMG

#define MAX_NETWORKS 10  // Max WiFi networks to display
#define DEFAULT_SCREEN_TIMEOUT 30 * 1000
#define HOME_SCREEN_TIMEOUT 10 * 1000
#define DEFAULT_COLOR (lv_color_make(252, 218, 72))

#define LV_COLOR_WHITE LV_COLOR_MAKE(0xFF, 0xFF, 0xFF)
// #define LV_COLOR_SILVER LV_COLOR_MAKE(0xC0, 0xC0, 0xC0)
#define LV_COLOR_GRAY LV_COLOR_MAKE(0x80, 0x80, 0x80)
// #define LV_COLOR_BLACK LV_COLOR_MAKE(0x00, 0x00, 0x00)
// #define LV_COLOR_RED LV_COLOR_MAKE(0xFF, 0x00, 0x00)
// #define LV_COLOR_MAROON LV_COLOR_MAKE(0x80, 0x00, 0x00)
// #define LV_COLOR_YELLOW LV_COLOR_MAKE(0xFF, 0xFF, 0x00)
// #define LV_COLOR_OLIVE LV_COLOR_MAKE(0x80, 0x80, 0x00)
// #define LV_COLOR_LIME LV_COLOR_MAKE(0x00, 0xFF, 0x00)
// #define LV_COLOR_GREEN LV_COLOR_MAKE(0x00, 0x80, 0x00)
// #define LV_COLOR_CYAN LV_COLOR_MAKE(0x00, 0xFF, 0xFF)
// #define LV_COLOR_AQUA LV_COLOR_CYAN
// #define LV_COLOR_TEAL LV_COLOR_MAKE(0x00, 0x80, 0x80)
// #define LV_COLOR_BLUE LV_COLOR_MAKE(0x00, 0x00, 0xFF)
// #define LV_COLOR_NAVY LV_COLOR_MAKE(0x00, 0x00, 0x80)
// #define LV_COLOR_MAGENTA LV_COLOR_MAKE(0xFF, 0x00, 0xFF)
// #define LV_COLOR_PURPLE LV_COLOR_MAKE(0x80, 0x00, 0x80)
// #define LV_COLOR_ORANGE LV_COLOR_MAKE(0xFF, 0xA5, 0x00)



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






LV_IMG_DECLARE(settings_icon);
LV_IMG_DECLARE(wifi_icon);
LV_IMG_DECLARE(calendar_icon);
LV_IMG_DECLARE(bluetooth_icon);
LV_IMG_DECLARE(crypto_icon);
LV_IMG_DECLARE(running_icon);


typedef struct {
  const uint32_t index;
  const lv_img_dsc_t *img_src;
  const uint32_t bg_color;
} Menu;

static Menu menus[] = {
  { 0, &calendar_icon, 0xFF3493 },
  { 7, &settings_icon, 0xF0F002 },
  { 3, &wifi_icon, 0x19E1E1 },
  { 4, &crypto_icon, 0xF2E9DF },
  { 5, &running_icon, 0x4FAE6F },
  { 8, &bluetooth_icon, 0xFFD9DA },
};



static const int MENU_SIZE = sizeof(menus) / sizeof(menus[0]);

time_t now;
struct tm timeinfo;
lv_timer_t *clockTimer;
const char *ntpServer1 = "pool.ntp.org";
const char *ntpServer2 = "time.nist.gov";

const char *time_zone = "WAT-1";  // TimeZone rule for Europe/Rome including daylight adjustment rules (optional)


static lv_obj_t *hour;
static lv_obj_t *minute;
static lv_obj_t *box;
const char *hour_opts = "1\n2\n3\n4\n5\n6\n7\n8\n9\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24";
static const char *years_list = "2028\n2027\n2026\n2025\n2024\n2023\n2022\n2021\n2020\n2019\n2018";
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
void wifi_input_ui(lv_obj_t *parent);
void fetch_crypto_lists();
void parse_crypo_list(String jsonData);

// Forward declaration of the async task
void fetchCryptoPricesTask(void *parameter);





extern "C" {
  void cta_block(lv_obj_t *parent, lv_coord_t arc_size, uint16_t img_zoom);
};

static lv_obj_t *view = NULL;
static lv_obj_t *home_tile = NULL;
static lv_obj_t *menu_tile = NULL;
static lv_obj_t *date_tile = NULL;
static lv_obj_t *wifi_tile = NULL;
static lv_obj_t *wifi_input_tile = NULL;
static lv_obj_t *bluetooth_tile = NULL;
static lv_obj_t *settings_tile = NULL;
static lv_obj_t *crypto_list_tile = NULL;
static lv_obj_t *crypto_detail_tile;

static lv_obj_t *unused_1;
static lv_obj_t *unused_2;
static lv_obj_t *unused_3;
static lv_obj_t *unused_4;

lv_obj_t *home_main_col = NULL;
lv_obj_t *wifi_table;
lv_obj_t *wifi_dropdown;
lv_obj_t *wifi_scan_btn;

const char *ssid = NULL;    // Change this to your WiFi SSID
const char *password = "";  // Change this to your WiFi password
lv_obj_t *wifi_status_label;




const char *apiURL = "https://api.coingecko.com/api/v3/simple/price?ids=bitcoin%2Cethereum%2Cripple%2Cdogecoin&vs_currencies=usd&include_market_cap=true&include_24hr_vol=true&include_24hr_change=true";
const char *apiKey = "CG-aohJQfFQ12WaFMJErKivKWKm";  // Demo Key
lv_obj_t *crypto_list;
lv_obj_t *crypto_table;
char *currentId = NULL;
JSONVar coinDetails;
JSONVar coins;


static lv_obj_t *chart;
static lv_chart_series_t *serX;
static lv_chart_series_t *serY;
static lv_chart_cursor_t *cursor;



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
                        timeinfo.tm_mon + 1,
                        timeinfo.tm_mday,
                        weekDays[timeinfo.tm_wday]);
  if (timeinfo.tm_hour == 12 && timeinfo.tm_min == 00 && timeinfo.tm_sec < 6) {
    //Buzz for 5 times
    // set the effect to play
    watch.setWaveform(0, 15);  // play effect
    // play the effect!
    watch.run();
  }
  battery_percentage = watch.getBatteryPercent();
  usbPlugIn = watch.isVbusIn();
  if (usbPlugIn) {
    lv_obj_clear_flag(charging_icon, LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(charging_icon, LV_OBJ_FLAG_HIDDEN);
  }
  if (WiFi.status() == WL_CONNECTED) {
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    // fetch_crypto_lists();
  } else {
    lv_label_set_text(wifi_label, "");
    // connect_wifi();
  }
}

void tick_time_cb(lv_timer_t *t) {
  do_tick();
}

void make_request_cb(lv_timer_t *t) {
}

void timeavailable(struct timeval *t) {
  Serial.println("Got time adjustment from NTP, Write the hardware clock");

  // Write synchronization time to hardware
  watch.hwClockWrite();
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
  lv_obj_set_tile_id(view, 2, id, LV_ANIM_OFF);
}



void on_tile_changed(lv_event_t *e) {
  lv_obj_t *act_tile = lv_tileview_get_tile_act(view);
}

void all_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *act_tile = lv_tileview_get_tile_act(view);
  switch (code) {
    case LV_EVENT_SCROLL_BEGIN:
      // Serial.println("LV_EVENT_SCROLL_BEGIN");
      break;
    case LV_EVENT_SCROLL:
      // Serial.println("LV_EVENT_SCROLL");
      break;
    case LV_EVENT_SCROLL_END:
      if (act_tile == unused_1 || act_tile == unused_2 || act_tile == unused_3 || act_tile == unused_4) {
        lv_obj_set_tile_id(view, 1, 0, LV_ANIM_OFF);
      }
      break;
    case LV_EVENT_VALUE_CHANGED:
      if (act_tile != crypto_detail_tile) {
        currentId = NULL;
      }

      break;
    default:
      // Serial.println("Other event triggered");
      break;
  }
}


void connect_wifi() {
  String s = prefs.getString("ssid", "");
  String p = prefs.getString("password", "");
  ssid = s.c_str();
  password = p.c_str();
  Serial.printf("Connecting to: \n ssid: %s pass: %s", ssid, password);
  WiFi.begin(ssid, password);
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  prefs.begin("wifi", false);
  watch.begin();
  watch.fillScreen(0x02022B);
  watch.pushImage(0, 0, THRIVE_LOGO_WIDTH, THRIVE_LOGO_HEIGHT, thrive_logo);
  delay(3000);
  beginLvglHelper();
  sntp_set_time_sync_notification_cb(timeavailable);
  configTzTime(time_zone, ntpServer1, ntpServer2);
  time(&now);
  localtime_r(&now, &timeinfo);
  main_ui();
  do_tick();
  connect_wifi();
  while (WiFi.status() != WL_CONNECTED) {
    delay(5);
    lv_task_handler();
    Serial.print(".");
  }

  // **Run API Request in a separate thread**
  xTaskCreatePinnedToCore(
    fetchCryptoPricesTask,  // Task function
    "FetchCryptoPrices",    // Task name
    8192,                   // Stack size (8KB)
    NULL,                   // Task parameter
    1,                      // Priority
    NULL,                   // Task handle
    1                       // Run on Core 1 (UI runs on Core 0)
  );
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
    // sleep();
  }
  if (watch.getTouched()) lv_disp_trig_activity(NULL);
  watch.incrementalBrightness(brightnessLevel);
}


void fetchCryptoPricesTask(void *parameter) {
  while (true) {  // Keep running this task
    fetch_crypto_lists();
    fetch_crypto_detail();
    vTaskDelay(1000 / portTICK_PERIOD_MS);  // Delay 1 minute before next request
  }
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
  // lv_obj_add_event_cb(view, on_tile_changed, LV_EVENT_VALUE_CHANGED, NULL);
  // lv_obj_add_event_cb(view, gesture_cb, LV_EVENT_SCROLL_END, NULL);
  lv_obj_add_event_cb(view, all_cb, LV_EVENT_ALL, NULL);




  /* All Tiles */

  home_tile = lv_tileview_add_tile(view, 0, 0, LV_DIR_RIGHT);
  lv_obj_set_style_bg_opa(home_tile, LV_OPA_TRANSP, NULL);

  menu_tile = lv_tileview_add_tile(view, 1, 0, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(menu_tile, LV_OPA_TRANSP, NULL);
  menu_page();

  /* Unused Tiles to help return swipe to the menu_tile */
  unused_1 = lv_tileview_add_tile(view, 1, 1, LV_DIR_TOP);
  unused_2 = lv_tileview_add_tile(view, 1, 2, LV_DIR_TOP);
  unused_3 = lv_tileview_add_tile(view, 1, 3, LV_DIR_TOP);
  unused_4 = lv_tileview_add_tile(view, 1, 4, LV_DIR_TOP);

  /*----------------------------------------------------------------*/



  /* All should be to the right so its not swippable */
  date_tile = lv_tileview_add_tile(view, 2, 0, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(date_tile, LV_OPA_TRANSP, NULL);
  date_ui();


  settings_tile = lv_tileview_add_tile(view, 2, 1, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(settings_tile, LV_OPA_TRANSP, NULL);

  bluetooth_tile = lv_tileview_add_tile(view, 2, 2, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(bluetooth_tile, LV_OPA_TRANSP, NULL);

  wifi_tile = lv_tileview_add_tile(view, 2, 3, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(wifi_tile, LV_OPA_TRANSP, NULL);
  wifi_input_ui(wifi_tile);

  crypto_list_tile = lv_tileview_add_tile(view, 2, 4, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(crypto_list_tile, LV_OPA_TRANSP, NULL);
  crypto_list_ui();

  crypto_detail_tile = lv_tileview_add_tile(view, 3, 4, LV_DIR_LEFT);
  lv_obj_set_style_bg_opa(crypto_detail_tile, LV_OPA_TRANSP, NULL);
  lv_obj_set_style_pad_hor(crypto_detail_tile, 0, NULL);
  crypto_detail_ui();




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
  cta_block(home_row2, 120, 1.9);



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
  if (WiFi.status() == WL_CONNECTED) {
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
  } else {
    lv_label_set_text(wifi_label, "");
  }
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
    char buf[4];
    lv_roller_get_selected_str(obj, buf, sizeof(buf));
    if (obj == hour) {
      set_time.tm_hour = atoi(buf);
    }
    if (obj == minute) {
      set_time.tm_min = atoi(buf);
    }
    LV_LOG_USER("Selected value: %d", atoi(buf));
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
  int h = timeinfo.tm_hour;
  lv_roller_set_selected(hour, h, LV_ANIM_OFF);

  lv_obj_t *separator_label = lv_label_create(box);
  lv_label_set_text(separator_label, ":");

  minute = lv_roller_create(box);
  lv_roller_set_options(minute, minute_opts, LV_ROLLER_MODE_NORMAL);
  lv_roller_set_visible_row_count(minute, 3);
  lv_obj_add_style(minute, &style_sel, LV_PART_SELECTED);
  lv_obj_add_event_cb(minute, time_settings_cb, LV_EVENT_ALL, NULL);
  int m = timeinfo.tm_min;
  lv_roller_set_selected(minute, m, LV_ANIM_OFF);



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
  lv_calendar_header_dropdown_set_year_list(calendar, years_list);
  lv_calendar_set_today_date(calendar, (timeinfo.tm_year + 1900), timeinfo.tm_mon + 1, timeinfo.tm_mday);
  lv_calendar_set_showed_date(calendar, (timeinfo.tm_year + 1900), timeinfo.tm_mday);

  selected_days[0].year = (timeinfo.tm_year + 1900);
  selected_days[0].month = timeinfo.tm_mon;
  selected_days[0].day = timeinfo.tm_mday;
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
  if (current_tile != menu_tile) lv_obj_set_tile(view, menu_tile, LV_ANIM_OFF);
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


static void ta_event_cb(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *ta = lv_event_get_target(e);
  lv_obj_t *kb = (lv_obj_t *)lv_event_get_user_data(e);
  if (code == LV_EVENT_FOCUSED) {
    lv_keyboard_set_textarea(kb, ta);
    lv_obj_clear_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(wifi_scan_btn, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_DEFOCUSED) {
    lv_keyboard_set_textarea(kb, NULL);
    lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clear_flag(wifi_scan_btn, LV_OBJ_FLAG_HIDDEN);
  }

  if (code == LV_EVENT_READY) {
    password = lv_textarea_get_text(ta);
    WiFi.begin(ssid, password);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      i++;
      delay(100);
      Serial.println("");
      Serial.printf("password: %s \n SSID: %s \n", password, ssid);
      if (i == 10) break;
    }
    if (WiFi.status() == WL_CONNECTED) {
      prefs.putString("ssid", String(ssid));
      prefs.putString("password", String(password));
      lv_label_set_text_fmt(wifi_status_label, "Connected to %s", ssid);
    }
  }
}



void scan_and_update_dropdown_cb(lv_event_t *e) {

  lv_dropdown_set_text(wifi_dropdown, "Scanning...");
  lv_dropdown_set_options(wifi_dropdown, "Select");
  int networkCount = WiFi.scanNetworks(); /* Scan available networks */
  if (networkCount > 0) {
    lv_dropdown_set_text(wifi_dropdown, NULL);
    for (int i = 0; i < min(networkCount, MAX_NETWORKS); i++) {
      // Add a new row for each network
      lv_dropdown_add_option(wifi_dropdown, WiFi.SSID(i).c_str(), i + 1);
    }
  }
  WiFi.scanDelete();  // Free memory after scan
}


static void wifi_dropdown_event_handler(lv_event_t *e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);
  int wifi = (int)lv_event_get_user_data(e);
  if (code == LV_EVENT_VALUE_CHANGED) {
    char buf[32];
    lv_dropdown_get_selected_str(obj, buf, sizeof(buf));
    ssid = (char *)buf;
    WiFi.begin(ssid, password);
    int i = 0;
    while (WiFi.status() != WL_CONNECTED) {
      i++;
      delay(100);
      Serial.print(".");
      if (i == 10) break;
    }
    if (WiFi.status() == WL_CONNECTED) {
      prefs.putString("ssid", String(ssid));
      prefs.putString("password", String(password));
      lv_label_set_text_fmt(wifi_status_label, "Connected to %s", ssid);
    }

    LV_LOG_USER("Option: %s", buf);
  }
}


void wifi_input_ui(lv_obj_t *parent) {

  lv_style_t style;
  lv_style_init(&style);
  lv_style_set_pad_column(&style, 10);
  lv_obj_t *col = lv_obj_create(parent);
  lv_obj_remove_style_all(col);
  lv_obj_set_size(col, LV_PCT(100), lv_pct(100));
  lv_obj_set_style_pad_hor(col, 20, NULL);
  lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, NULL);
  lv_obj_add_style(col, &style, NULL);


  wifi_dropdown = lv_dropdown_create(col);
  lv_obj_align(wifi_dropdown, LV_ALIGN_TOP_MID, 0, 20);
  lv_obj_set_size(wifi_dropdown, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_add_event_cb(wifi_dropdown, wifi_dropdown_event_handler, LV_EVENT_ALL, NULL);
  lv_dropdown_set_text(wifi_dropdown, "Scan for Wifi");


  /*Create a text area. The keyboard will write here*/
  lv_obj_t *ta;
  ta = lv_textarea_create(col);
  lv_obj_align_to(ta, wifi_dropdown, LV_ALIGN_OUT_BOTTOM_MID, 30, 20);
  /*Create a keyboard to use it with an of the text areas*/
  lv_obj_t *kb = lv_keyboard_create(wifi_tile);
  lv_obj_add_flag(kb, LV_OBJ_FLAG_HIDDEN);
  // lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
  lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
  lv_textarea_set_placeholder_text(ta, "Password");
  lv_obj_set_size(ta, lv_pct(100), 40);
  lv_textarea_set_one_line(ta, true);
  lv_textarea_set_password_mode(ta, true);
  lv_keyboard_set_textarea(kb, ta);
  lv_obj_align_to(kb, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

  lv_obj_t *div = lv_obj_create(col);
  lv_obj_remove_style_all(div);
  lv_obj_set_size(div, lv_pct(100), 20);
  lv_obj_align_to(div, ta, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
  wifi_status_label = lv_label_create(div);
  lv_obj_center(wifi_status_label);
  lv_label_set_long_mode(wifi_status_label, LV_LABEL_LONG_SCROLL);
  // lv_obj_set_style_text_font()
  if (WiFi.status() == WL_CONNECTED) {
    lv_label_set_text_fmt(wifi_status_label, "Connected to %s", ssid);
  } else {
    lv_label_set_text(wifi_status_label, "Not Connected");
  }

  lv_obj_set_style_text_color(wifi_status_label, LV_COLOR_WHITE, NULL);

  wifi_scan_btn = lv_btn_create(col);
  lv_obj_align(wifi_scan_btn, LV_ALIGN_BOTTOM_MID, 0, -20);
  lv_obj_t *label = lv_img_create(wifi_scan_btn);
  lv_img_set_src(label, LV_SYMBOL_WIFI "  Scan");
  lv_obj_center(label);


  // Add event callback to refresh WiFi list
  lv_obj_add_event_cb(wifi_scan_btn, scan_and_update_dropdown_cb, LV_EVENT_CLICKED, NULL);
}



void cell_select_cb(lv_event_t *e) {
  lv_obj_t *obj = lv_event_get_target(e);
  uint16_t col;
  uint16_t row;
  lv_table_get_selected_cell(obj, &row, &col);
  if (row != 0) {
    JSONVar keys = coins.keys();
    const char *coinName = (const char *)keys[row - 1];
    currentId = (char *)lv_table_get_cell_value(obj, row, 0);
    Serial.printf("Clicked %d %s", row, currentId);
    Serial.println();
    lv_obj_set_tile_id(view, 3, 4, LV_ANIM_OFF);
  }
}

void parse_crypo_list(String jsonData) {

  coins = JSON.parse(jsonData);
  if (JSON.typeof(coins) == "undefined") {
    Serial.println("Parsing failed!");
    return;
  }
  // myObject.keys() can be used to get an array of all the keys in the object
  JSONVar keys = coins.keys();

  for (int i = 0; i < keys.length(); i++) {
    const char *coinName = (const char *)keys[i];  // Get the coin name
    JSONVar coinData = coins[keys[i]];

    float price = (double)coinData["usd"];
    float marketCap = (double)coinData["usd_market_cap"] / 1e9;
    float volume = (double)coinData["usd_24h_vol"];
    float change = (double)coinData["usd_24h_change"];

    // Convert values to char* using snprintf
    char priceStr[20], marketCapStr[20], volumeStr[20], changeStr[20];
    snprintf(priceStr, sizeof(priceStr), "$%.2f", price);
    snprintf(marketCapStr, sizeof(marketCapStr), "%.2fB", marketCap);
    snprintf(volumeStr, sizeof(volumeStr), "%.2f", volume);
    snprintf(changeStr, sizeof(changeStr), "%.2f%%", change);


    lv_table_set_cell_value(crypto_table, i + 1, 0, coinName);
    lv_table_set_cell_value(crypto_table, i + 1, 1, priceStr);
    lv_table_set_cell_value(crypto_table, i + 1, 2, marketCapStr);
    lv_table_set_cell_value(crypto_table, i + 1, 3, changeStr);
  }
}


void fetch_crypto_lists() {
  if (currentId) {
    return;
  }
  HTTPClient http;
  http.begin(apiURL);
  http.addHeader("accept", "application/json");
  http.addHeader("x-cg-demo-api-key", apiKey);

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    // Serial.println(payload);
    parse_crypo_list(payload);
  } else {
    Serial.println("Failed to fetch data");
  }
  http.end();
}


void parse_crypto_detail(String data) {

  coinDetails = JSON.parse(data);
  if (JSON.typeof(coinDetails) == "undefined") {
    Serial.println("Parsing failed!");
    return;
  }
  coinDetails["id"] = String(currentId);
  String v = JSON.stringify(coinDetails);
  Serial.println(v);

  JSONVar prices = coinDetails["prices"];
  if (prices.length() == 0) return;
  // Initialize min and max with the first price value
  float minY = atof(JSON.stringify(prices[0][0]).c_str());
  float maxY = minY;
  float minX = atof(JSON.stringify(prices[0][1]).c_str());
  float maxX = minX;
  // Find min and max price values
  for (int i = 1; i < prices.length(); i++) {
    float price = atof(JSON.stringify(prices[i][0]).c_str());
    if (price < minY) minY = price;
    if (price > maxY) maxY = price;
  }


  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
  // Normalize and add values to the chart
  for (int i = 0; i < prices.length(); i++) {
    float price = atof(JSON.stringify(prices[i][0]).c_str());
    int normalizedValueY = ((price - minY) / (maxY - minY)) * 100;  // Scale to 0-100
    lv_chart_set_next_value(chart, serY, normalizedValueY);
  }
  // lv_chart_refresh(chart);
  // lv_chart_set_ext_y_array(chart, ser, value_array)
  // lv_chart_refresh(chart)
  // JSONVar coinName = coinDetails["id"];
  // lv_obj_t *label = lv_label_create(chart);
  // lv_label_set_text_fmt(label, "%s Price series (3days)", coinName);
  // lv_obj_align_to(label, chart, LV_ALIGN_BOTTOM_MID, 0, 15);
}



void fetch_crypto_detail() {
  if (!currentId) {
    return;
  }
  const char *url = "https://api.coingecko.com/api/v3/coins/";
  const char *end = "/market_chart?vs_currency=usd&days=1";

  int length = strlen(url) + strlen(end) + strlen(currentId) + 1;
  char apiUrl[length];
  // Format and store in apiUrl
  snprintf(apiUrl, sizeof(apiUrl), "%s%s%s", url, currentId, end);
  // Serial.printf("APP_URL: %s %s", apiUrl, currentId);
  Serial.println();
  // Serial.println(apiUrl);
  if (coinDetails["id"] == String(currentId)) {
    String s = JSON.stringify(coinDetails);
    parse_crypto_detail(s);
    return;
  }

  HTTPClient http;
  http.begin(apiUrl);
  http.addHeader("accept", "application/json");
  http.addHeader("x-cg-demo-api-key", apiKey);

  int httpCode = http.GET();
  if (httpCode > 0) {
    String payload = http.getString();
    // Serial.println(payload);
    parse_crypto_detail(payload);
  } else {
    Serial.println("Failed to fetch data");
  }
  http.end();
}

static void draw_part_event_cb(lv_event_t *e) {

  lv_obj_t *obj = lv_event_get_target(e);
  lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
  /*If the cells are drawn...*/
  if (dsc->part == LV_PART_ITEMS) {
    uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
    uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

    /*Make the texts in the first cell center aligned*/
    if (row == 0) {
      dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
      dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_BLUE), dsc->rect_dsc->bg_color, LV_OPA_20);
      dsc->rect_dsc->bg_opa = LV_OPA_COVER;
    }
    /*In the first column align the texts to the right*/
    else if (col == 0) {
      dsc->label_dsc->align = LV_TEXT_ALIGN_CENTER;
    }

    /*MAke every 2nd row grayish*/
    if ((row != 0 && row % 2) == 0) {
      dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
      dsc->rect_dsc->bg_opa = LV_OPA_COVER;
    }
  }
}



void crypto_list_ui() {
  lv_style_t style;
  lv_style_init(&style);
  lv_style_set_pad_column(&style, 0);
  lv_obj_t *col = lv_obj_create(crypto_list_tile);
  lv_obj_remove_style_all(col);
  lv_obj_set_size(col, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_pad_hor(col, 0, NULL);
  lv_obj_set_style_bg_opa(col, LV_OPA_TRANSP, NULL);
  lv_obj_add_style(col, &style, NULL);

  crypto_list = lv_table_create(col);
  // Create a list inside the tile
  lv_obj_set_size(crypto_list, lv_pct(100), LV_SIZE_CONTENT);
  lv_obj_center(crypto_list);

  crypto_table = lv_table_create(crypto_list);
  lv_table_set_cell_value(crypto_table, 0, 0, "Coin");
  lv_table_set_cell_value(crypto_table, 0, 1, "Price");
  lv_table_set_cell_value(crypto_table, 0, 2, "Market Cap");
  lv_table_set_cell_value(crypto_table, 0, 3, "24h change");
  /*Add an event callback to to apply some custom drawing*/
  lv_obj_add_event_cb(crypto_table, draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
  lv_obj_add_event_cb(crypto_table, cell_select_cb, LV_EVENT_VALUE_CHANGED, NULL);
}



static void event_cb(lv_event_t *e) {
  static int32_t last_id = -1;
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t *obj = lv_event_get_target(e);

  if (code == LV_EVENT_VALUE_CHANGED) {
    last_id = lv_chart_get_pressed_point(obj);
    if (last_id != LV_CHART_POINT_NONE) {
      lv_chart_set_cursor_point(obj, cursor, NULL, last_id);
    }
  } else if (code == LV_EVENT_DRAW_PART_END) {
    lv_obj_draw_part_dsc_t *dsc = lv_event_get_draw_part_dsc(e);
    if (!lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_CURSOR)) return;
    if (dsc->p1 == NULL || dsc->p2 == NULL || dsc->p1->y != dsc->p2->y || last_id < 0) return;

    lv_coord_t *data_array = lv_chart_get_y_array(chart, serY);
    lv_coord_t v = data_array[last_id];
    char buf[16];
    lv_snprintf(buf, sizeof(buf), "%d", v);

    lv_point_t size;
    lv_txt_get_size(&size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

    lv_area_t a;
    a.y2 = dsc->p1->y - 5;
    a.y1 = a.y2 - size.y - 10;
    a.x1 = dsc->p1->x + 10;
    a.x2 = a.x1 + size.x + 10;

    lv_draw_rect_dsc_t draw_rect_dsc;
    lv_draw_rect_dsc_init(&draw_rect_dsc);
    draw_rect_dsc.bg_color = lv_palette_main(LV_PALETTE_BLUE);
    draw_rect_dsc.radius = 1;

    lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

    lv_draw_label_dsc_t draw_label_dsc;
    lv_draw_label_dsc_init(&draw_label_dsc);
    draw_label_dsc.color = lv_color_white();
    a.x1 += 5;
    a.x2 -= 5;
    a.y1 += 5;
    a.y2 -= 5;
    lv_draw_label(dsc->draw_ctx, &draw_label_dsc, &a, buf, NULL);
  }
}

void crypto_detail_ui() {
  chart = lv_chart_create(crypto_detail_tile);
  lv_obj_set_size(chart, 200, 200);
  lv_obj_align(chart, LV_ALIGN_CENTER, 20, 0);

  lv_obj_add_event_cb(chart, event_cb, LV_EVENT_ALL, NULL);
  lv_obj_refresh_ext_draw_size(chart);

  cursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_BLUE), LV_DIR_LEFT | LV_DIR_BOTTOM);

  serY = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
  // serX = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_GREEN), LV_CHART_AXIS_PRIMARY_X);
  lv_chart_set_update_mode(chart, LV_CHART_UPDATE_MODE_SHIFT);
  lv_chart_set_zoom_y(chart, 256 * 5);
  lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 10, 3, 25, 10, true, 25);

  lv_obj_t *label = lv_label_create(chart);
  lv_label_set_text(label, "Click on a point");
  lv_obj_align_to(label, chart, LV_ALIGN_OUT_TOP_MID, 0, 15);
}
