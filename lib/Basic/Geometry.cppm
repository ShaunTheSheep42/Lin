module;

export module Basic.Geometry;

export namespace Lin {

struct XYPair {
  int x = 0;
  int y = 0;

  friend bool operator==(XYPair a, XYPair b) {
    return a.x == b.x && a.y == b.y;
  }

  friend XYPair operator+(XYPair a, XYPair b) { return {a.x + b.x, a.y + b.y}; }
  friend XYPair &operator+=(XYPair &a, XYPair b) {
    a.x += b.x;
    a.y += b.y;
    return a;
  }
  friend XYPair operator-(XYPair a, XYPair b) { return {a.x - b.x, a.y - b.y}; }
  friend XYPair &operator-=(XYPair &a, XYPair b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
  }
};

using Position = XYPair;

} // namespace Lin
