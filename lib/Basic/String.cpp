module;

#include <string>
#include <vector>

module Basic.String;

import Basic.Char;

namespace Lin {

template <typename T> String MakeString(T c) {
  String s;
  s.push_back(c);
  return s;
}

String MakeString(const std::u32string &s32) {
  String s;
  std::vector<Char> chars;
  for (const auto &c32 : s32) {
    Char c = MakeChar(c32);
    if (c == '\n') {
      s.push_back(chars);
      chars.clear();
      continue;
    }

    chars.push_back(c);
    if (IsFullWidth(c))
      chars.push_back(PlaceHolder);
  }

  return s;
}

std::u32string ToStdU32String(const String &s) {
  std::u32string s32;
  for (const auto &l : s) {
    for (const auto &c : l)
      s32.push_back(ToRawChar(c));

    s32.push_back('\n');
  }

  s32.pop_back();
  return s32;
}

} // namespace Lin
