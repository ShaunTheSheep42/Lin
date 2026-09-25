module;

#include <string>
#include <vector>

export module Basic.String;
import Basic.Char;

export namespace lin {

using Char = char32_t;
// NOTE: With "PlaceHolder"
using String = std::vector<Char>;

template <typename T>
  requires std::same_as<std::remove_cvref_t<T>, Char>
String MakeString(T c);
String MakeString(const std::u32string &s32);

std::u32string ToStdU32String(const String &c);

} // namespace lin
