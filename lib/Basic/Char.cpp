#include "Basic/Char.h"
#include <QStringList>

namespace Lin {

// The highest bit is used as a surrogate pair flag
constexpr uint32_t FLAG_SURROGATE = 1u << 31;
constexpr uint32_t FLAG_DOUBLEWIDTH = 1u << 30;
constexpr uint32_t MASK_CODEPOINT = 0x1FFFFF; // 21-bit mask

Char MakeChar(char16_t a, bool doubleWidth) {
  Char c = static_cast<char32_t>(a);
  if (doubleWidth)
    c |= FLAG_DOUBLEWIDTH;

  return c;
}

Char MakeChar(char16_t a, char16_t b, bool doubleWidth) {
  uint32_t high = static_cast<uint32_t>(a) - 0xD800;
  uint32_t low = static_cast<uint32_t>(b) - 0xDC00;
  uint32_t codepoint = (high << 10) + low + 0x10000;

  Char c = codepoint | FLAG_SURROGATE;
  if (doubleWidth)
    c |= FLAG_DOUBLEWIDTH;

  return c;
}

char32_t GetChar(Char c) { return c & MASK_CODEPOINT; }

bool IsSurrogatePair(Char c) { return (c & FLAG_SURROGATE) != 0; }

bool IsDoubleWidth(Char c) { return (c & FLAG_DOUBLEWIDTH) != 0; }

bool IsPlaceHolder(Char c) { return (c & FLAG_PLACEHOLDER) != 0; }

bool IsEmptyChar(Char c) { return GetChar(c) == 0; }

QString CharToString(Char c) {
  char32_t codepoint = GetChar(c);
  return QString::fromUcs4(&codepoint, 1);
}

QString CharsToString(const Chars &c) {
  QString ss;
  for (auto &y : c) {
    for (auto &x : y)
      if (!IsPlaceHolder(x))
        ss += CharToString(x);

    ss += "\n";
  }

  if (!ss.isEmpty())
    ss.chop(1);
  return ss;
}

Chars StringToChars(const QString &s, bool noPlaceholder) {
  Chars result;
  QStringList lines = s.split('\n');

  for (const QString &line : lines) {
    std::vector<Char> row;

    for (int i = 0; i < line.size(); ++i) {
      QChar qc = line[i];
      char16_t u = qc.unicode();

      Char ch;
      bool doubleWidth = false;

      if (qc.isHighSurrogate() && i + 1 < line.size() &&
          line[i + 1].isLowSurrogate()) {
        QChar low = line[i + 1];
        char16_t uLow = low.unicode();
        QString surrogateStr;
        surrogateStr.append(qc);
        surrogateStr.append(low);

        ch = MakeChar(u, uLow);
        doubleWidth = IsChinese(surrogateStr) || IsEmoji(surrogateStr) ||
                      IsNerdFont(surrogateStr);
        ++i;
      } else {
        QString singleStr(qc);
        ch = MakeChar(u);
        doubleWidth =
            IsChinese(singleStr) || IsEmoji(singleStr) || IsNerdFont(singleStr);
      }

      if (doubleWidth)
        ch |= FLAG_DOUBLEWIDTH;

      row.push_back(ch);

      if (doubleWidth && !noPlaceholder)
        row.push_back(PlaceHolder);
    }

    result.push_back(row);
  }

  return result;
}

bool IsChinese(const QString &s) {
  if (s.isEmpty())
    return false;
  char32_t codepoint = s.at(0).unicode();

  // Basic Chinese characters
  if (codepoint >= 0x4E00 && codepoint <= 0x9FFF)
    return true;

  // Extension Area A
  if (codepoint >= 0x3400 && codepoint <= 0x4DBF)
    return true;

  // Extension Area B and above
  if (codepoint >= 0x20000 && codepoint <= 0x2FA1F)
    return true;

  return false;
}

bool IsEnglish(const QString &s) {
  if (s.isEmpty())
    return false;
  char32_t codepoint = s.at(0).unicode();

  return ((codepoint >= U'A' && codepoint <= U'Z') ||
          (codepoint >= U'a' && codepoint <= U'z'));
}

bool IsPunctuation(const QString &s) {
  if (s.isEmpty())
    return false;
  char32_t codepoint = s.at(0).unicode();

  // ASCII punctuation
  if ((codepoint >= 0x21 && codepoint <= 0x2F) || // !"#$%&'()*+,-./
      (codepoint >= 0x3A && codepoint <= 0x40) || // :;<=>?@
      (codepoint >= 0x5B && codepoint <= 0x60) || // [\]^_`
      (codepoint >= 0x7B && codepoint <= 0x7E))   // {|}~
    return true;

  // Fullwidth punctuation U+3000 ~ U+303F
  if (codepoint >= 0x3000 && codepoint <= 0x303F)
    return true;

  return false;
}

bool IsEmoji(const QString &s) {
  if (s.isEmpty())
    return false;

  char32_t codepoint;
  if (s.size() >= 2 && s.at(0).isHighSurrogate() && s.at(1).isLowSurrogate()) {
    // Combine surrogate pair
    char16_t high = s.at(0).unicode();
    char16_t low = s.at(1).unicode();
    codepoint = ((static_cast<uint32_t>(high) - 0xD800) << 10) +
                (static_cast<uint32_t>(low) - 0xDC00) + 0x10000;
  } else {
    codepoint = s.at(0).unicode();
  }

  // Emoji ranges
  if (codepoint >= 0x1F600 && codepoint <= 0x1F64F)
    return true;
  if ((codepoint >= 0x1F300 && codepoint <= 0x1F5FF) ||
      (codepoint >= 0x1F680 && codepoint <= 0x1F6FF) ||
      (codepoint >= 0x1F900 && codepoint <= 0x1F9FF))
    return true;

  return false;
}

bool IsNerdFont(const QString &s) {
  if (s.isEmpty())
    return false;
  char32_t codepoint = s.at(0).unicode();

  // NerdFont uses Private Use Area (PUA)
  if ((codepoint >= 0xE000 && codepoint <= 0xF8FF) ||   // BMP PUA
      (codepoint >= 0xF0000 && codepoint <= 0xFFFFD) || // Plane 15
      (codepoint >= 0x100000 && codepoint <= 0x10FFFD)) // Plane 16
    return true;

  return false;
}

bool IsIcon(const QString &s) {
  // Only emoji or NerdFont symbols are considered double-width icons
  return IsEmoji(s) || IsNerdFont(s);
}

} // namespace Lin
