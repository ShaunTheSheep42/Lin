#include "Basic/Canvas.h"
#include "Basic/Char.h"
#include "Edit/Core.h"
#include "Edit/Recognize.h"
#include <algorithm>

namespace Lin::Edit {

namespace {

bool isMotionPunctuation(Char ch) {
  const char32_t codepoint = GetChar(ch);
  return QStringView(u",.;:!?，。！？；：")
      .contains(QChar::fromUcs4(codepoint));
}

std::vector<Token> motionTokens(const Block &block) {
  std::vector<Token> tokens;
  const auto &text = block.getText();
  const QPoint anchor = block.getAnchor();
  const bool horizontal = block.getDirection() == Left2Right;

  auto addLine = [&](const std::vector<Char> &line, int lineIndex) {
    Token current;
    for (int index = 0; index < static_cast<int>(line.size()); ++index) {
      const Char ch = line[index];
      if (IsPlaceHolder(ch))
        continue;

      const bool whitespace = QChar::isSpace(GetChar(ch));
      const bool punctuation = isMotionPunctuation(ch);
      if (whitespace) {
        if (!current.positions.empty()) {
          tokens.push_back(std::move(current));
          current = {};
        }
        continue;
      }

      QPoint position;
      if (horizontal)
        position = {anchor.x() + index, anchor.y() + lineIndex};
      else
        position = {anchor.x() + lineIndex, anchor.y() + index};

      if (punctuation) {
        if (!current.positions.empty()) {
          tokens.push_back(std::move(current));
          current = {};
        }
        Token punctuationToken;
        punctuationToken.text = CharToString(ch);
        punctuationToken.positions.push_back(position);
        tokens.push_back(std::move(punctuationToken));
        continue;
      }

      current.text += CharToString(ch);
      current.positions.push_back(position);
    }
    if (!current.positions.empty())
      tokens.push_back(std::move(current));
  };

  if (horizontal) {
    for (int row = 0; row < static_cast<int>(text.size()); ++row)
      addLine(text[row], row);
  } else {
    int width = 0;
    for (const auto &line : text)
      width = std::max(width, static_cast<int>(line.size()));
    for (int col = 0; col < width; ++col) {
      std::vector<Char> line;
      for (const auto &row : text)
        if (col < static_cast<int>(row.size()))
          line.push_back(row[col]);
      addLine(line, col);
    }
  }
  return tokens;
}

int findMotionToken(const std::vector<Token> &tokens, QPoint cursor,
                    const Block &block) {
  for (int i = 0; i < static_cast<int>(tokens.size()); ++i)
    if (std::find(tokens[i].positions.begin(), tokens[i].positions.end(),
                  cursor) != tokens[i].positions.end())
      return i;

  for (int i = 0; i < static_cast<int>(tokens.size()); ++i) {
    const QPoint start = tokens[i].startPos();
    if ((block.getDirection() == Left2Right && start.y() == cursor.y() &&
         start.x() > cursor.x()) ||
        (block.getDirection() == Up2Down && start.x() == cursor.x() &&
         start.y() > cursor.y()))
      return i;
  }
  return -1;
}

} // namespace

void Core::mirrorMove(bool isVertical) {
  auto &canvas = fileManager.getCurrentCanvas();
  QPoint cursor = canvas.getCursorPos();
  QPoint mirrored = cursor;

  if (isVertical)
    mirrored.setY(Canvas::Height - cursor.y());
  else
    mirrored.setX(Canvas::Width - cursor.x());

  canvas.moveCursorToPos(mirrored);
}

void Core::moveCursorToNextChar() {
  auto &canvas = fileManager.getCurrentCanvas();
  auto cursor = canvas.getCursorPos();
  const Block *block = canvas.getBlock(cursor);
  if (!block) {
    canvas.moveCursorByOffSet(1, 0);
    return;
  }

  Char c = block->getRawChar(cursor);
  canvas.moveCursorByOffSet(IsDoubleWidth(c) ? 2 : 1, 0);
}

void Core::moveCursorToPrevChar() {
  auto &canvas = fileManager.getCurrentCanvas();
  auto cursor = canvas.getCursorPos();
  const Block *block = canvas.getBlock(cursor);
  if (!block) {
    canvas.moveCursorByOffSet(-1, 0);
    return;
  }

  Char c = canvas.getRawChar(cursor);
  canvas.moveCursorByOffSet(IsPlaceHolder(c) ? -2 : -1, 0);
}

void Core::moveCursorByOffSet(int offsetX, int offsetY) {
  fileManager.getCurrentCanvas().moveCursorByOffSet(offsetX, offsetY);
}

void Core::moveCursorToPos(QPoint p) {
  fileManager.getCurrentCanvas().moveCursorToPos(p);
}

void Core::moveCursorWord(int direction) {
  Canvas &canvas = fileManager.getCurrentCanvas();
  const Block *block = canvas.getBlock(canvas.getCursorPos());
  if (!block)
    return;

  const auto tokens = motionTokens(*block);
  if (tokens.empty())
    return;

  const QPoint cursor = canvas.getCursorPos();
  const int current = findMotionToken(tokens, cursor, *block);

  if (direction > 0) {
    const int target = current < 0 ? 0 : current + 1;
    if (target < static_cast<int>(tokens.size()))
      canvas.moveCursorToPos(tokens[target].startPos());
    else if (current >= 0)
      canvas.moveCursorToPos(tokens[current].endPos());
  } else if (direction == 0) {
    if (current < 0)
      return;

    if (cursor == tokens[current].endPos() &&
        current + 1 < static_cast<int>(tokens.size()))
      canvas.moveCursorToPos(tokens[current + 1].endPos());
    else
      canvas.moveCursorToPos(tokens[current].endPos());
  } else if (current >= 0) {
    if (cursor == tokens[current].startPos() && current > 0)
      canvas.moveCursorToPos(tokens[current - 1].startPos());
    else
      canvas.moveCursorToPos(tokens[current].startPos());
  } else if (current < 0) {
    canvas.moveCursorToPos(tokens.front().startPos());
  }
}

void Core::moveVisualCursor(int offsetX, int offsetY) {
  if (!visualContext.c ||
      !std::holds_alternative<std::pair<QPoint, QPoint>>(visualContext.r))
    return;

  QPoint target = visualContext.c->getCursorPos();
  target += QPoint(offsetX, offsetY);

  if (visualContext.visualRectangle) {
    visualContext.c->moveCursorToPos(target);
  } else {
    const Block *block = visualContext.c->getBlock(visualContext.anchor);
    if (!block)
      return;

    const auto &text = block->getText();
    if (text.empty())
      return;

    const int minY = block->getAnchor().y();
    const int maxY = minY + static_cast<int>(text.size()) - 1;
    target.setY(std::clamp(target.y(), minY, maxY));

    const auto &line = text[target.y() - minY];
    if (line.empty())
      target.setX(block->getAnchor().x());
    else
      target.setX(std::clamp(
          target.x(), block->getAnchor().x(),
          block->getAnchor().x() + static_cast<int>(line.size()) - 1));

    visualContext.c->moveCursorToPos(target);
  }

  visualContext.r =
      std::make_pair(visualContext.anchor, visualContext.c->getCursorPos());
}

void Core::moveVisualWord(int direction) {
  if (mode != Mode::Visual || visualContext.visualRectangle ||
      !visualContext.c)
    return;

  const Block *block = visualContext.c->getBlock(visualContext.anchor);
  if (!block)
    return;

  QPoint cursor = visualContext.c->getCursorPos();
  const auto tokens = motionTokens(*block);
  if (tokens.empty())
    return;

  const int current = findMotionToken(tokens, cursor, *block);

  if (current < 0)
    return;

  int target = current;
  const bool atBlockEnd =
      direction > 0 && current + 1 >= static_cast<int>(tokens.size());
  if (direction > 0) {
    if (!atBlockEnd)
      target = current + 1;
  } else if (direction < 0 && cursor == tokens[current].startPos()) {
    target = current - 1;
  } else if (direction == 0 && cursor == tokens[current].endPos()) {
    target = current + 1;
  }

  if (atBlockEnd) {
    visualContext.c->moveCursorToPos(tokens[current].endPos());
    visualContext.r =
        std::make_pair(visualContext.anchor, visualContext.c->getCursorPos());
    return;
  }

  if (target < 0 || target >= static_cast<int>(tokens.size()))
    return;

  QPoint targetPos =
      direction == 0 ? tokens[target].endPos() : tokens[target].startPos();
  visualContext.c->moveCursorToPos(targetPos);
  visualContext.r =
      std::make_pair(visualContext.anchor, visualContext.c->getCursorPos());
}

} // namespace Lin::Edit
