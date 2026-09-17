#pragma once

#include "Basic/Char.h"
#include "Support/Hash.h"
#include <unordered_map>

namespace Lin {

inline std::unordered_map<QString, Char> Icons = {
    {"Google", MakeChar(0xE7F0, true)},                  // " "
    {"Vim", MakeChar(0xE7C5, true)},                     // " "
    {"NeoVim", MakeChar(0xE6AE, true)},                  // " "
    {"Dragon", MakeChar(0xEEF8, true)},                  // " "
    {"GoogleTranslate", MakeChar(0xDB80, 0xDEBF, true)}, // "󰊿 "
    {"Dino", MakeChar(0xdb84, 0xDF62, true)},            // "󱍢 "
    {"Video", MakeChar(0xdb82, 0xde1c, true)},           // "󰨜 "

};

} // namespace Lin
