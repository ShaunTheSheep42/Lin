#include "Basic/Block.h"
#include "Basic/Char.h"
#include "Basic/Cursor.h"
#include "Basic/Direction.h"
#include "Edit/Core.h"
#include "Edit/Mode.h"
#include <optional>

namespace Lin::Edit {

void Core::enterInsert(EditPosition ep, bool isNewGroup, Direction d) {
  setMode(Mode::Insert);

  Canvas &c = fileManager.getCurrentCanvas();
  fileManager.getTmpCanvas().moveCursorToPos(c.getCursorPos());

  if (const Block *b = c.getBlock(c.getCursorPos())) {
    insertContext.b = *b;
    insertContext.anchor = b->getAnchor();
    insertContext.ep = ep;
    insertContext.groupId = c.getGroupId(c.getCursorPos());
    insertContext.direction = b->getDirection();

    Canvas &t = fileManager.getTmpCanvas();
    t.addBlock(*insertContext.b);
    if (d == Left2Right) {
      auto ch = b->getRawChar(c.getCursorPos());
      if (IsPlaceHolder(ch) && ep == Forward)
        t.moveCursorByOffSet(-1, 0);

      if (IsDoubleWidth(ch) && ep == Backward)
        t.moveCursorByOffSet(1, 0);
    }

    return;
  }

  insertContext.b = std::nullopt;
  insertContext.anchor = c.getCursorPos();
  insertContext.ep = Forward;

  if (isNewGroup)
    insertContext.groupId = -1;
  else
    insertContext.groupId = c.getGroupId(c.getCursorPos());

  insertContext.direction = d;
}

void Core::exitInsert() {
  auto &t = fileManager.getTmpCanvas();
  Canvas &c = fileManager.getCurrentCanvas();
  c.moveCursorToPos(t.getCursorPos());
  if (!insertContext.b) {
    t.reset();
    setMode(Mode::Normal);
    return;
  }

  Block edited = *insertContext.b;
  const Block *existing = c.getBlock(edited.getId());

  if (c.canPlace(edited)) {
    if (existing)
      c.updateBlock(edited.getId(), edited);
    else
      c.addBlock(edited);

    insertContext.b = std::nullopt;
    t.reset();
    setMode(Mode::Normal);
    return;
  }

  // Keep the edited block in TmpCanvas and enter Lin mode so it can be
  // moved away from the blocks that remain on the main canvas.
  linContext.c = &c;
  linContext.original.clear();
  linContext.tmpPrepared = true;
  if (existing) {
    linContext.original.push_back(*existing);
    c.deleteBlock(existing->getId(), false);
  }

  insertContext.b = std::nullopt;
  setMode(Mode::Lin);
}

void Core::insertChar(Char ch) {
  Canvas &t = fileManager.getTmpCanvas();
  if (!insertContext.b) {
    if (insertContext.groupId == -1) {
      Block b(insertContext.anchor, {}, insertContext.direction);
      EditResult er = b.insertChar(insertContext.anchor, ch, insertContext.ep);
      insertContext.b = b;
      t.addBlock(*insertContext.b);
      t.moveCursorToPos(er.nextCursorPos);
      insertContext.ep = er.ep;
      return;
    }

    Block b(insertContext.anchor, insertContext.groupId, {},
            insertContext.direction);
    EditResult er = b.insertChar(insertContext.anchor, ch, insertContext.ep);
    insertContext.b = b;
    t.addBlock(*insertContext.b);

    t.moveCursorToPos(er.nextCursorPos);
    insertContext.ep = er.ep;
    return;
  }

  Block &b = *insertContext.b;
  EditResult er = b.insertChar(t.getCursorPos(), ch, insertContext.ep);
  if (t.getBlock(b.getId()))
    t.updateBlock(insertContext.b->getId(), *insertContext.b);
  else
    t.addBlock(*insertContext.b);

  t.moveCursorToPos(er.nextCursorPos);
  insertContext.ep = er.ep;
}

void Core::deleteChar() {
  if (!insertContext.b || insertContext.b->isEmpty())
    return;

  Block &b = *insertContext.b;
  Canvas &t = fileManager.getTmpCanvas();
  QPoint cur = t.getCursorPos();

  EditResult er = b.deleteChar(cur, insertContext.ep);
  t.updateBlock(insertContext.b->getId(), *insertContext.b);
  t.moveCursorToPos(er.nextCursorPos);
  insertContext.ep = er.ep;
}

} // namespace Lin::Edit
