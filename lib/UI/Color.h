#pragma once

#include <QColor>

namespace Lin {

struct Color {
  QColor font;
  QColor background; // Background when opaque
  QColor cursor;
};

} // namespace Lin
