#include <pebble.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "gcolor_definitions.h"
#include "model.h"
#include "stronger_math.h"
#include "trig.h"

static Window *s_main_window;
static Layer *dots_layer;

#define yes true
#define no false

#define NELEMS(x) (sizeof(x) / sizeof((x)[0]))

#define DOTS_COUNT VERTICES_LENGTH

static vec3 accel_data;

static basis3 basis;

char *vec3_str(vec3 v) {
  static char string[16];
  snprintf(string, sizeof(string), "%03d, %03d, %03d", (int)(v.x * 100),
           (int)(v.y * 100), (int)(v.z * 100));
  return string;
}

char *basis_str(basis3 b) {
  static char string[48];
  char *x = vec3_str(b.x);
  char *y = vec3_str(b.y);
  char *z = vec3_str(b.z);
  snprintf(string, sizeof(string), "%s, %s, %s", x, y, z);
  // free(x);
  // free(y);
  // free(z);
  return string;
}

static vec3 convert_accel_data(AccelData pebble) {
  return VEC3(pebble.x / 1000.0, pebble.y / 1000.0, pebble.z / 1000.0);
}

static void main_window_unload(Window *window) { layer_destroy(dots_layer); }

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  // redraw_text();
}

static void accel_data_handler(AccelData *data, uint32_t count) {
  vec3 new_accel_data = convert_accel_data(*data);
  accel_data =
      add_v3(accel_data, mulf_v3(sub_v3(new_accel_data, accel_data), 0.3));
  basis =
      orthonormalize_b3(rotate_b3(rotate_b3(basis, VEC3_Z, -accel_data.x * 0.1),
                                  VEC3_X, -accel_data.y * 0.1));
  // vec3 accel_vec = normalize_v3(accel_data);

  // get an orthonormalized basis
  // basis.z = accel_vec;
  // basis.x = normalize_v3(cross(VEC3_X, basis.z));
  // basis.y = cross(basis.x, basis.z);

  // char *bs = basis_str(basis);
  // APP_LOG(APP_LOG_LEVEL_DEBUG, "%s", bs);
  // free(bs);
}

static GPoint convert_vec2(float x, float y, GRect bounds) {
  int16_t min_dimension;
  if (bounds.size.w < bounds.size.h) {
    min_dimension = bounds.size.w;
  } else {
    min_dimension = bounds.size.h;
  }

  return GPoint((x * min_dimension + bounds.size.w) / 2,
                (y * min_dimension + bounds.size.h) / 2);
}

static GPoint points[DOTS_COUNT];
static uint16_t points_sizes[DOTS_COUNT];

static void dots_update_handler(Layer *layer, GContext *ctx) {
  GRect layer_frame = layer_get_bounds(layer);

  // vec3 dots[DOTS_COUNT];

  // for (size_t i = 0; i < DOTS_COUNT; i++) {
  //   float y = 1 - (i / (float)(DOTS_COUNT - 1)) * 2;
  //   float theta = PHI * i;
  //   float radius = sqrt_approx(1.0 - y * y);
  //   vertices[i] = VEC3(pebble_cos(theta) * radius, y, pebble_sin(theta) *
  //   radius);
  // }

  for (size_t i = 0; i < DOTS_COUNT; i++) {
    Vertex v = vertices[i];
    vec3 xformed =
        add_v3(basis_xform_3(v.position, basis), mulf_v3(VEC3_Y, 0.75));
    vec3 xformed_normal = basis_xform_3(v.normal, basis);
    // if (xformed_normal.y > 0) {
    //   points_sizes[i] = -1;
    //   continue;
    // }
    float recip_depth = 1.0 / xformed.y;
    points[i] = convert_vec2(xformed.x * recip_depth, xformed.z * recip_depth,
                             layer_frame);
    points_sizes[i] = (uint16_t)(recip_depth * -xformed_normal.y + 1);
  }

  graphics_context_set_antialiased(ctx, no);
  graphics_context_set_fill_color(ctx, GColorWhite);
  graphics_context_set_compositing_mode(ctx, GCompOpAssign);
  for (size_t i = 0; i < DOTS_COUNT; i++) {
    if (points_sizes[i] > 0)
      graphics_fill_circle(ctx, points[i], points_sizes[i]);
  }
}

static void redraw_timer_handler(void *data) {
  layer_mark_dirty(dots_layer);
  app_timer_register(100, redraw_timer_handler, NULL);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  dots_layer = layer_create(bounds);

  // Add it as a child layer to the Window's root layer
  layer_add_child(window_layer, dots_layer);

  AccelData *pebble_accel = (AccelData *)malloc(sizeof(AccelData));
  accel_service_peek(pebble_accel);
  accel_data = convert_accel_data(*pebble_accel);
  free(pebble_accel);

  accel_data_service_subscribe(1, accel_data_handler);

  redraw_timer_handler(NULL);
}

static void init() {
  basis = BASIS3_IDENTITY;
  // Create main Window element and assign to pointer
  s_main_window = window_create();
  window_set_background_color(s_main_window, GColorBlack);

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(
      s_main_window,
      (WindowHandlers){.load = main_window_load, .unload = main_window_unload});

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);
  layer_set_update_proc(dots_layer, dots_update_handler);
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

  APP_LOG(APP_LOG_LEVEL_DEBUG, "%d", sizeof(vertices));
}

static void deinit() { window_destroy(s_main_window); }

int main(void) {
  init();
  app_event_loop();
  deinit();
}
