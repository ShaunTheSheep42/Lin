#include "Basic/Block.h"
#include "Basic/Char.h"
#include "Edit/Core.h"
#include "Edit/Recognize.h"
#include "Edit/Region.h"
#include <algorithm>
#include <variant>

namespace Lin::Edit {

void Core::copyMotion(QPoint start, QPoint end) {
  Canvas &canvas = fileManager.getCurrentCanvas();
  const Block *source = canvas.getBlock(start);
  if (!source || canvas.getBlock(end) != source)
    return;

  const auto &text = source->getText();
  const QPoint anchor = source->getAnchor();
  Chars copied;
  if (source->getDirection() == Left2Right && start.y() == end.y()) {
    const int row = start.y() - anchor.y();
    int first = std::min(start.x(), end.x()) - anchor.x();
    int last = std::max(start.x(), end.x()) - anchor.x();
    if (row >= 0 && row < static_cast<int>(text.size())) {
      first = std::max(first, 0);
      last = std::min(last, static_cast<int>(text[row].size()) - 1);
      if (first <= last)
        copied.push_back(
            {text[row].begin() + first, text[row].begin() + last + 1});
    }
  } else if (source->getDirection() == Up2Down && start.x() == end.x()) {
    const int col = start.x() - anchor.x();
    const int first = std::max(0, std::min(start.y(), end.y()) - anchor.y());
    const int last = std::max(start.y(), end.y()) - anchor.y();
    if (col >= 0) {
      std::vector<Char> line;
      for (int row = first; row <= last && row < static_cast<int>(text.size());
           ++row) {
        if (col < static_cast<int>(text[row].size()))
          line.push_back(text[row][col]);
      }

      if (!line.empty())
        copied.push_back(std::move(line));
    }
  }

  if (!copied.empty()) {
    registerManager.getRegister(';') = std::move(copied);
    registerManager.syncToSystemClipboard();
  }
}

void Core::deleteMotion(QPoint start, QPoint end, bool includeEnd) {
  Canvas &canvas = fileManager.getCurrentCanvas();
  const Block *source = canvas.getBlock(start);
  if (!source)
    return;
  if (includeEnd && canvas.getBlock(end) != source)
    return;

  Block edited = *source;
  const auto &text = source->getText();
  const QPoint anchor = source->getAnchor();
  std::vector<QPoint> positions;
  bool deleteWholeLine = false;
  bool wordDelete = false;

  if (!includeEnd) {
    SimpleRecognizer recognizer;
    const auto tokens = recognizer.tokenize(*source);
    for (const auto &token : tokens) {
      const auto &tokenPositions = token.positions;
      if (std::find(tokenPositions.begin(), tokenPositions.end(), start) !=
          tokenPositions.end()) {
        wordDelete = true;
        for (const QPoint position : tokenPositions)
          positions.push_back(position);
        break;
      }
    }
  }

  if (wordDelete) {
    // dw deletes the complete token under the cursor. The motion endpoint
    // may be on another line when this is the last token on a line.
  } else if (source->getDirection() == Left2Right && start.y() == end.y()) {
    const int row = start.y() - anchor.y();
    int first = deleteWholeLine ? 0 : std::min(start.x(), end.x()) - anchor.x();
    int last = deleteWholeLine ? static_cast<int>(text[row].size()) - 1
                               : std::max(start.x(), end.x()) - anchor.x();
    if (row < 0 || row >= static_cast<int>(text.size()))
      return;
    if (!includeEnd && end.x() >= start.x()) {
      --last;
    }
    first = std::max(first, 0);
    last = std::min(last, static_cast<int>(text[row].size()) - 1);
    for (int col = first; col <= last; ++col)
      if (!IsPlaceHolder(text[row][col]))
        positions.push_back({anchor.x() + col, anchor.y() + row});
  } else if (source->getDirection() == Up2Down && start.x() == end.x()) {
    const int col = start.x() - anchor.x();
    int first = deleteWholeLine ? 0 : std::min(start.y(), end.y()) - anchor.y();
    int last = deleteWholeLine ? static_cast<int>(text.size()) - 1
                               : std::max(start.y(), end.y()) - anchor.y();
    if (!includeEnd && end.y() >= start.y())
      --last;
    first = std::max(first, 0);
    last = std::min(last, static_cast<int>(text.size()) - 1);
    for (int row = first; row <= last; ++row)
      if (col >= 0 && col < static_cast<int>(text[row].size()) &&
          !IsPlaceHolder(text[row][col]))
        positions.push_back({anchor.x() + col, anchor.y() + row});
  }

  if (positions.empty())
    return;
  std::sort(positions.begin(), positions.end(), [](QPoint a, QPoint b) {
    return a.y() == b.y() ? a.x() > b.x() : a.y() > b.y();
  });
  for (const QPoint position : positions)
    edited.deleteChar(position, Backward);

  if (edited.isEmpty())
    canvas.deleteBlock(source->getId());
  else
    canvas.updateBlock(source->getId(), edited);
  canvas.moveCursorToPos(start);
}

void Core::copy(Region r) {
  auto &c = fileManager.getCurrentCanvas();
  auto &reg = registerManager.getRegister(';');

  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, quint64>) {
          reg = Block(*c.getBlock(arg));
        } else if constexpr (std::is_same_v<T, std::vector<quint64>>) {
          auto blocks = Blocks();
          for (auto id : arg) {
            const Block *b = c.getBlock(id);
            blocks.push_back(*b);
          }

          reg = std::move(blocks);
        } else if constexpr (std::is_same_v<T, std::pair<QPoint, QPoint>>) {
          QPoint start = arg.first;
          QPoint end = arg.second;
          const int left = std::min(start.x(), end.x());
          const int right = std::max(start.x(), end.x());
          const int top = std::min(start.y(), end.y());
          const int bottom = std::max(start.y(), end.y());
          Chars cs(bottom - top + 1);
          for (int y = top; y <= bottom; ++y) {
            for (int x = left; x <= right; ++x) {
              if (c.getChar({x, y})) {
                cs[y - top].push_back(c.getChar({x, y}));
              } else {
                cs[y - top].push_back(MakeChar(' '));
              }
            }
          }
          reg = Chars(std::move(cs));
        }
      },
      r);
  registerManager.syncToSystemClipboard();
}

