module;

#include <cstdint>

export module Basic.ViewPort;

import Basic.Geometry;
import Basic.Canvas;

export namespace Lin {

class ViewPort {
public:
  explicit ViewPort(const Canvas &canvas, Size size = Size{80, 24});

  void moveCursorPosition(int offsetX, int offsetY);
  void setCursorPosition(Position p);
  Position getCursorPosition() const;
  void setSize(Size size);
  Size getSize() const;
  void setTopLeft(Position p);
  Position getTopLeft() const;
  bool contains(Position p) const;

  uint64_t getCursorPositionBlockId() const;
  uint64_t getCursorPositionGroupId() const;
  uint64_t getBlockId(Position p) const;
  uint64_t getGroupId(Position p) const;

private:
  const Canvas &canvas;
  Position cursorPosition{0, 0};
  Position topLeft{0, 0};
  Size size{80, 24};
};

void EncodeViewPort();
void DecodeViewPort();

} // namespace Lin
