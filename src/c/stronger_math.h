#pragma once
#include <pebble.h>

typedef struct {
  float x;
  float y;
  float z;
} vec3;

#define VEC3(x, y, z)                                                          \
  (vec3) { x, y, z }

float inv_sqrt(float x) {
  float xhalf = 0.5f * x;
  union {
    float x;
    int i;
  } u;
  u.x = x;
  u.i = 0x5f375a86 - (u.i >> 1);
  /* The next line can be repeated any number of times to increase accuracy */
  u.x = u.x * (1.5f - xhalf * u.x * u.x);
  return u.x;
}

float dot(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

float length_sqr(vec3 v) { return dot(v, v); }

float inv_length(vec3 x) { return inv_sqrt(length_sqr(x)); }

vec3 multiply(vec3 a, float b) { return VEC3(a.x * b, a.y * b, a.z * b); }

vec3 divide(vec3 a, float b) { return VEC3(a.x / b, a.y / b, a.z / b); }

vec3 normalize(vec3 x) { return multiply(x, inv_length(x)); }

vec3 cross(vec3 a, vec3 b) {
  return VEC3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x);
}

vec3 basis_xform(vec3 a, vec3 x, vec3 y, vec3 z) {
  return VEC3(dot(a, x), dot(a, y), dot(a, z));
}
