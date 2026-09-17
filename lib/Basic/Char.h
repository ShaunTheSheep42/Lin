#pragma once

#include <QString>

namespace Lin {

using Char = char32_t;
using Chars = std::vector<std::vector<Char>>;

constexpr uint32_t FLAG_PLACEHOLDER = 1u << 29;
constexpr char32_t PlaceHolder = U'\0' | FLAG_PLACEHOLDER;

Char MakeChar(char16_t a, bool doubleWidth = false);
Char MakeChar(char16_t a, char16_t b, bool doubleWidth = false);

char32_t GetChar(Char c);
bool IsSurrogatePair(Char c);
bool IsDoubleWidth(Char c);
bool IsPlaceHolder(Char c);
bool IsEmptyChar(Char c);
QString CharToString(Char c);
QString CharsToString(const Chars &c);
Chars StringToChars(const QString &s, bool noPlaceholder = false);

bool IsChinese(const QString &ch);
bool IsEnglish(const QString &ch);
bool IsPunctuation(const QString &ch);
bool IsEmoji(const QString &ch);
bool IsNerdFont(const QString &ch);
bool IsIcon(const QString &ch);

} // namespace Lin
