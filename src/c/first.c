#include "pebble_fonts.h"
#include <pebble.h>
#include <stdio.h>

#include "stronger_math.h"

vec3 *accel_vec;

static Window *s_main_window;
static TextLayer *s_time_layer;

static void convert_accel_data(AccelData *pebble, vec3 *dvector) {
  dvector->x = pebble->x / 1000.0f;
  dvector->y = pebble->y / 1000.0f;
  dvector->z = pebble->z / 1000.0f;
}

static void main_window_unload(Window *window) {
  text_layer_destroy(s_time_layer);
}

static void redraw_text() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[32];
  // strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
  // "%H:%M" : "%I:%M", tick_time);
  snprintf(s_buffer, sizeof(s_buffer), "%d, %d, %d",
           (int)(accel_vec->x * 10),
           (int)(accel_vec->y * 10),
           (int)(accel_vec->z * 10));

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // redraw_text();
}

static void accel_data_handler(AccelData *data, uint32_t count) {
  convert_accel_data(data, accel_vec);
  *accel_vec = vec3Normalize(*accel_vec);
  redraw_text();
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  // Create the TextLayer with specific bounds
  s_time_layer =
      text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(58, 52), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

  accel_vec = (vec3 *)malloc(sizeof(vec3));
  AccelData *pebble_accel = (AccelData *)malloc(sizeof(AccelData));
  accel_service_peek(pebble_accel);
  convert_accel_data(pebble_accel, accel_vec);
  accel_data_service_subscribe(1, accel_data_handler);
}

static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(
      s_main_window,
      (WindowHandlers){.load = main_window_load, .unload = main_window_unload});

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  window_destroy(s_main_window);
  free(accel_vec);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
