#include <pebble.h>

#define DOT_SIZE 9
#define X_OFFSET 5
#define Y_OFFSET 5
#define MAZE_SIZE 15
#define DIGIT_SIZE 5

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

static void load_resources() {

}

static void destroy_resources() {

}

static void draw_dot(GContext *ctx, int x, int y, bool show) {
  int x_position = x * DOT_SIZE;
  int y_position = y * DOT_SIZE;

  GRect rect = GRect(x_position, y_position, DOT_SIZE, DOT_SIZE);

  if (show) {
    graphics_context_set_fill_color(ctx, GColorBlack);
  } else {
    graphics_context_set_fill_color(ctx, GColorWhite);
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
  fill_layer(layer, ctx, GColorBlack);
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

static void update_maze(Layer *layer, GContext *ctx) {
  draw_maze_background(ctx, layer);
  draw_time(ctx);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  layer_mark_dirty(maze_layer);
}

static void main_window_load(Window *window) {
  load_resources();

  Layer *main_window_layer = window_get_root_layer(main_window);
  layer_set_update_proc(main_window_layer, update_main_layer);
  GRect bounds = layer_get_bounds(main_window_layer);

  GRect frame = GRect(X_OFFSET, bounds.size.h - Y_OFFSET - MAZE_SIZE * DOT_SIZE, MAZE_SIZE * DOT_SIZE, MAZE_SIZE * DOT_SIZE);
  maze_layer = layer_create(frame);
  layer_add_child(main_window_layer, maze_layer);
  layer_set_update_proc(maze_layer, update_maze);
}

static void main_window_unload(Window *window) {
  destroy_resources();
}

static void init() {
  main_window = window_create();

  window_set_window_handlers(main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(main_window, true);
  tick_timer_service_subscribe(SECOND_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
