#include "Edit/Recognize.h"
#include "Basic/Char.h"
#include <algorithm>
#include <numeric>

namespace Lin::Edit {

bool SimpleRecognizer::isSeparator(Char ch) const {
  const char32_t codepoint = GetChar(ch);
  return codepoint == U'\n' || QChar::isSpace(codepoint) ||
         QStringView(u",.;:!?，。！？；：")
             .contains(QChar::fromUcs4(codepoint));
}

std::vector<Token> SimpleRecognizer::tokenize(const Block &block) const {
  std::vector<std::pair<QPoint, Char>> cells;
  const auto &text = block.getText();
  const QPoint anchor = block.getAnchor();

  if (block.getDirection() == Left2Right) {
    for (int row = 0; row < static_cast<int>(text.size()); ++row) {
      for (int col = 0; col < static_cast<int>(text[row].size()); ++col) {
        Char ch = text[row][col];
        if (!IsPlaceHolder(ch))
          cells.push_back({{anchor.x() + col, anchor.y() + row}, ch});
      }
      // Block rows are implicit token boundaries. A Block does not store a
      // newline character, but moving to the next row starts a new token.
      cells.push_back({{}, U'\n'});
    }
  } else {
    const int width = std::max(
        0, std::accumulate(
               text.begin(), text.end(), 0, [](int size, const auto &line) {
                 return std::max(size, static_cast<int>(line.size()));
               }));
    for (int col = 0; col < width; ++col) {
      for (int row = 0; row < static_cast<int>(text.size()); ++row) {
        if (col >= static_cast<int>(text[row].size()))
          continue;
        Char ch = text[row][col];
        if (!IsPlaceHolder(ch))
          cells.push_back({{anchor.x() + col, anchor.y() + row}, ch});
      }
      // For vertical Blocks, each logical column is a separate line.
      cells.push_back({{}, U'\n'});
    }
  }

  std::vector<Token> tokens;
  Token current;
  for (const auto &[position, ch] : cells) {
    if (isSeparator(ch)) {
      if (!current.positions.empty()) {
        tokens.push_back(std::move(current));
        current = {};
      }
      continue;
    }

    current.text += CharToString(ch);
    current.positions.push_back(position);
  }

  if (!current.positions.empty())
    tokens.push_back(std::move(current));

  return tokens;
}

} // namespace Lin::Edit
