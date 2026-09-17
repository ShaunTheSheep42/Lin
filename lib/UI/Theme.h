#pragma once

#include "UI/Color.h"
#include "UI/Font.h"
#include <qfont.h>

namespace Lin {

enum class ThemeKind { Normal };

class Theme {
public:
  ThemeKind kind;

  Theme(ThemeKind kind) {}
  ~Theme() = default;

  Color color;
  Font normalfont;
};

class NormalTheme : public Theme {
public:
  NormalTheme() : Theme(ThemeKind::Normal) {
    color = {
        .font = Qt::white,
        .background = Qt::black,
        .cursor = QColor(100, 150, 255, 120),
    };

    QFont chinese("YEFONTBoBoTi");
    QFont english("RecMonoCasual Nerd Font");
    QFont icon("RecMonoCasual Nerd Font");
    chinese.setPixelSize(18);
    english.setPixelSize(16);
    // english.setStyleStrategy(QFont::NoSubpixelAntialias);
    icon.setPixelSize(16);
    // icon.setStyleStrategy(QFont::NoSubpixelAntialias);

    normalfont = {
        .chinese = chinese,
        .english = english,
        .icon = icon,
    };
  }
};

} // namespace Lin
