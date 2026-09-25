module;

export module Basic.Geometry;

namespace lin {

struct XYPair {
  int x = 0;
  int y = 0;

  friend bool operator==(XYPair lhs, XYPair rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
};

} // namespace lin
