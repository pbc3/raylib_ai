#pragma once
#include <cmath>

struct Vec2 {
  float x, y;
  float length() { return std::hypot(x, y); }

  float distance(Vec2 other) {
    float dx = other.x - x;
    float dy = other.y - y;
    float b = (dx * dx) + (dy * dy);
    return std::sqrt(b);
  }
  bool operator==(Vec2 rhs) { return this->x == rhs.x && this->y == rhs.y; }
  bool operator!=(Vec2 rhs) { return !(*this == rhs); }
  void normalise() {
    float len = length();
    // if (IsNearlyZero(len)) {
    //	return;
    // }
    if (!len) {
      // TODO - this is getting called off start!
      return;
    }
    this->x /= len;
    this->y /= len;
  }
  Vec2 operator+(Vec2 rhs) { return {this->x + rhs.x, this->y + rhs.y}; }
  Vec2 normalised() {
    float len = length();
    // if (IsNearlyZero(len)) {
    //	return;
    // }
    if (!len) {
      return {0, 0};
    }
    return {x / len, y / len};
  }

  void rotate(float rads) {
    float c = std::cosf(rads);
    float s = std::sinf(rads);
    float new_x = x * c - y * s;
    float new_y = x * s + y * c;
    x = new_x;
    y = new_y;
  }

  float dot(Vec2 rhs) { return (x * rhs.x) + (y * rhs.y); }
  Vec2 operator-(Vec2 rhs) { return {x - rhs.x, y - rhs.y}; }
  Vec2 &operator*=(float scalar) {
    this->x *= scalar;
    this->y *= scalar;
    return *this;
  }
  Vec2 operator*(float scalar) { return {x * scalar, y * scalar}; }

  Vec2 &operator+=(Vec2 rhs) {
    this->x += rhs.x;
    this->y += rhs.y;
    return *this;
  }
};

inline Vec2 centre_to_top_left(Vec2 centre_coords, float size) {
  float half_s = size / 2;
  float x = centre_coords.x - half_s;
  float y = centre_coords.y - half_s;
  return {x, y};
};

struct Ray_ {
  Vec2 origin;
  Vec2 direction;
  Vec2 at(float t) { return origin + (direction * t); }
};