void Core::paste(bool newGroup) {
  auto &c = fileManager.getCurrentCanvas();
  QPoint p = c.getCursorPos();
  auto gid = c.getGroupId(p);

  auto &reg = registerManager.getRegister(';');

  if (!reg.has_value())
    return;

  std::visit(
      [&](auto &&arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, Block>) {
          if (newGroup) {
            Block b(p, arg.getText());
            if (c.canPlace(b))
              c.addBlock(b);
            return;
          }

          Block b(p, gid, arg.getText());
          c.addBlock(b);
        } else if constexpr (std::is_same_v<T, Blocks>) {
          if (arg.empty())
            return;

          QPoint origin = arg.front().getAnchor();
          for (const auto &item : arg)
            origin = {std::min(origin.x(), item.getAnchor().x()),
                      std::min(origin.y(), item.getAnchor().y())};

          quint64 targetGroup = gid;
          bool groupInitialized = !newGroup;
          Change batch;
          batch.op = Change::Batch;
          for (const auto &item : arg) {
            QPoint offset = item.getAnchor() - origin;
            Block b =
                groupInitialized
                    ? Block(p + offset, targetGroup, item.getText(),
                            item.getDirection())
                    : Block(p + offset, item.getText(), item.getDirection());
            b.setSemantics(item.getSemantics());
            if (!groupInitialized) {
              targetGroup = b.getGroupId();
              groupInitialized = true;
            }
            if (!c.canPlace(b))
              continue;

            Change change;
            change.op = Change::Add;
            change.newBlock = b;
            batch.changes.push_back(std::move(change));
            c.addBlock(b, false);
          }

          if (!batch.changes.empty())
            c.pushBatchOperation(batch);
        } else if constexpr (std::is_same_v<T, Chars>) {
          if (newGroup) {
            Block b(p, arg);
            if (c.canPlace(b))
              c.addBlock(b);

            return;
          }

          Block b(p, gid, arg);
          c.addBlock(b);
        }
      },
      *reg);
}

} // namespace Lin::Edit
