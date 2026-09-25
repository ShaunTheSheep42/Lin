module;

#include <concepts>

export module Basic.Char;

// WARN: Only supports English, Chinese, Emojis, and NerdFonts Icon.

export namespace lin {

using Char = char32_t;

constexpr char32_t PlaceHolder = U'\0';

template <typename T>
  requires std::same_as<std::remove_cvref_t<T>, char16_t> ||
           std::same_as<std::remove_cvref_t<T>, char32_t>
Char MakeChar(T a);

template <typename T>
  requires std::same_as<std::remove_cvref_t<T>, char16_t>
Char MakeChar(T a, T b);

char32_t ToRawChar(Char c);

bool IsSurrogatePair(Char c);
bool IsFullWidth(Char c);

bool IsSurrogatePair(char32_t c);
bool IsChinese(char32_t c);
bool IsEnglish(char32_t c);
bool IsPunctuation(char32_t c);

bool IsIcon(char32_t c);
bool IsEmoji(char32_t c);
bool IsNerdFontIcon(char32_t c);

} // namespace lin
