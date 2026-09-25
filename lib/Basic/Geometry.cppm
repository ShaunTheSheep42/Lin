module;

export module Basic;

namespace Lin {

struct Point {
  int x = 0;
  int y = 0;

  friend bool operator==(Point lhs, Point rhs) {
    return lhs.x == rhs.x && lhs.y == rhs.y;
  }
};

struct Size {
  int width = 0;
  int height = 0;
};

struct Rect {
  Point origin;
  Size size;

  bool contains(Point point) const {
    return point.x >= origin.x && point.y >= origin.y &&
           point.x < origin.x + size.width && point.y < origin.y + size.height;
  }
};

} // namespace Lin
