#include "Edit/Core.h"
#include <algorithm>

namespace Lin::Edit {

void Core::enterVisualBlock() {
  Canvas &c = fileManager.getCurrentCanvas();
  const Block *b = c.getBlock(c.getCursorPos());
  if (!b)
    return;

  visualContext.c = &c;
  visualContext.r = std::make_pair(c.getCursorPos(), c.getCursorPos());
  visualContext.anchor = c.getCursorPos();
  visualContext.visualRectangle = false;
  setMode(Mode::Visual);
}

void Core::enterVisualGroup() {
  Canvas &c = fileManager.getCurrentCanvas();
  const Block *current = c.getBlock(c.getCursorPos());
  if (!current)
    return;

  const quint64 groupId = current->getGroupId();
  std::vector<quint64> ids;
  for (const auto &[id, block] : c.getBlocks()) {
    if (block.getGroupId() == groupId)
      ids.push_back(id);
  }
  if (ids.empty())
    return;

  visualContext.c = &c;
  visualContext.r = std::move(ids);
  visualContext.anchor = c.getCursorPos();
  visualContext.visualRectangle = false;
  setMode(Mode::Visual);
}

void Core::enterVisualRectangle() {
  Canvas &c = fileManager.getCurrentCanvas();
  visualContext.c = &c;
  visualContext.anchor = c.getCursorPos();
  visualContext.r = std::make_pair(visualContext.anchor, visualContext.anchor);
  visualContext.visualRectangle = true;
  setMode(Mode::Visual);
}

void Core::exitVisual() {
  visualContext.c = nullptr;
  visualContext.r = quint64{0};
  visualContext.anchor = {};
  visualContext.visualRectangle = false;
  visualContext.moveOriginal.clear();
  setMode(Mode::Normal);
}

void Core::visualCopy() {
  if (mode != Mode::Visual)
    return;
  copy(visualContext.r);
  exitVisual();
}

void Core::visualDelete() {
  if (mode != Mode::Visual || !visualContext.c)
    return;

  std::visit(
      [&](const auto &selection) {
        using T = std::decay_t<decltype(selection)>;
        if constexpr (std::is_same_v<T, quint64>) {
          if (visualContext.c->getBlock(selection))
            visualContext.c->deleteBlock(selection);
        } else if constexpr (std::is_same_v<T, std::vector<quint64>>) {
          for (quint64 id : selection)
            if (visualContext.c->getBlock(id))
              visualContext.c->deleteBlock(id);
        } else {
          const QPoint start = selection.first;
          const QPoint end = selection.second;
          const Block *source = visualContext.c->getBlock(start);
          if (!source || visualContext.c->getBlock(end) != source)
            return;

          const auto &text = source->getText();
          const QPoint anchor = source->getAnchor();
          std::vector<QPoint> cells;

          if (source->getDirection() == Left2Right) {
            for (int row = 0; row < static_cast<int>(text.size()); ++row)
              for (int col = 0;
                   col < static_cast<int>(text[row].size()); ++col)
                cells.push_back({anchor.x() + col, anchor.y() + row});
          } else {
            int width = 0;
            for (const auto &line : text)
              width = std::max(width, static_cast<int>(line.size()));
            for (int col = 0; col < width; ++col)
              for (int row = 0; row < static_cast<int>(text.size()); ++row)
                if (col < static_cast<int>(text[row].size()))
                  cells.push_back({anchor.x() + col, anchor.y() + row});
          }

          auto findCell = [&](QPoint point) {
            return std::find(cells.begin(), cells.end(), point);
          };
          const auto startIt = findCell(start);
          const auto endIt = findCell(end);
          if (startIt == cells.end() || endIt == cells.end())
            return;

          int first = static_cast<int>(startIt - cells.begin());
          int last = static_cast<int>(endIt - cells.begin());
          if (first > last)
            std::swap(first, last);

          Block edited = *source;
          std::vector<QPoint> positions;
          for (int i = first; i <= last; ++i) {
            const QPoint position = cells[i];
            if (!IsPlaceHolder(source->getRawChar(position)))
              positions.push_back(position);
          }

          std::sort(positions.begin(), positions.end(),
                    [source](QPoint a, QPoint b) {
                      if (source->getDirection() == Left2Right)
                        return a.y() == b.y() ? a.x() < b.x()
                                              : a.y() < b.y();
                      return a.x() == b.x() ? a.y() < b.y() : a.x() < b.x();
                    });

          if (!positions.empty()) {
            QPoint position = positions.back();
            for (int i = 0; i < static_cast<int>(positions.size()); ++i) {
              if (!edited.onBlock(position))
                break;
              if (IsPlaceHolder(edited.getRawChar(position))) {
                position.rx()--;
                if (!edited.onBlock(position))
                  break;
              }
              const EditResult result = edited.deleteChar(position, Backward);
              position = result.nextCursorPos;
            }
          }

          if (edited.isEmpty())
            visualContext.c->deleteBlock(source->getId());
          else
            visualContext.c->updateBlock(source->getId(), edited);
        }
      },
      visualContext.r);

  exitVisual();
}

} // namespace Lin::Edit
