
struct Vec2 {
  float x, y;
  float length() { return std::hypot(x, y); }
  
  float distance(Vec2 other){
    float dx = other.x - x;
    float dy = other.y - y;
    float b = (dx * dx) + (dy * dy);
    return std::sqrt(b);
  }

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