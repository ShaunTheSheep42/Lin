module;

#include <string>

module Basic.String;

import Basic.Char;

namespace lin {

template <typename T> String MakeString(T c) {
  String s;
  s.push_back(c);
  return s;
}

String MakeString(const std::u32string &s32) {
  String s;
  for (const auto &c : s32) {
    s.push_back(MakeChar(c));
    if (IsFullWidth(c))
      s.push_back(PlaceHolder);
  }

  return s;
}

std::u32string ToStdU32String(const String &s) {
  std::u32string s32;
  for (const auto &c : s)
    s32.push_back(ToRawChar(c));

  return s32;
}

} // namespace lin
