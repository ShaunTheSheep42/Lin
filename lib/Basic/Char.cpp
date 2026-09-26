module;

#include <cstdint>

module Basic.Char;

namespace Lin {

// The highest bit is used as a surrogate pair flag
constexpr uint32_t FLAG_SURROGATE = 1u << 31;
constexpr uint32_t FLAG_FULLWIDTH = 1u << 30;
constexpr uint32_t MASK_CODEPOINT = 0x1FFFFF; // 21-bit mask

template <typename T> Char MakeChar(T a) {
  Char c = static_cast<char32_t>(a);
  if (IsChinese(a) || IsIcon(a))
    c |= FLAG_FULLWIDTH;

  return c;
}

template <typename T> Char MakeChar(T a, T b) {
  uint32_t high = static_cast<uint32_t>(a) - 0xD800;
  uint32_t low = static_cast<uint32_t>(b) - 0xDC00;
  uint32_t codepoint = (high << 10) + low + 0x10000;

  Char c = codepoint | FLAG_SURROGATE;
  // HACK: A rough assessment is made here
  c |= FLAG_FULLWIDTH;

  return c;
}

char32_t ToRawChar(Char c) { return c & MASK_CODEPOINT; }

bool IsSurrogatePair(Char c) { return (c & FLAG_SURROGATE) != 0; }
bool IsDoubleWidth(Char c) { return (c & FLAG_FULLWIDTH) != 0; }

bool IsChinese(char32_t c) {
  // Basic Chinese characters
  if (c >= 0x4E00 && c <= 0x9FFF)
    return true;

  // Extension Area A
  if (c >= 0x3400 && c <= 0x4DBF)
    return true;

  // Extension Area B and above
  if (c >= 0x20000 && c <= 0x2FA1F)
    return true;

  return false;
}

bool IsEnglish(char32_t c) {
  return (c >= U'A' && c <= U'Z') || (c >= U'a' && c <= U'z');
}

bool IsPunctuation(char32_t c) {
  if ((c >= 0x21 && c <= 0x2F) || // !"#$%&'()*+,-./
      (c >= 0x3A && c <= 0x40) || // :;<=>?@
      (c >= 0x5B && c <= 0x60) || // [\]^_`
      (c >= 0x7B && c <= 0x7E))   // {|}~
    return true;

  // Fullwidth punctuation U+3000 ~ U+303F
  if (c >= 0x3000 && c <= 0x303F)
    return true;

  return false;
}

bool IsEmoji(char32_t c) {
  if (c >= 0x1F600 && c <= 0x1F64F)
    return true;

  if ((c >= 0x1F300 && c <= 0x1F5FF) || (c >= 0x1F680 && c <= 0x1F6FF) ||
      (c >= 0x1F900 && c <= 0x1F9FF))
    return true;

  return false;
}

bool IsNerdFontIcon(char32_t c) {
  // NerdFont uses Private Use Area (PUA)
  if ((c >= 0xE000 && c <= 0xF8FF) ||   // BMP PUA
      (c >= 0xF0000 && c <= 0xFFFFD) || // Plane 15
      (c >= 0x100000 && c <= 0x10FFFD)) // Plane 16
    return true;

  return false;
}

// Only emoji or NerdFont symbols are considered double-width icons
bool IsIcon(char32_t c) { return IsEmoji(c) || IsNerdFontIcon(c); }

} // namespace Lin
