module;

#include <cstdint>
#include <limits>

module Basic.ViewPort;
import Basic.Geometry;
import Basic.Canvas;
import Debug;

namespace Lin {
namespace {

void KeepCursorVisible(Position cursor, Position &topLeft, Size size) {
  const int64_t right = static_cast<int64_t>(topLeft.x) + size.x;
  const int64_t bottom = static_cast<int64_t>(topLeft.y) + size.y;
  if (cursor.x < topLeft.x)
    topLeft.x = cursor.x;
  else if (cursor.x >= right)
    topLeft.x = cursor.x - size.x + 1;
  if (cursor.y < topLeft.y)
    topLeft.y = cursor.y;
  else if (cursor.y >= bottom)
    topLeft.y = cursor.y - size.y + 1;
}

} // namespace

ViewPort::ViewPort(const Canvas &canvas, Size size) : canvas(canvas) {
  setSize(size);
}

void ViewPort::moveCursorPosition(int offsetX, int offsetY) {
  const int64_t x = static_cast<int64_t>(cursorPosition.x) + offsetX;
  const int64_t y = static_cast<int64_t>(cursorPosition.y) + offsetY;
  Check(x >= std::numeric_limits<int>::min() &&
            x <= std::numeric_limits<int>::max() &&
            y >= std::numeric_limits<int>::min() &&
            y <= std::numeric_limits<int>::max(),
        "Cursor movement exceeds the supported coordinate range");
  cursorPosition = {static_cast<int>(x), static_cast<int>(y)};
  KeepCursorVisible(cursorPosition, topLeft, size);
}

Position ViewPort::getCursorPosition() const { return cursorPosition; }
void ViewPort::setCursorPosition(Position p) {
  cursorPosition = p;
  KeepCursorVisible(cursorPosition, topLeft, size);
}

void ViewPort::setSize(Size newSize) {
  Check(newSize.x > 0 && newSize.y > 0,
        "Viewport size must be positive, got {}x{}", newSize.x, newSize.y);
  size = newSize;
  setCursorPosition(cursorPosition);
}

Size ViewPort::getSize() const { return size; }

void ViewPort::setTopLeft(Position p) { topLeft = p; }

Position ViewPort::getTopLeft() const { return topLeft; }

bool ViewPort::contains(Position p) const {
  return p.x >= topLeft.x && p.y >= topLeft.y &&
         static_cast<int64_t>(p.x) < static_cast<int64_t>(topLeft.x) + size.x &&
         static_cast<int64_t>(p.y) < static_cast<int64_t>(topLeft.y) + size.y;
}

uint64_t ViewPort::getCursorPositionBlockId() const {
  return getBlockId(cursorPosition);
}

uint64_t ViewPort::getCursorPositionGroupId() const {
  return getGroupId(cursorPosition);
}

uint64_t ViewPort::getBlockId(Position p) const {
  return canvas.getBlockIdAt(p);
}

uint64_t ViewPort::getGroupId(Position p) const {
  return canvas.getGroupIdAt(p);
}

} // namespace Lin
