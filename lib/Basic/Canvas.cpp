#include "Basic/Canvas.h"
#include "Basic/Block.h"
#include <QJsonArray>
#include <optional>

namespace Lin {

Char Canvas::getRawChar(QPoint p) {
  if (p.y() <= 0 || p.y() > Height || p.x() <= 0 || p.x() > Width)
    qFatal() << "Accessing the canvas out of bounds";

  const Block *b = getBlock(p);
  if (b == nullptr)
    return 0;

  return b->getRawChar(p);
}

Char Canvas::getChar(QPoint p) {
  if (p.y() <= 0 || p.y() > Height || p.x() <= 0 || p.x() > Width)
    qFatal() << "Accessing the canvas out of bounds";

  const Block *b = getBlock(p);
  if (b == nullptr)
    return 0;

  return b->getChar(p);
}

void Canvas::updateBlockId(quint64 id, bool isDelete) {
  for (int y = 1; y <= Height; ++y)
    for (int x = 1; x <= Width; ++x)
      if (blockId[y][x] == id)
        blockId[y][x] = 0;

  const auto &b = blocks.at(id);
  if (isDelete)
    id = 0;

  for (int y = 0; y < b.getText().size(); ++y)
    for (int x = 0; x < b.getText()[y].size(); ++x)
      blockId[b.getAnchor().y() + y][b.getAnchor().x() + x] = id;
}

void Canvas::updateGroupId() {
  struct Cell {
    int x, y;
    quint64 gid;
  };

  std::deque<Cell> q;
  std::vector<std::vector<bool>> visited(Height + 1,
                                         std::vector<bool>(Width + 1, false));

  for (auto &[id, block] : blocks) {
    quint64 gid = block.getGroupId();
    const auto &text = block.getText();
    QPoint anchor = block.getAnchor();

    for (int r = 0; r < (int)text.size(); r++) {
      for (int c = 0; c < (int)text[r].size(); c++) {
        int x = anchor.x() + c;
        int y = anchor.y() + r;
        if (x >= 0 && x < Width && y >= 0 && y < Height) {
          groupId[y][x] = gid;
          q.push_back({x, y, gid});
          visited[y][x] = true;
        }
      }
    }
  }

  int dx[4] = {1, -1, 0, 0};
  int dy[4] = {0, 0, 1, -1};

  while (!q.empty()) {
    auto cur = q.front();
    q.pop_front();

    for (int k = 0; k < 4; k++) {
      int nx = cur.x + dx[k];
      int ny = cur.y + dy[k];
      if (nx <= 0 || nx > Width || ny <= 0 || ny > Height)
        continue;

      if (!visited[ny][nx]) {
        groupId[ny][nx] = cur.gid;
        visited[ny][nx] = true;
        q.push_back({nx, ny, cur.gid});
      }
    }
  }
}

const Block *Canvas::getBlock(QPoint p) const {
  if (p.y() <= 0 || p.y() > Height || p.x() <= 0 || p.x() > Width)
    qFatal() << "Accessing the canvas out of bounds";

  quint64 id = blockId[p.y()][p.x()];
  auto it = blocks.find(id);
  if (it == blocks.end())
    return nullptr;

  return &it->second;
}

const Block *Canvas::getBlock(quint64 id) const {
  auto it = blocks.find(id);
  if (it == blocks.end())
    return nullptr;

  return &it->second;
}

quint64 Canvas::getBlockId(QPoint p) const {
  if (p.y() <= 0 || p.y() > Height || p.x() <= 0 || p.x() > Width)
    qFatal() << "Accessing the canvas out of bounds";

  return blockId[p.y()][p.x()];
}

quint64 Canvas::getGroupId(QPoint p) const {
  if (p.y() <= 0 || p.y() > Height || p.x() <= 0 || p.x() > Width)
    qFatal() << "Accessing the canvas out of bounds";

  return groupId[p.y()][p.x()];
}

bool Canvas::canPlace(const Block &b) {
  const auto &text = b.getText();
  QPoint anchor = b.getAnchor();

  for (int row = 0; row < static_cast<int>(text.size()); ++row) {
    const auto &line = text[row];
    for (int col = 0; col < static_cast<int>(line.size()); ++col) {
      const Char &ch = line[col];

      int x = anchor.x() + col;
      int y = anchor.y() + row;

      if (x <= 0 || x > Width || y <= 0 || y > Height)
        return false;

      quint64 existingId = blockId[y][x];
      if (existingId != 0 && existingId != b.getId())
        return false;
    }
  }

  return true;
}

void Canvas::updateBlock(quint64 id, Block newBlock, bool recordChange) {
  if (newBlock.isEmpty()) {
    deleteBlock(id, recordChange);
    return;
  }

  if (newBlock.getId() != id)
    qFatal() << "Can't update block, because different id";

  const Block *existing = getBlock(id);
  if (!existing) {
    if (recordChange)
      qWarning() << "Cannot update non-existent block with id:" << id;
    return;
  }

  if (recordChange)
    pushOperation({
        .op = Change::Update,
        .oldBlock = std::optional<Block>(*existing),
        .newBlock = newBlock,
    });

  blocks.insert_or_assign(id, newBlock);
  updateBlockId(id);
  updateGroupId();
}

void Canvas::addBlock(Block b, bool recordChange) {
  if (b.isEmpty())
    return;

  if (recordChange)
    pushOperation({
        .op = Change::Add,
        .oldBlock = std::nullopt,
        .newBlock = b,
    });

  blocks.emplace(b.getId(), b);
  updateBlockId(b.getId());
  updateGroupId();
}

void Canvas::deleteBlock(quint64 id, bool recordChange) {
  const Block *existing = getBlock(id);
  if (!existing) {
    if (recordChange)
      qWarning() << "Cannot delete non-existent block with id:" << id;
    return;
  }

  if (recordChange)
    pushOperation({
        .op = Change::Delete,
        .oldBlock = std::optional<Block>(*existing),
        .newBlock = std::nullopt,
    });

  updateBlockId(id, true);
  auto it = blocks.find(id);
  if (it != blocks.end())
    blocks.erase(it);
  updateGroupId();
}

void Canvas::moveCursorToPos(QPoint p) {
  cursorPos.setX(std::clamp(p.x(), 1, Canvas::Width));
  cursorPos.setY(std::clamp(p.y(), 1, Canvas::Height));
}

void Canvas::moveCursorByOffSet(int offsetX, int offsetY) {
  cursorPos.setX(std::clamp(cursorPos.x() + offsetX, 1, Canvas::Width));
  cursorPos.setY(std::clamp(cursorPos.y() + offsetY, 1, Canvas::Height));
}

void Canvas::reset() {
  cursorPos = {Width / 2, Height / 2};
  blocks.clear();

  for (int y = 1; y <= Height; ++y)
    for (int x = 1; x <= Width; ++x)
      blockId[y][x] = 0;

  for (int i = 1; i <= Height; i++) {
    for (int j = 1; j <= Width; j++) {
      groupId[i][j] = 0;
    }
  }
}

void Canvas::pushOperation(const Change &ch) {
  history.erase(history.begin() + (index + 1), history.end());
  history.push_back(ch);

  if (history.size() > HistoryMaxSize)
    history.pop_front();

  index = history.size() - 1;
}

void Canvas::pushBatchOperation(const Change &batch) {
  pushOperation(batch);
  updateGroupId();
}

void Canvas::undo() {
  if (index < 0)
    qFatal() << "No operation can be undo";

  applyChangeBackward(history[index--]);
}

void Canvas::redo() {
  if (index >= static_cast<int>(history.size()) - 1)
    qFatal() << "No operation can be redo";

  applyChangeForward(history[++index]);
}

void Canvas::applyChangeForward(const Change &ch) {
  switch (ch.op) {
  case Change::Add:
    if (ch.newBlock.has_value())
      addBlock(ch.newBlock.value(), false);
    break;

  case Change::Update:
    if (ch.oldBlock.has_value() && ch.newBlock.has_value())
      updateBlock(ch.oldBlock.value().getId(), ch.newBlock.value(), false);
    break;

  case Change::Delete:
    if (ch.oldBlock.has_value())
      deleteBlock(ch.oldBlock.value().getId(), false);
    break;

  case Change::Batch:
    for (const auto &change : ch.changes)
      applyChangeForward(change);
    break;
  }
}

void Canvas::applyChangeBackward(const Change &ch) {
  switch (ch.op) {
  case Change::Add:
    if (ch.newBlock.has_value())
      deleteBlock(ch.newBlock.value().getId(), false);
    break;

  case Change::Update:
    if (ch.oldBlock.has_value() && ch.newBlock.has_value())
      updateBlock(ch.newBlock.value().getId(), ch.oldBlock.value(), false);
    break;

  case Change::Delete:
    if (ch.oldBlock.has_value())
      addBlock(ch.oldBlock.value(), false);
    break;

  case Change::Batch:
    for (int i = static_cast<int>(ch.changes.size()) - 1; i >= 0; --i)
      applyChangeBackward(ch.changes[i]);
    break;
  }
}

QString Encode(const Canvas &c) {
  QJsonObject root;

  QJsonObject cursor;
  cursor["x"] = c.cursorPos.x();
  cursor["y"] = c.cursorPos.y();
  root["cursor"] = cursor;

  QJsonArray blocksArray;
  for (auto &[_, b] : c.blocks)
    blocksArray.append(Encode(b));

  root["blocks"] = blocksArray;

  QJsonArray rows;
  for (const auto &rowVec : c.blockId) {
    QJsonArray row;
    for (quint64 val : rowVec)
      row.append(static_cast<qint64>(val));
    rows.append(row);
  }

  root["blockId"] = rows;

  QJsonArray opsArray;
  for (const auto &ch : c.history)
    opsArray.append(EncodeChange(ch));

  root["operations"] = opsArray;
  root["index"] = c.index;

  QJsonDocument doc(root);
  return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

std::optional<Canvas> Decode(const QString &s) {
  QByteArray data = s.toUtf8();
  if (data.isEmpty())
    return std::nullopt;

  QJsonDocument doc = QJsonDocument::fromJson(data);
  if (!doc.isObject())
    return std::nullopt;

  QJsonObject root = doc.object();
  Canvas canvas(s);

  QJsonObject cursorObj = root["cursor"].toObject();
  canvas.cursorPos = QPoint(cursorObj["x"].toInt(), cursorObj["y"].toInt());

  QJsonArray blocksArray = root["blocks"].toArray();
  for (const auto &val : blocksArray) {
    Block b = Decode(val.toObject());
    canvas.blocks.emplace(b.getId(), b);
  }

  QJsonArray rows = root["blockId"].toArray();
  for (int i = 1; i < rows.size() && i <= Canvas::Height; ++i) {
    QJsonArray rowArr = rows[i].toArray();
    for (int j = 1; j < rowArr.size() && j <= Canvas::Width; ++j)
      canvas.blockId[i][j] =
          static_cast<quint64>(rowArr[j].toVariant().toLongLong());
  }

  QJsonArray opsArray = root["operations"].toArray();
  for (const auto &val : opsArray) {
    Change ch = DecodeChange(val.toObject());
    canvas.history.push_back(ch);
  }

  int idx = root["index"].toInt();
  canvas.index =
      std::clamp(idx, -1, static_cast<int>(canvas.history.size() - 1));

  canvas.updateGroupId();
  return canvas;
}

} // namespace Lin
