#include "Basic/Block.h"
#include "Basic/Char.h"
#include "Basic/Cursor.h"
#include "Basic/Direction.h"
#include <QDateTime>
#include <QJsonArray>
#include <QThread>
#include <qlogging.h>
#include <vector>

namespace Lin {

quint64 Block::lastID = 0;

QString Block::toString() const { return CharsToString(text); }

bool Block::onBlock(QPoint p) const {
  const int y = p.y() - anchor.y();
  const int x = p.x() - anchor.x();

  return y >= 0 && y < static_cast<int>(text.size()) && x >= 0 &&
         x < static_cast<int>(text[y].size());
}

Char Block::getRawChar(QPoint p) const {
  const int y = p.y() - anchor.y();
  const int x = p.x() - anchor.x();
  return text[y][x];
}

Char Block::getChar(QPoint p) const {
  const int y = p.y() - anchor.y();
  const int x = p.x() - anchor.x();
  auto c = text[y][x];
  if (IsPlaceHolder(c))
    return text[y][x - 1];

  return c;
}

void Block::updateChar(QPoint p, Char ch) {
  const int y = p.y() - anchor.y();
  const int x = p.x() - anchor.x();

  if (text.begin() + y >= text.end() || y < 0)
    qFatal() << "Beyond the boundary";
  if (text[y].begin() + x >= text[y].end() || x < 0)
    qFatal() << "Beyond the boundary";

  if (IsPlaceHolder(text[y][x])) {
    p.rx()--;
    updateChar(p, ch);
    return;
  }

  if (IsDoubleWidth(ch)) {
    if (!IsDoubleWidth(text[y][x]))
      text[y].insert(text[y].begin() + x + 1, PlaceHolder);

    text[y][x] = ch;
    return;
  }

  if (GetChar(ch) != U'\n') {
    if (!IsDoubleWidth(text[y][x])) {
      text[y][x] = ch;
      return;
    }

    text[y].erase(text[y].begin() + x + 1);
    text[y][x] = ch;
    return;
  }

  int offset = IsDoubleWidth(text[y][x]) ? x + 2 : x + 1;
  std::vector<Char> newLine(text[y].begin() + offset, text[y].end());
  text.insert(text.begin() + y + 1, newLine);
  if (y == 0)
    anchor.ry()++;

  text[y].erase(text[y].begin() + x, text[y].end());
  if (text[y].size() == 0)
    text.erase(text.begin() + y);
}

EditResult Block::insertLeft2Right(QPoint p, Char ch, EditPosition ep) {
  int y = p.y() - anchor.y();
  int x = p.x() - anchor.x();

  // Insert on empty line
  if (text.begin() + y == text.end()) {
    if (GetChar(ch) == U'\n')
      return {{anchor.x(), anchor.y()}, Forward};

    if (IsDoubleWidth(ch)) {
      text.push_back({ch, PlaceHolder});
      return {{p.x() + 1, p.y()}, Backward};
    } else {
      text.push_back({ch});
      return {{p.x(), p.y()}, Backward};
    }
  }

  if (GetChar(ch) == U'\n') {
    if (ep == Backward && text[y].size() == x + 1)
      return {{anchor.x(), p.y() + 1}, Forward};

    int dividePosition = x;
    if (ep == Backward)
      dividePosition = IsDoubleWidth(text[y][x]) ? x + 2 : x + 1;

    std::vector<Char> newLine(text[y].begin() + dividePosition, text[y].end());
    text.insert(text.begin() + y + 1, newLine);
    text[y].erase(text[y].begin() + dividePosition, text[y].end());

    if (text[y].size() == 0) {
      text.erase(text.begin() + y);
      if (y == 0)
        anchor.ry()++;
    }

    return {{anchor.x(), p.y() + 1}, Forward};
  }

  int insertWidth = IsDoubleWidth(ch) ? 2 : 1;
  int existWidth = IsDoubleWidth(text[y][x]) ? 2 : 1;
  int moveWidth = ep == Backward ? existWidth + insertWidth : existWidth;
  --moveWidth;
  int insertPlace = ep == Forward ? x : x + existWidth;

  text[y].insert(text[y].begin() + insertPlace, ch);
  if (IsDoubleWidth(ch))
    text[y].insert(text[y].begin() + insertPlace + 1, PlaceHolder);

  return {{p.x() + moveWidth, p.y()}, ep};
}

EditResult Block::insertUp2Down(QPoint p, Char ch, EditPosition ep) {
  int y = p.y() - anchor.y();
  int x = p.x() - anchor.x();

  if (GetChar(ch) == U'\n')
    return {p, ep};

  if (ep == Backward) {
    text[y].insert(text[y].begin() + x + 1, ch);
    return {{p.x(), p.y() + 2}, ep};
  }

  text[y].insert(text[y].begin() + x, ch);
  return {{p.x(), p.y() + 1}, ep};
}

EditResult Block::insertChar(QPoint p, Char ch, EditPosition ep) {
  int y = p.y() - anchor.y();
  int x = p.x() - anchor.x();

  if (text.begin() + y > text.end() || y < 0)
    qFatal() << "Beyond the boundary";

  if (text.begin() + y < text.end()) {
    if (text[y].size() > 0) {
      if (text[y].begin() + x >= text[y].end() || x < 0)
        qFatal() << "Beyond the boundary";
    } else {
      if (text[y].begin() + x > text[y].end() || x < 0)
        qFatal() << "Beyond the boundary";
    }
  }

  if (direction == Left2Right)
    return insertLeft2Right(p, ch, ep);

  return insertUp2Down(p, ch, ep);
}

EditResult Block::deleteLeft2Right(QPoint p, EditPosition ep) {
  const int y = p.y() - anchor.y();
  const int x = p.x() - anchor.x();

  if (ep == Forward) {
    if (x == 0 && y == 0)
      return {p, Forward};

    if (x == 0) {
      int mergePosition = text[y - 1].size() - 1;
      text[y - 1].insert(text[y - 1].end(), text[y].begin(), text[y].end());
      text.erase(text.begin() + y);
      return {{anchor.x() + mergePosition, p.y() - 1}, Backward};
    }

    if (IsPlaceHolder(text[y][x]))
      p.rx()--;

    p.rx()--;
    return deleteLeft2Right(p, Backward);
  }

  if (IsPlaceHolder(text[y][x])) {
    p.rx()--;
    return deleteLeft2Right(p, ep);
  }

  if (IsDoubleWidth(text[y][x]))
    text[y].erase(text[y].begin() + x);

  text[y].erase(text[y].begin() + x);
  if (text[y].size() == 0) {
    text.erase(text.begin() + y);
    if (!text.empty()) {
      int prevLineEndPos = anchor.x() + text[y - 1].size() - 1;
      return {{prevLineEndPos, p.y() - 1}, Backward};
    }
  }

  if (p.x() == anchor.x())
    return {{p.x(), p.y()}, Forward};

  return {{p.x() - 1, p.y()}, ep};
}

EditResult Block::deleteUp2Down(QPoint p, EditPosition ep) {
  const int y = p.y() - anchor.y();
  const int x = p.x() - anchor.x();

  if (text.empty())
    return {{p.x(), p.y() - 1}, ep};

  if (ep == Forward) {
    if (y == 0)
      return {p, Forward};

    p.ry()--;
    return deleteChar(p, Backward);
  }

  text[x].erase(text[x].begin() + y);
  if (text[x].size() == 0)
    return {{p.x(), anchor.y()}, Forward};

  return {{p.x(), p.y() - 1}, ep};
}

EditResult Block::deleteChar(QPoint p, EditPosition ep) {
  const int y = p.y() - anchor.y();
  const int x = p.x() - anchor.x();

  if (text.begin() + y > text.end() || y < 0)
    qFatal() << "Beyond the boundary";

  if (text.begin() + y < text.end()) {
    if (text[y].size() > 0) {
      if (text[y].begin() + x >= text[y].end() || x < 0)
        qFatal() << "Beyond the boundary";
    } else if (text[y].begin() + x > text[y].end() || x < 0) {
      qFatal() << "Beyond the boundary";
    }
  }

  if (direction == Left2Right)
    return deleteLeft2Right(p, ep);

  return deleteUp2Down(p, ep);
}

quint64 Block::CreateOnlyID() {
  if (lastID == 0) {
    QDateTime now = QDateTime::currentDateTime();
    QString idStr = now.toString("yyyyMMddHHmmsszzz");
    lastID = idStr.toULongLong();
  }

  while (true) {
    QDateTime now = QDateTime::currentDateTime();

    QString idStr = now.toString("yyyyMMddHHmmsszzz");
    quint64 id = idStr.toULongLong();

    if (id > lastID) {
      lastID = id;
      return id;
    }

    // The smallest granularity of thread sleep depends on
    // the operating system's timer resolution
    // So to be precise, it's "at least" 5 milliseconds between intervals.
    QThread::msleep(5);
  }
}

QJsonObject Encode(const Block &b) {
  QJsonObject obj;
  obj["id"] = QString::number(b.id);
  obj["groupId"] = QString::number(b.groupId);
  obj["semantics"] = b.semantics;
  obj["direction"] = b.direction == Left2Right ? "Left2Right" : "Up2Down";

  QJsonObject anchor;
  anchor["x"] = b.anchor.x();
  anchor["y"] = b.anchor.y();
  obj["anchor"] = anchor;

  QJsonArray content;
  for (const auto &lineVec : b.text) {
    QJsonArray line;
    for (const auto &ch : lineVec)
      line.append(static_cast<qint64>(ch));
    content.append(line);
  }

  obj["content"] = content;
  return obj;
}

Block Decode(const QJsonObject &obj) {
  Block b;
  b.id = obj["id"].toString().toULongLong();
  b.groupId = obj["groupId"].toString().toULongLong();
  b.semantics = obj["semantics"].toString();
  b.direction =
      (obj["direction"].toString() == "Left2Right") ? Left2Right : Up2Down;

  QJsonObject anchor = obj["anchor"].toObject();
  b.anchor.setX(anchor["x"].toInt());
  b.anchor.setY(anchor["y"].toInt());

  QJsonArray contentArr = obj["content"].toArray();
  for (const auto &lineVal : contentArr) {
    std::vector<Lin::Char> line;
    QJsonArray lineArr = lineVal.toArray();
    for (const auto &chVal : lineArr)
      line.push_back(static_cast<Lin::Char>(chVal.toInteger()));

    b.text.push_back(line);
  }

  return b;
}

} // namespace Lin
