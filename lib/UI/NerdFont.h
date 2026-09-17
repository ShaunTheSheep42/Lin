#pragma once

#include <QString>
#include <utility>

namespace Lin {

constexpr std::pair<const char *, QChar> Icon[3] = {
    std::pair<const char *, QChar>{"none", ' '},
    {"link", QChar(0xDB84)},     // 高代理
    {"link_low", QChar(0xDF62)}, // 低代理
};

}
