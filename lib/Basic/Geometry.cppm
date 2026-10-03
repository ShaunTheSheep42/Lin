module;

#include <cstdint>
export module Basic.Geometry;

export namespace Lin {

class XYPair {
public:
  int x = 0;
  int y = 0;
  XYPair(int x, int y) : x(x), y(y) {}

  bool operator==(XYPair p) { return this->x == p.x && this->y == p.y; }

  XYPair operator+(XYPair p) const { return {this->x + p.x, this->y + p.y}; }
  XYPair &operator+=(XYPair p) {
    this->x += p.x;
    this->y += p.y;
    return *this;
  }

  XYPair operator-(XYPair p) const { return {this->x - p.x, this->y - p.y}; }
  XYPair &operator-=(XYPair p) {
    x -= p.x;
    y -= p.y;
    return *this;
  }
};

using Position = XYPair;
using Size = XYPair;

[[nodiscard]] uint64_t Encode(int x, int y) {
  return (uint64_t(uint32_t(y)) << 32) | uint32_t(x);
}

[[nodiscard]] uint64_t Encode(XYPair p) {
  return (uint64_t(uint32_t(p.y)) << 32) | uint32_t(p.x);
}

[[nodiscard]] XYPair Decode(uint64_t key) {
  return {static_cast<int>(uint32_t(key)), static_cast<int>(key >> 32)};
}

} // namespace Lin
