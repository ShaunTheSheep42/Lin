module;

#include <concepts>
#include <cstdint>

export module Basic.Char;

namespace Lin {

export using Char = char32_t;

// The highest bit is used as a surrogate pair flag
constexpr uint32_t FLAG_SURROGATE = 1u << 31;
constexpr uint32_t FLAG_FULLWIDTH = 1u << 30;
constexpr uint32_t FLAG_PLACEHOLDER = 1u << 29;
constexpr uint32_t FLAG_WHITESPACE = 1u << 28;
constexpr uint32_t MASK_CODEPOINT = 0x1FFFFF; // 21-bit mask
export constexpr char32_t PlaceHolder = U'\0' | FLAG_PLACEHOLDER;
export constexpr char32_t WhiteSpace = U'\0' | FLAG_WHITESPACE;

export char32_t ToRawChar(Char c);

export bool IsSurrogatePair(Char c);
export bool IsFullWidth(Char c);

export bool IsSurrogatePair(char32_t c);
export bool IsChinese(char32_t c);
export bool IsEnglish(char32_t c);
export bool IsPunctuation(char32_t c);

export bool IsIcon(char32_t c);
export bool IsEmoji(char32_t c);
export bool IsNerdFontIcon(char32_t c);

export template <typename T>
  requires std::same_as<std::remove_cvref_t<T>, char16_t> ||
           std::same_as<std::remove_cvref_t<T>, char32_t>
Char MakeChar(T a) {
  Char c = static_cast<char32_t>(a);
  if (IsChinese(a) || IsIcon(a))
    c |= FLAG_FULLWIDTH;

  return c;
}

export template <typename T>
  requires std::same_as<std::remove_cvref_t<T>, char16_t>
Char MakeChar(T a, T b) {
  uint32_t high = static_cast<uint32_t>(a) - 0xD800;
  uint32_t low = static_cast<uint32_t>(b) - 0xDC00;
  uint32_t codepoint = (high << 10) + low + 0x10000;

  Char c = codepoint | FLAG_SURROGATE;
  // HACK: A rough assessment is made here
  c |= FLAG_FULLWIDTH;

  return c;
}

} // namespace Lin
