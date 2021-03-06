#include <pebble.h>
#include <time.h>

#define DOT_SIZE 9
#define X_OFFSET PBL_IF_RECT_ELSE(5, 22)
#define Y_OFFSET PBL_IF_RECT_ELSE(5, 18)
#define ICON_OFFSET 3
#define MAZE_SIZE 15
#define DIGIT_SIZE 5

#define BATTERY_WIDTH 15
#define BATTERY_HEIGHT 7

#define BT_WIDTH 7
#define BT_HEIGHT 11

#define QUIET_WIDTH 8
#define QUIET_HEIGHT 9

#define STATUS_WIDTH (BT_WIDTH + QUIET_WIDTH + ICON_OFFSET)
#define STATUS_HEIGHT 11

#define DATE_HEIGHT 21

#define DATE_Y_OFFSET PBL_IF_RECT_ELSE(-2, 4)
#define DATE_FORMAT "%a %D"
#define DATE_BUFFER_LEN 30

#define BG_COLOR GColorBlack
#define FG_COLOR PBL_IF_COLOR_ELSE(GColorScreaminGreen, GColorWhite)
#define TITLE_COLOR GColorWhite

static const int maze_background[MAZE_SIZE] = {
  0b000001000010010,
  0b010111011111000,
  0b110000010000011,
  0b010000010000010,
  0b010000000000010,
  0b000000010000010,
  0b010000010000011,
  0b011111000111110,
  0b110000010000000,
  0b010000010000011,
  0b010000010000010,
  0b010000010000010,
  0b010000010000000,
  0b110111110111110,
  0b000001000100010
};

static const int digits[10][DIGIT_SIZE] = {
  // 0
  {0b11111,
   0b10001,
   0b10101,
   0b10001,
   0b11111},
  // 1
  {0b10101,
   0b10101,
   0b10111,
   0b11101,
   0b10101},
  // 2
  {0b11111,
   0b00001,
   0b11111,
   0b10000,
   0b11111},
  // 3
  {0b11111,
   0b00001,
   0b11111,
   0b00001,
   0b11111},
  // 4
  {0b10101,
   0b10101,
   0b11111,
   0b00001,
   0b11101},
  // 5
  {0b11111,
   0b10000,
   0b11111,
   0b00001,
   0b11111},
  // 06
  {0b11111,
   0b10000,
   0b11111,
   0b10001,
   0b11111},
  // 7
  {0b11111,
   0b10001,
   0b11101,
   0b00101,
   0b10101},
  // 8
  {0b11111,
   0b10001,
   0b11111,
   0b10001,
   0b11111},
  // 9
  {0b11111,
   0b10001,
   0b11111,
   0b00001,
   0b11111}
};

static Window *main_window;
static Layer *maze_layer;
static Layer *battery_layer;
static Layer *status_layer;
static TextLayer *date_layer;

static GBitmap *bmp_quiet;
static GBitmap *bmp_bt;
static GBitmap *bmp_battery;

static char date_buffer[DATE_BUFFER_LEN];

static void load_resources() {
  bmp_quiet = gbitmap_create_with_resource(RESOURCE_ID_QUIET);
  bmp_bt = gbitmap_create_with_resource(RESOURCE_ID_BT);
  bmp_battery = gbitmap_create_with_resource(RESOURCE_ID_BATTERY);
}

static void destroy_resources() {
  gbitmap_destroy(bmp_quiet);
  gbitmap_destroy(bmp_bt);
  gbitmap_destroy(bmp_battery);
}

static void draw_dot(GContext *ctx, int x, int y, bool show) {
  int x_position = x * DOT_SIZE;
  int y_position = y * DOT_SIZE;

  GRect rect = GRect(x_position, y_position, DOT_SIZE, DOT_SIZE);

  if (show) {
    graphics_context_set_fill_color(ctx, BG_COLOR);
  } else {
    graphics_context_set_fill_color(ctx, FG_COLOR);
  }

  graphics_fill_rect(ctx, rect, 0, GCornerNone);
}

