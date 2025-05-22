#pragma once

#include <stdint.h>

#include "trig.h"

typedef struct {
  float x;
  float y;
  float z;
} vec3;

typedef struct {
  vec3 x;
  vec3 y;
  vec3 z;
} basis3;

#define VEC3(x, y, z) ((vec3){x, y, z})

#define VEC3_X VEC3(1.0, 0.0, 0.0)
#define VEC3_Y VEC3(0.0, 1.0, 0.0)
#define VEC3_Z VEC3(0.0, 0.0, 1.0)

#define BASIS3_IDENTITY ((basis3){VEC3_X, VEC3_Y, VEC3_Z})

float sqrt_approx(float z) {
  union {
    float f;
    uint32_t i;
  } val = {z}; /* Convert type, preserving bit pattern */
  /*
   * To justify the following code, prove that
   *
   * ((((val.i / 2^m) - b) / 2) + b) * 2^m = ((val.i - 2^m) / 2) + ((b + 1) / 2)
   * * 2^m)
   *
   * where
   *
   * b = exponent bias
   * m = number of mantissa bits
   */
  val.i -= 1 << 23; /* Subtract 2^m. */
  val.i >>= 1;      /* Divide by 2. */
  val.i += 1 << 29; /* Add ((b + 1) / 2) * 2^m. */

  return val.f; /* Interpret again as float */
}

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

float dot_v3(vec3 a, vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

float length_sqr_v3(vec3 v) { return dot_v3(v, v); }

float inv_length_v3(vec3 x) { return inv_sqrt(length_sqr_v3(x)); }

vec3 mulf_v3(vec3 a, float b) { return VEC3(a.x * b, a.y * b, a.z * b); }

vec3 divf_v3(vec3 a, float b) { return VEC3(a.x / b, a.y / b, a.z / b); }

vec3 addf_v3(vec3 a, float b) { return VEC3(a.x + b, a.y + b, a.z + b); }

vec3 subf_v3(vec3 a, float b) { return VEC3(a.x - b, a.y - b, a.z - b); }

vec3 mul_v3(vec3 a, vec3 b) { return VEC3(a.x * b.x, a.y * b.y, a.z * b.z); }

vec3 div_v3(vec3 a, vec3 b) { return VEC3(a.x / b.x, a.y / b.y, a.z / b.z); }

vec3 add_v3(vec3 a, vec3 b) { return VEC3(a.x + b.x, a.y + b.y, a.z + b.z); }

vec3 sub_v3(vec3 a, vec3 b) { return VEC3(a.x - b.x, a.y - b.y, a.z - b.z); }

vec3 normalize_v3(vec3 x) { return mulf_v3(x, inv_length_v3(x)); }

vec3 cross(vec3 a, vec3 b) {
  return VEC3(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z,
              a.x * b.y - a.y * b.x);
}

basis3 basis3_transpose(basis3 basis) {
  return (basis3){.x = VEC3(basis.x.x, basis.y.x, basis.z.x),
                  .y = VEC3(basis.x.y, basis.y.y, basis.z.y),
                  .z = VEC3(basis.x.z, basis.y.z, basis.z.z)};
}

vec3 basis_xform_3(vec3 a, basis3 basis) {
  basis3 t_basis = basis3_transpose(basis);
  return VEC3(dot_v3(a, t_basis.x), dot_v3(a, t_basis.y), dot_v3(a, t_basis.z));
}

vec3 rotate_v3(vec3 v, vec3 axis, float angle) {
  // v * cos(angle) + (axis x v) * sin(angle) + axis * (axis . v) * (1 -
  // cos(angle))
  float cos = pebble_cos(angle);
  float sin = pebble_sin(angle);
  return add_v3(add_v3(mulf_v3(v, cos), mulf_v3(cross(axis, v), sin)),
                mulf_v3(axis, dot_v3(axis, v) * (1.0 - cos)));
}

basis3 rotate_b3(basis3 basis, vec3 axis, float angle) {
  return (basis3){rotate_v3(basis.x, axis, angle),
                  rotate_v3(basis.y, axis, angle),
                  rotate_v3(basis.z, axis, angle)};
}

basis3 normalize_b3(basis3 basis) {
  return (basis3){
      normalize_v3(basis.x),
      normalize_v3(basis.y),
      normalize_v3(basis.z),
  };
}

basis3 orthonormalize_b3(basis3 basis) {
  vec3 x = basis.x;
  vec3 y = basis.y;
  vec3 z = basis.z;
  x = normalize_v3(x);
  y = sub_v3(y, mulf_v3(x, dot_v3(x, y)));
  y = normalize_v3(y);
  z = sub_v3(sub_v3(z, mulf_v3(x, dot_v3(x, z))), mulf_v3(y, dot_v3(y, z)));
  z = normalize_v3(z);
  return (basis3) { x, y, z };
}
