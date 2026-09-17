#pragma once

#include "Basic/Char.h"
#include "Basic/Cursor.h"
#include "Basic/Direction.h"
#include <QJsonObject>
#include <QPoint>
#include <QString>
#include <qpoint.h>
#include <vector>

namespace Lin {

class Block;
using Blocks = std::vector<Block>;

struct EditResult {
  QPoint nextCursorPos;
  EditPosition ep;
};

class Block {
public:
  Block(QPoint anchor, quint64 groupId, std::vector<std::vector<Char>> text,
        Direction direction = Left2Right)
      : id(CreateOnlyID()), anchor(anchor), groupId(groupId), text(text),
        direction(direction) {}

  Block(QPoint anchor, std::vector<std::vector<Char>> text,
        Direction direction = Left2Right)
      : id(CreateOnlyID()), anchor(anchor), groupId(CreateOnlyID()), text(text),
        direction(direction) {}

  // Getter
  quint64 getId() const { return id; }
  quint64 getGroupId() const { return groupId; }
  QPoint getAnchor() const { return anchor; }
  Direction getDirection() const { return direction; }
  Char getRawChar(QPoint p) const;
  Char getChar(QPoint p) const;
  const std::vector<std::vector<Char>> &getText() const { return text; }
  QString getSemantics() const { return semantics; }
  QString toString() const;
  bool isEmpty() const { return text.size() == 0; }
  bool onBlock(QPoint p) const;

  // Setter
  void setSemantics(QString s) { semantics = s; }
  void moveBy(int offsetX, int offsetY) { anchor += QPoint(offsetX, offsetY); }
  void updateChar(QPoint p, Char ch);
  EditResult insertChar(QPoint p, Char ch, EditPosition ep = Forward);
  EditResult deleteChar(QPoint p, EditPosition ep = Backward);

private:
  Block() = default; // Used by Decode()
  quint64 CreateOnlyID();
  EditResult insertLeft2Right(QPoint p, Char ch, EditPosition ep);
  EditResult insertUp2Down(QPoint p, Char ch, EditPosition ep);
  EditResult deleteLeft2Right(QPoint p, EditPosition ep);
  EditResult deleteUp2Down(QPoint p, EditPosition ep);
  friend QJsonObject Encode(const Block &b);
  friend Block Decode(const QJsonObject &obj);

  quint64 id;
  quint64 groupId;
  QPoint anchor;
  Direction direction;
  Chars text;
  QString semantics; // python code, block's sematic

  static quint64 lastID;
};

QJsonObject Encode(const Block &b);
Block Decode(const QJsonObject &obj);

} // namespace Lin
