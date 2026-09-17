#include "Basic/Canvas.h"
#include "Edit/Core.h"
#include "Edit/Register.h"
#include <vector>

namespace Lin::Edit {

namespace {

bool fitsCanvas(const Block &block) {
  const QPoint anchor = block.getAnchor();
  const auto &text = block.getText();

  for (int row = 0; row < static_cast<int>(text.size()); ++row) {
    for (int col = 0; col < static_cast<int>(text[row].size()); ++col) {
      const int x = anchor.x() + col;
      const int y = anchor.y() + row;
      if (x <= 0 || x > Canvas::Width || y <= 0 || y > Canvas::Height)
        return false;
    }
  }

  return true;
}

} // namespace

void Core::enterLin() {
  if (mode != Mode::Normal || !linContext.c)
    return;

  Canvas &canvas = *linContext.c;
  Canvas &tmp = fileManager.getTmpCanvas();
  if (!linContext.tmpPrepared) {
    tmp.reset();
    tmp.moveCursorToPos(canvas.getCursorPos());
    for (const Block &block : linContext.original) {
      canvas.deleteBlock(block.getId(), false);
      tmp.addBlock(block, false);
    }
    linContext.tmpPrepared = true;
  }
  setMode(Mode::Lin);
}

void Core::enterLinBlock() {
  if (mode != Mode::Normal)
    return;

  Canvas &canvas = fileManager.getCurrentCanvas();
  const Block *block = canvas.getBlock(canvas.getCursorPos());
  if (!block)
    return;

  linContext.c = &canvas;
  linContext.original = {*block};
  linContext.tmpPrepared = false;
  enterLin();
}

void Core::enterLinGroup() {
  if (mode != Mode::Normal)
    return;

  Canvas &canvas = fileManager.getCurrentCanvas();
  const Block *current = canvas.getBlock(canvas.getCursorPos());
  if (!current)
    return;

  linContext.c = &canvas;
  linContext.original.clear();
  for (const auto &[_, block] : canvas.getBlocks()) {
    if (block.getGroupId() == current->getGroupId())
      linContext.original.push_back(block);
  }
  linContext.tmpPrepared = false;
  enterLin();
}

void Core::moveSelectedBlocks(int offsetX, int offsetY) {
  if (mode != Mode::Lin || !linContext.c)
    return;

  Canvas &tmp = fileManager.getTmpCanvas();
  const QPoint cursor = tmp.getCursorPos();
  std::vector<Block> moved;
  moved.reserve(tmp.getBlocks().size());
  for (const auto &[_, block] : tmp.getBlocks()) {
    Block candidate = block;
    candidate.moveBy(offsetX, offsetY);
    if (!fitsCanvas(candidate))
      return;
    moved.push_back(std::move(candidate));
  }

  tmp.reset();
  tmp.moveCursorToPos(cursor);
  for (Block &block : moved)
    tmp.addBlock(std::move(block), false);
  tmp.moveCursorByOffSet(offsetX, offsetY);
}

void Core::linCopy() {
  if (mode != Mode::Lin || !linContext.c)
    return;

  Blocks copied;
  for (const auto &[_, block] : fileManager.getTmpCanvas().getBlocks())
    copied.push_back(block);

  auto &reg = registerManager.getRegister(';');
  if (copied.size() == 1)
    reg = copied.front();
  else if (!copied.empty())
    reg = std::move(copied);

  registerManager.syncToSystemClipboard();
  cancelLin();
}

void Core::linDelete() {
  if (mode != Mode::Lin || !linContext.c)
    return;

  Blocks copied;
  for (const auto &[_, block] : fileManager.getTmpCanvas().getBlocks())
    copied.push_back(block);
  auto &reg = registerManager.getRegister(';');
  if (copied.size() == 1)
    reg = copied.front();
  else if (!copied.empty())
    reg = std::move(copied);

  registerManager.syncToSystemClipboard();
  Canvas &canvas = *linContext.c;
  Change batch;
  batch.op = Change::Batch;
  
  for (const Block &block : linContext.original) {
    const Block *existing = canvas.getBlock(block.getId());
    if (existing) {
      Change change;
      change.op = Change::Delete;
      change.oldBlock = *existing;
      batch.changes.push_back(std::move(change));
    }
  }
  
  for (const Block &block : linContext.original) {
    if (canvas.getBlock(block.getId()))
      canvas.deleteBlock(block.getId(), false);
  }
  
  if (!batch.changes.empty())
    canvas.pushBatchOperation(batch);

  fileManager.getTmpCanvas().reset();
  linContext = {};
  setMode(Mode::Normal);
}

void Core::confirmLin() {
  if (mode != Mode::Lin || !linContext.c)
    return;

  Canvas &canvas = *linContext.c;
  Canvas &tmp = fileManager.getTmpCanvas();
  for (const auto &[_, block] : tmp.getBlocks()) {
    if (!fitsCanvas(block)) {
      cancelLin();
      return;
    }
  }

  Change batch;
  batch.op = Change::Batch;
  
  for (const Block &block : linContext.original) {
    const Block *existing = canvas.getBlock(block.getId());
    if (existing) {
      Change change;
      change.op = Change::Delete;
      change.oldBlock = *existing;
      batch.changes.push_back(std::move(change));
    }
  }
  
  std::vector<Block> addedBlocks;
  for (const auto &[_, block] : tmp.getBlocks()) {
    addedBlocks.push_back(block);
    Change change;
    change.op = Change::Add;
    change.newBlock = block;
    batch.changes.push_back(std::move(change));
  }

  for (const Block &block : linContext.original) {
    if (canvas.getBlock(block.getId()))
      canvas.deleteBlock(block.getId(), false);
  }
  
  for (const Block &b : addedBlocks)
    canvas.addBlock(b, false);
  
  const QPoint cursor = tmp.getCursorPos();
  canvas.moveCursorToPos(cursor);
  
  if (!batch.changes.empty())
    canvas.pushBatchOperation(batch);

  tmp.reset();
  linContext = {};
  setMode(Mode::Normal);
}

void Core::cancelLin() {
  if (mode != Mode::Lin || !linContext.c)
    return;

  Canvas &canvas = *linContext.c;
  Canvas &tmp = fileManager.getTmpCanvas();
  const QPoint cursor = tmp.getCursorPos();
  tmp.reset();
  for (const Block &block : linContext.original)
    canvas.addBlock(block, false);
  canvas.moveCursorToPos(cursor);

  linContext = {};
  setMode(Mode::Normal);
}

} // namespace Lin::Edit
