#pragma once

#include "UI/Theme.h"
#include <QPoint>
#include <optional>

namespace Lin {

struct Config {
  bool useCursorLastPos = true;
  ThemeKind themeKind = ThemeKind::Normal;
  bool isTransparent = true;
  double opacity = 0.8;
};

std::optional<Config> LoadConfig(QString path);

} // namespace Lin