static void draw_digit(GContext *ctx, int digit, int x, int y) {
  int offset_x = 2 + x * (DIGIT_SIZE + 1);
  int offset_y = 2 + y * (DIGIT_SIZE + 1);

  for (int i = 0; i < DIGIT_SIZE; i++) {
    int line = digits[digit][i];
    for (int j = 0; j < DIGIT_SIZE; j++) {
      int dot = (line >> (DIGIT_SIZE - 1 - j)) & 0b1;

      int dot_x = offset_x + j;
      int dot_y = offset_y + i;

      draw_dot(ctx, dot_x, dot_y, !dot);
    }
  }
}

static void draw_maze_background(GContext *ctx, Layer *layer) {
  for (int i = 0; i < MAZE_SIZE; i++) {
    int line = maze_background[i];
    for (int j = 0; j < MAZE_SIZE; j++) {
      int dot = (line >> (MAZE_SIZE - 1 - j)) & 0b1;

      draw_dot(ctx, j, i, dot);
    }
  }
}

static void fill_layer(Layer *layer, GContext *ctx, GColor color) {
  GRect bounds = layer_get_bounds(layer);
  graphics_context_set_fill_color(ctx, color);
  graphics_fill_rect(ctx, bounds, 0, GCornerNone);
}

static void update_main_layer(Layer *layer, GContext* ctx) {
  fill_layer(layer, ctx, BG_COLOR);
}

static void draw_time(GContext *ctx) {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  int hours = tick_time->tm_hour;
  if (!clock_is_24h_style()) {
    hours = hours % 12;
    if (hours == 0) {
      hours = 12;
    }
  }

  int hour_dicker = hours / 10;
  int hour_unit = hours % 10;

  int min_dicker = tick_time->tm_min / 10;
  int min_unit = tick_time->tm_min % 10;

  draw_digit(ctx, hour_dicker, 0, 0);
  draw_digit(ctx, hour_unit, 1, 0);
  draw_digit(ctx, min_dicker, 0, 1);
  draw_digit(ctx, min_unit, 1, 1);
}

static void draw_battery(GContext *ctx, int index) {
  GRect img_rect = GRect(0, BATTERY_HEIGHT * index, BATTERY_WIDTH, BATTERY_HEIGHT);
  GBitmap *img = gbitmap_create_as_sub_bitmap(bmp_battery, img_rect);

  graphics_draw_bitmap_in_rect(ctx, img, GRect(0, 0, BATTERY_WIDTH, BATTERY_HEIGHT));

  gbitmap_destroy(img);
}

static void update_maze(Layer *layer, GContext *ctx) {
  draw_maze_background(ctx, layer);
  draw_time(ctx);
}

static void update_status(Layer *layer, GContext *ctx) {
  GRect rect = GRect(0, 0, STATUS_WIDTH, STATUS_HEIGHT);
  graphics_context_set_fill_color(ctx, BG_COLOR);

  graphics_fill_rect(ctx, rect, 0, GCornerNone);

  int x = STATUS_WIDTH;
  if (bluetooth_connection_service_peek()) {
    rect = GRect(x - BT_WIDTH, (STATUS_HEIGHT - BT_HEIGHT) / 2, BT_WIDTH, BT_HEIGHT);
    graphics_draw_bitmap_in_rect(ctx, bmp_bt, rect);
    x -= (BT_WIDTH + ICON_OFFSET);
  }

  if (quiet_time_is_active()) {
    rect = GRect(x - QUIET_WIDTH, (STATUS_HEIGHT - QUIET_HEIGHT) / 2, QUIET_WIDTH, QUIET_HEIGHT);
    graphics_draw_bitmap_in_rect(ctx, bmp_quiet, rect);
  }
}

static void update_battery(Layer *layer, GContext *ctx) {
  BatteryChargeState charge_state = battery_state_service_peek();
  int img_index;
  if (charge_state.is_charging) {
    img_index = 11;
  } else if (charge_state.is_plugged) {
    img_index = 12;
  } else {
    img_index = charge_state.charge_percent / 10;
  }

  draw_battery(ctx, img_index);
}

