#pragma once

#include "Basic/Block.h"
#include "Basic/Change.h"
#include <QString>
#include <deque>
#include <unordered_map>

namespace Lin {

class Canvas {
public:
  Canvas(const QString &path) : path(path), index(-1) { reset(); }
  Canvas(Canvas &&c) noexcept = default;
  Canvas &operator=(Canvas &&) noexcept = default;

  QString getPath() const { return path; }
  void setPath(QString newPath) { path = std::move(newPath); }

  void reset();

  Char getRawChar(QPoint p);
  Char getChar(QPoint p);
  quint64 getBlockId(QPoint p) const;
  quint64 getGroupId(QPoint p) const;

  // Block
  bool haveBlocks() const { return blocks.size() > 0; }
  const Block *getBlock(QPoint p) const;
  const Block *getBlock(quint64 id) const;
  const std::unordered_map<quint64, Block> &getBlocks() const { return blocks; }

  bool canPlace(const Block &b);
  void updateBlock(quint64 id, Block newBlock, bool recordChange = true);
  void addBlock(Block b, bool recordChange = true);
  void deleteBlock(quint64 id, bool recordChange = true);

  // Cursor
  QPoint getCursorPos() const { return cursorPos; }
  void moveCursorToPos(QPoint p);
  void moveCursorByOffSet(int offsetX, int offsetY);

  // Undo/Redo
  bool canUndo() const { return index >= 0; }
  bool canRedo() const { return index < static_cast<int>(history.size()) - 1; }
  void undo();
  void redo();
  void pushBatchOperation(const Change &batch);

  // When font size 16
  constexpr static int Height = 8 * 4 + 6;
  constexpr static int Width = 8 * 15 + 8;

private:
  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;

  void updateBlockId(quint64 id, bool isDelete = false);
  void updateGroupId();
  void pushOperation(const Change &changes);
  void applyChangeForward(const Change &ch);
  void applyChangeBackward(const Change &ch);

  friend QString Encode(const Canvas &c);
  friend std::optional<Canvas> Decode(const QString &s);

  QString path;
  QPoint cursorPos; // cursor position
  std::unordered_map<quint64, Block> blocks;

  // Save the ID of the block to which each cell belongs
  // 1-Base
  quint64 blockId[Height + 1][Width + 1];
  quint64 groupId[Height + 1][Width + 1];

  // 0-Base
  std::deque<Change> history;
  int index; // last applied change index
  constexpr static int HistoryMaxSize = 1000;
};

QString Encode(const Canvas &c);
std::optional<Canvas> Decode(const QString &s);

} // namespace Lin
