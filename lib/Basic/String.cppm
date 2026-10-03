module;

#include <concepts>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

export module Basic.String;
import Basic.Char;

export namespace Lin {

using String = std::vector<std::vector<Char>>;

template <typename T>
  requires std::same_as<std::remove_cvref_t<T>, Char>
String MakeString(T c) {
  String result;
  std::vector<Char> line;
  if (ToRawChar(c) == U'\t') {
    result.emplace_back(WhiteSpace);
    return result;
  }

  if (ToRawChar(c) == U'\n') {
    result.emplace_back();
    result.emplace_back();
    return result;
  }

  line.push_back(c);
  if (IsFullWidth(c))
    line.push_back(PlaceHolder);
  result.push_back(std::move(line));
  return result;
}

inline String MakeString(const std::u32string &s32) {
  if (s32.empty())
    return {};

  String result;
  std::vector<Char> line;

  for (char32_t codePoint : s32) {
    Char c = MakeChar(codePoint);
    if (ToRawChar(c) == U'\t') {
      line.push_back(MakeChar(U' '));
      continue;
    }

    if (ToRawChar(c) == U'\n') {
      result.push_back(std::move(line));
      line.clear();
      continue;
    }

    line.push_back(c);
    if (IsFullWidth(c))
      line.push_back(PlaceHolder);
  }

  result.push_back(std::move(line));
  return result;
}

inline std::u32string ToStdU32String(const String &s) {
  if (s.empty())
    return {};

  std::u32string result;
  for (std::size_t row = 0; row < s.size(); ++row) {
    for (Char c : s[row]) {
      if (c == PlaceHolder)
        continue;

      if (c == WhiteSpace) {
        result.push_back(U' ');
        continue;
      }

      result.push_back(ToRawChar(c));
    }

    if (row + 1 < s.size())
      result.push_back(U'\n');
  }

  return result;
}

} // namespace Lin