static void update_date() {
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  memset(date_buffer, 0, DATE_BUFFER_LEN);
  strftime(date_buffer, DATE_BUFFER_LEN, DATE_FORMAT, tick_time);

  text_layer_set_text(date_layer, date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(maze_layer);
  update_date();
}

static void bt_handler(bool connected) {
  layer_mark_dirty(status_layer);
}

static void battery_handler(BatteryChargeState charge_state) {
  layer_mark_dirty(battery_layer);
}

static void main_window_load(Window *window) {
  load_resources();

  Layer *main_window_layer = window_get_root_layer(main_window);
  layer_set_update_proc(main_window_layer, update_main_layer);
  GRect bounds = layer_get_bounds(main_window_layer);

  GRect maze_frame = GRect(X_OFFSET, bounds.size.h - Y_OFFSET - MAZE_SIZE * DOT_SIZE, MAZE_SIZE * DOT_SIZE, MAZE_SIZE * DOT_SIZE);
  maze_layer = layer_create(maze_frame);
  layer_add_child(main_window_layer, maze_layer);
  layer_set_update_proc(maze_layer, update_maze);

#if defined(PBL_RECT)
  int w = bounds.size.w - X_OFFSET;
  GRect status_frame = GRect(w - STATUS_WIDTH - BATTERY_WIDTH - ICON_OFFSET, Y_OFFSET, STATUS_WIDTH, STATUS_HEIGHT);
  GRect battery_frame = GRect(w - BATTERY_WIDTH, Y_OFFSET + (BT_HEIGHT - BATTERY_HEIGHT) / 2, BATTERY_WIDTH, BATTERY_HEIGHT);
  GRect date_rect = GRect(X_OFFSET, DATE_Y_OFFSET, w - STATUS_WIDTH - BATTERY_WIDTH - ICON_OFFSET * 2 - X_OFFSET, DATE_HEIGHT);
#else
  int w = bounds.size.w;
  int h = bounds.size.h;
  int status_battery_width = STATUS_WIDTH + ICON_OFFSET + BATTERY_WIDTH;
  int status_x = (w - status_battery_width) / 2;
  GRect status_frame = GRect(status_x, h - DATE_Y_OFFSET - STATUS_HEIGHT, STATUS_WIDTH, STATUS_HEIGHT);;
  GRect battery_frame = GRect(status_x + STATUS_WIDTH + ICON_OFFSET, h - DATE_Y_OFFSET - STATUS_HEIGHT + (STATUS_HEIGHT - BATTERY_HEIGHT) / 2, BATTERY_WIDTH, BATTERY_HEIGHT);
  GRect date_rect = GRect(0, DATE_Y_OFFSET, w, DATE_HEIGHT);
#endif
  
  status_layer = layer_create(status_frame);
  layer_add_child(main_window_layer, status_layer);
  layer_set_update_proc(status_layer, update_status);

  battery_layer = layer_create(battery_frame);
  layer_add_child(main_window_layer, battery_layer);
  layer_set_update_proc(battery_layer, update_battery);

  date_layer = text_layer_create(date_rect);
  text_layer_set_text_alignment(date_layer, PBL_IF_RECT_ELSE(GTextAlignmentLeft, GTextAlignmentCenter));
  text_layer_set_text_color(date_layer, TITLE_COLOR);
  text_layer_set_background_color(date_layer, BG_COLOR);
  text_layer_set_font(date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  layer_add_child(main_window_layer, text_layer_get_layer(date_layer));
}

static void main_window_unload(Window *window) {
  layer_destroy(maze_layer);
  layer_destroy(status_layer);
  layer_destroy(battery_layer);
  text_layer_destroy(date_layer);

  destroy_resources();
}

static void init() {
  setlocale(LC_TIME, "");
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(main_window, true);

  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  bluetooth_connection_service_subscribe(bt_handler);
  battery_state_service_subscribe(battery_handler);
}

static void deinit() {
  tick_timer_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  battery_state_service_unsubscribe();

  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
