#include "Edit/Core.h"
#include "Basic/Canvas.h"
#include "Edit/File.h"
#include "Edit/Interpreter.h"
#include "Edit/Mode.h"
#include <QColor>
#include <QCommandLineOption>
#include <QDateTime>
#include <QDebug>
#include <QEvent>
#include <QFile>
#include <algorithm>
#include <type_traits>

namespace Lin::Edit {

Core::Core(Config &cfg, QObject *parent)
    : cfg(cfg), QObject(parent), fileManager(""), mode(Mode::Normal),
      interpreter(*this) {}

void Core::exit() {
  fileManager.closeAllFile();
  quit();
}

void Core::setMode(Mode m) { mode = m; }

void Core::openFile(QString path, bool isCurrent) {
  int id = fileManager.openFile(path, isCurrent);

  // Restore cursor position or center it
  if (!cfg.useCursorLastPos)
    fileManager.getCanvas(id).moveCursorToPos(
        {Canvas::Height / 2, Canvas::Width / 2});

  doUpdate();
  // TODO:
  // whenOpen()
}

void Core::saveFile() {
  fileManager.saveToFile(fileManager.getCurrentCanvas());
}

void Core::closeFile(int id) {
  // TODO:
  // whenClose()

  fileManager.closeFile(id);
}

void Core::doUpdate() {
  if (fileManager.getCurrentId() == -1)
    return;

  std::optional<Region> visualRegion;
  if (mode == Mode::Visual) {
    visualRegion = visualContext.r;
  } else if (mode == Mode::Lin) {
    std::vector<quint64> ids;
    ids.reserve(fileManager.getTmpCanvas().getBlocks().size());
    for (const auto &[id, _] : fileManager.getTmpCanvas().getBlocks())
      ids.push_back(id);
    visualRegion = std::move(ids);
  }

  RenderData rd{
      .mode = this->mode,
      .mainCanvas = &fileManager.getCurrentCanvas(),
      .tmpCanvas = &fileManager.getTmpCanvas(),
      .block = insertContext.b,
      .ep = insertContext.ep,
      .direction = insertContext.direction,
      .visualRegion = std::move(visualRegion),
      .visualRectangle = visualContext.visualRectangle,
  };
  emit update(rd);
}

void Core::handleEvent(QKeyEvent *eK, QInputMethodEvent *e) {
  interpreter.translate(eK, e);
  if (fileManager.getCurrentId() != -1)
    doUpdate();
}

// void Context::beginOperation() {
//   currentOpChanges.clear();
//   inOperation = true;
// }
//
// void Context::recordChange(const UndoManager::Change &ch) {
//   if (!inOperation)
//     beginOperation();
//   currentOpChanges.push_back(ch);
// }
//
// void Context::endOperation() {
//   if (!inOperation)
//     return;
//
//   if (!currentOpChanges.isEmpty()) {
//     UndoManager::Operation op;
//     op.changes = currentOpChanges;
//     undoManager.pushOperation(op);
//   }
//
//   currentOpChanges.clear();
//   inOperation = false;
//   currentOpDesc.clear();
// }
//
// // finalize tmp insert: try to merge tmpCanvas into current canvas;
// // on overlap -> enter Lin mode and keep transient for move operations
// void Context::finalizeInsert() {
//   if (!tmpCanvas.has_value())
//     return;
//
//   Canvas *cptr = getCanvas();
//   if (!cptr)
//     return;
//   Canvas &c = *cptr;
//
//   Canvas &t = tmpCanvas.value();
//
//   // detect overlap: any cell in transient that maps to non-zero in c.blockId
//   if (!c.blockId.empty()) {
//     for (auto itb = t.blocks.constBegin(); itb != t.blocks.constEnd(); ++itb)
//     {
//       const Block &tb = itb.value();
//       for (auto it = tb.content.constBegin(); it != tb.content.constEnd();
//            ++it) {
//         QPoint p = it.key();
//         if (c.blockId[p.x()][p.y()] != 0) {
//           // overlap detected -> enter Lin mode and keep transient for moving
//           mode = Mode::Lin;
//           emit Updated();
//           return;
//         }
//       }
//     }
//   }
//
//   // no overlap -> merge transient blocks into main canvas
//   for (auto itb = t.blocks.constBegin(); itb != t.blocks.constEnd(); ++itb) {
//     quint64 key = itb.key();
//     const Block &tb = itb.value();
//     c.blocks[key] = tb;
//     for (auto it = tb.content.constBegin(); it != tb.content.constEnd();
//     ++it) {
//       QPoint p = it.key();
//       if (c.blockId.empty())
//         c.blockId = QVector<QVector<quint64>>(GetRows(c),
//                                               QVector<quint64>(GetCols(c),
//                                               0));
//       c.blockId[p.x()][p.y()] = key;
//     }
//   }
//
//   clearTmpCanvas();
//   // reset active tmp block id
//   activeTmpBlockId = 0;
//   emit Updated();
// }
//
//
// void Context::doAppend(int id) {
//   Canvas *cptr = getCanvas(id);
//   if (!cptr)
//     return;
//   Canvas &c = *cptr;
//   int maxC = GetCols(c);
//   QPoint &cur = c.cursor;
//   if (cur.y() < maxC - 1)
//     cur.setY(cur.y() + 1);
//
//   // enter insert
//   mode = Mode::Insert;
//   c.cursor = cur;
//   emit Updated();
// }
//
// void Context::doOpenBelow(int id) {
//   Canvas *cptr = getCanvas(id);
//   if (!cptr)
//     return;
//   Canvas &c = *cptr;
//   int maxR = GetRows(c);
//   QPoint &cur = c.cursor;
//   if (cur.x() < maxR - 1)
//     cur.setX(cur.x() + 1);
//   mode = Mode::Insert;
//   c.cursor = cur;
//   emit Updated();
// }
//
//
// void Context::doDelete(int id) {
//   Canvas *cptr = getCanvas(id);
//   if (!cptr)
//     return;
//   Canvas &c = *cptr;
//   beginOperation();
//   if (mode == Mode::Lin) {
//     if (!selection.isEmpty()) {
//       // delete every selected point
//       for (const QPoint &p : selection) {
//         int r = p.x();
//         int cc = p.y();
//         if (!c.blockId.isEmpty()) {
//           quint64 bid = c.blockId[r][cc];
//           if (bid != 0 && c.blocks.contains(bid)) {
//             Block &b = c.blocks[bid];
//             QPoint q(r, cc);
//             std::optional<Cell> beforeCell = std::nullopt;
//             if (b.content.contains(q))
//               beforeCell = b.content[q];
//             if (b.content.contains(q))
//               b.content.remove(q);
//             if (b.content.isEmpty())
//               c.blocks.remove(bid);
//             c.blockId[r][cc] = 0;
//
//             UndoManager::Change chg;
//             chg.pos = q;
//             chg.before = beforeCell;
//             chg.after = std::nullopt;
//             chg.beforeBlockId = bid;
//             chg.afterBlockId = 0;
//             recordChange(chg);
//           }
//         }
//       }
//
//       // keep cursor where it was
//       QPoint &cur = c.cursor;
//       c.cursor = cur;
//       emit Updated();
//       // clear selection after deletion
//       selection.clear();
//       endOperation();
//       return;
//     }
//
//     // no selection points -> delete single cell at cursor
//     QPoint &cur = c.cursor;
//     int rr = cur.x(), cc = cur.y();
//     QPoint pos(rr, cc);
//     quint64 beforeBid = 0;
//     std::optional<Cell> beforeCell = std::nullopt;
//     if (!c.blockId.isEmpty()) {
//       beforeBid = c.blockId[rr][cc];
//       if (beforeBid != 0 && c.blocks.contains(beforeBid)) {
//         Block &b = c.blocks[beforeBid];
//         if (b.content.contains(pos))
//           beforeCell = b.content[pos];
//       }
//     }
//
//     if (beforeBid != 0 && c.blocks.contains(beforeBid)) {
//       Block &b = c.blocks[beforeBid];
//       if (beforeCell.has_value())
//         b.content.remove(pos);
//       if (b.content.isEmpty())
//         c.blocks.remove(beforeBid);
//       c.blockId[rr][cc] = 0;
//
//       UndoManager::Change chg;
//       chg.pos = pos;
//       chg.before = beforeCell;
//       chg.after = std::nullopt;
//       chg.beforeBlockId = beforeBid;
//       chg.afterBlockId = 0;
//       recordChange(chg);
//     }
//
//     c.cursor = QPoint(rr, cc);
//     emit Updated();
//     endOperation();
//     return;
//   }
//   // delete selection rect for non-Lin modes
//   int r1 = std::min(selectStart.x(), selectEnd.x());
//   int r2 = std::max(selectStart.x(), selectEnd.x());
//   int c1 = std::min(selectStart.y(), selectEnd.y());
//   int c2 = std::max(selectStart.y(), selectEnd.y());
//   for (int r = r1; r <= r2; ++r) {
//     for (int cc = c1; cc <= c2; ++cc) {
//       if (!c.blockId.isEmpty()) {
//         quint64 bid = c.blockId[r][cc];
//         if (bid != 0 && c.blocks.contains(bid)) {
//           Block &b = c.blocks[bid];
//           QPoint p(r, cc);
//           std::optional<Cell> beforeCell = std::nullopt;
//           if (b.content.contains(p))
//             beforeCell = b.content[p];
//           if (b.content.contains(p))
//             b.content.remove(p);
//           if (b.content.isEmpty())
//             c.blocks.remove(bid);
//           c.blockId[r][cc] = 0;
//
//           UndoManager::Change chg;
//           chg.pos = p;
//           chg.before = beforeCell;
//           chg.after = std::nullopt;
//           chg.beforeBlockId = bid;
//           chg.afterBlockId = 0;
//           recordChange(chg);
//         }
//       }
//     }
//   }
//
//   c.cursor = QPoint(selectStart.x(), selectStart.y());
//   emit Updated();
//   endOperation();
// }
//
// void Context::doMove(int id) {
//   Canvas *cptr = getCanvas(id);
//   if (!cptr)
//     return;
//   Canvas &c = *cptr;
//   const auto cursorRow = c.cursor.x();
//   const auto cursorCol = c.cursor.y();
//   // yank into clipboard first
//   doYank(id);
//   beginOperation();
//
//   // compute deletion positions
//   QVector<QPoint> dels;
//   if (mode == Mode::Lin) {
//     if (!selection.isEmpty()) {
//       for (const QPoint &p : selection)
//         dels.push_back(p);
//     } else {
//       dels.push_back(QPoint(cursorRow, cursorCol));
//     }
//   } else {
//     int r1 = std::min(selectStart.x(), selectEnd.x());
//     int r2 = std::max(selectStart.x(), selectEnd.x());
//     int c1 = std::min(selectStart.y(), selectEnd.y());
//     int c2 = std::max(selectStart.y(), selectEnd.y());
//     for (int r = r1; r <= r2; ++r)
//       for (int cc = c1; cc <= c2; ++cc)
//         dels.push_back(QPoint(r, cc));
//   }
//
//   // delete cells (record changes)
//   for (const QPoint &p : dels) {
//     int r = p.x(), cc = p.y();
//     quint64 beforeBid = 0;
//     std::optional<Cell> beforeCell = std::nullopt;
//     if (!c.blockId.isEmpty()) {
//       beforeBid = c.blockId[r][cc];
//       if (beforeBid != 0 && c.blocks.contains(beforeBid)) {
//         Block &b = c.blocks[beforeBid];
//         if (b.content.contains(p))
//           beforeCell = b.content[p];
//       }
//     }
//
//     if (beforeBid != 0 && c.blocks.contains(beforeBid)) {
//       Block &b = c.blocks[beforeBid];
//       if (beforeCell.has_value())
//         b.content.remove(p);
//       if (b.content.isEmpty())
//         c.blocks.remove(beforeBid);
//       c.blockId[r][cc] = 0;
//
//       UndoManager::Change chg;
//       chg.pos = p;
//       chg.before = beforeCell;
//       chg.after = std::nullopt;
//       chg.beforeBlockId = beforeBid;
//       chg.afterBlockId = 0;
//       recordChange(chg);
//     }
//   }
//
//   // paste clipboard at cursor
//   if (!clipboard.isEmpty()) {
//     quint64 newId = CreateID();
//     quint64 key = newId;
//     Block b;
//     b.id = newId;
//     b.semantics = "";
//     int minr = INT_MAX, minc = INT_MAX;
//     for (const Cell &cell : clipboard) {
//       minr = std::min(minr, cell.row);
//       minc = std::min(minc, cell.col);
//     }
//
//     for (const Cell &cell : clipboard) {
//       Cell nc = cell;
//       nc.row = cursorRow + (cell.row - minr);
//       nc.col = cursorCol + (cell.col - minc);
//       QPoint p(nc.row, nc.col);
//       quint64 beforeBid = 0;
//       std::optional<Cell> beforeCell = std::nullopt;
//       if (!c.blockId.isEmpty()) {
//         beforeBid = c.blockId[nc.row][nc.col];
//         if (beforeBid != 0 && c.blocks.contains(beforeBid)) {
//           Block &bb = c.blocks[beforeBid];
//           if (bb.content.contains(p))
//             beforeCell = bb.content[p];
//         }
//       }
//       b.content[p] = nc;
//       if (c.blockId.isEmpty())
//         c.blockId = QVector<QVector<quint64>>(GetRows(c),
//                                               QVector<quint64>(GetCols(c),
//                                               0));
//       c.blockId[nc.row][nc.col] = key;
//       UndoManager::Change chg;
//       chg.pos = p;
//       chg.before = beforeCell;
//       chg.after = nc;
//       chg.beforeBlockId = beforeBid;
//       chg.afterBlockId = newId;
//       recordChange(chg);
//     }
//     c.blocks[key] = b;
//   }
//
//   emit Updated();
//   endOperation();
// }
//
// void Context::executeCommand(QString cmd) { commandManager.execute(cmd); }
//
// void Context::toggleInputDirection() {
//   if (inputDirection == Left2Right)
//     inputDirection = Up2Down;
//   else
//     inputDirection = Left2Right;
// }
//
// void Context::moveTmpBlock(int dx, int dy) {
//   if (!tmpCanvas.has_value() || activeTmpBlockId == 0)
//     return;
//   Canvas &t = tmpCanvas.value();
//   if (!t.blocks.contains(activeTmpBlockId))
//     return;
//
//   // remove previous blockId markings for this block
//   if (!t.blockId.isEmpty()) {
//     int rows = t.blockId.size();
//     int cols = t.blockId[0].size();
//     for (int r = 0; r < rows; ++r)
//       for (int c = 0; c < cols; ++c)
//         if (t.blockId[r][c] == activeTmpBlockId)
//           t.blockId[r][c] = 0;
//   }
//
//   Block b = t.blocks[activeTmpBlockId];
//   QMap<QPoint, Cell> newContent;
//   for (auto it = b.content.constBegin(); it != b.content.constEnd(); ++it) {
//     Cell cell = it.value();
//     cell.row += dx;
//     cell.col += dy;
//     QPoint np(cell.row, cell.col);
//     newContent[np] = cell;
//   }
//   b.content = newContent;
//   t.blocks[activeTmpBlockId] = b;
//
//   // ensure blockId grid exists
//   if (t.blockId.isEmpty())
//     t.blockId =
//         QVector<QVector<quint64>>(GetRows(t), QVector<quint64>(GetCols(t),
//         0));
//   for (auto it = b.content.constBegin(); it != b.content.constEnd(); ++it) {
//     QPoint p = it.key();
//     if (p.x() >= 0 && p.x() < GetRows(t) && p.y() >= 0 && p.y() < GetCols(t))
//       t.blockId[p.x()][p.y()] = activeTmpBlockId;
//   }
//
//   emit Updated();
// }
//
// bool Context::isTmpOverlapping() {
//   if (!tmpCanvas.has_value())
//     return false;
//   Canvas *cptr = getCanvas();
//   if (!cptr)
//     return false;
//   const Canvas &c = *cptr;
//   const Canvas &t = tmpCanvas.value();
//
//   if (c.blockId.empty())
//     return false;
//
//   for (auto itb = t.blocks.constBegin(); itb != t.blocks.constEnd(); ++itb) {
//     const Block &tb = itb.value();
//     for (auto it = tb.content.constBegin(); it != tb.content.constEnd();
//     ++it) {
//       QPoint p = it.key();
//       if (p.x() < 0 || p.x() >= GetRows(c) || p.y() < 0 || p.y() >=
//       GetCols(c))
//         continue;
//       if (c.blockId[p.x()][p.y()] != 0)
//         return true;
//     }
//   }
//   return false;
// }

} // namespace Lin::Edit
