#include "Edit/Interpreter.h"
#include "Basic/Char.h"
#include "Basic/Cursor.h"
#include "Basic/Direction.h"
#include "Edit/Core.h"
#include "Edit/Key.h"
#include "Edit/Mode.h"
#include <functional>

namespace Lin::Edit {

Interpreter::Interpreter(Core &core) : core(core), count(-1), next(nullptr) {}

void Interpreter::normal(Key k) {
  Qt::Key key = k.key;
  Qt::KeyboardModifiers mods = k.mods;

  if (mods == Qt::ShiftModifier)
    goto capital;

  if (mods == Qt::ControlModifier)
    goto control;

  if (key >= Qt::Key_0 && key <= Qt::Key_9) {
    count = count == -1 ? 0 : count;
    count = count * 10 + (key - Qt::Key_0);
    return;
  }

  switch (key) {
  case Qt::Key_Escape:
    done();
    break;
  case Qt::Key_A:
    core.enterInsert(Backward, true);
    break;
  case Qt::Key_I:
    core.enterInsert(Forward, false);
    break;
  case Qt::Key_V:
    core.enterVisualBlock();
    break;
  case Qt::Key_G:
    core.enterLinBlock();
    break;
  case Qt::Key_Space:
    core.mirrorMove();
    break;
  case Qt::Key_H:
  case Qt::Key_J:
  case Qt::Key_K:
  case Qt::Key_L:
    parseToMotion(key);
    break;
  case Qt::Key_D:
    pendingOperator = Operator::Delete;
    operatorStart = core.cursorPos();
    return;
  case Qt::Key_Y:
    pendingOperator = Operator::Yank;
    operatorStart = core.cursorPos();
    return;
  case Qt::Key_W:
    core.moveCursorWord(1);
    break;
  case Qt::Key_E:
    core.moveCursorWord(0);
    break;
  case Qt::Key_B:
    core.moveCursorWord(-1);
    break;
  case Qt::Key_P:
    core.paste();
    break;
  case Qt::Key_U:
    core.undo();
    break;
  default:
    break;
  }

  return;

capital:
  switch (key) {
  case Qt::Key_Space:
    core.mirrorMove(true);
    break;
  case Qt::Key_I:
    core.enterInsert(Forward, true, Up2Down);
    break;
  case Qt::Key_A:
    core.enterInsert(Backward, true, Up2Down);
    break;
  case Qt::Key_G:
    core.enterLinGroup();
    break;
  case Qt::Key_V:
    core.enterVisualRectangle();
    break;
  case Qt::Key_H:
  case Qt::Key_J:
  case Qt::Key_K:
  case Qt::Key_L:
    count = 8;
    parseToMotion(key);
    break;
  case Qt::Key_U:
    core.redo();
    break;
  case Qt::Key_P:
    core.paste(true);
    break;
  default:
    break;
  }

  return;

control:
  switch (key) {
  case Qt::Key_S:
  case Qt::Key_W:
    core.saveFile();
    break;
  case Qt::Key_Q:
    core.exit();
    done();
    break;
  case Qt::Key_V:
    core.enterVisualRectangle();
    done();
    break;
  default:
    break;
  }
}

void Interpreter::operatorMotion(Key k) {
  if (pendingOperator == Operator::None)
    return;

  switch (k.key) {
  case Qt::Key_H:
    core.moveCursorToPrevChar();
    break;
  case Qt::Key_J:
    core.moveCursorByOffSet(0, 1);
    break;
  case Qt::Key_K:
    core.moveCursorByOffSet(0, -1);
    break;
  case Qt::Key_L:
    core.moveCursorToNextChar();
    break;
  case Qt::Key_W:
    core.moveCursorWord(1);
    break;
  case Qt::Key_E:
    core.moveCursorWord(0);
    break;
  case Qt::Key_B:
    core.moveCursorWord(-1);
    break;
  default:
    pendingOperator = Operator::None;
    return;
  }

  const QPoint end = core.cursorPos();
  if (pendingOperator == Operator::Delete)
    core.deleteMotion(operatorStart, end, k.key != Qt::Key_W);
  else
    core.copyMotion(operatorStart, end);
  pendingOperator = Operator::None;
}

void Interpreter::insert(QString ss) {
  if (ss.isEmpty())
    return;

  Chars cs = StringToChars(ss, true /*no placeholder*/);
  for (auto &line : cs)
    for (auto &c : line)
      core.insertChar(c);
}

void Interpreter::visual(Key k) {
  const int countValue = count == -1 ? 1 : count;

  if (k.mods == Qt::ControlModifier && k.key == Qt::Key_V) {
    core.enterVisualRectangle();
    done();
    return;
  }

  if (k.mods != Qt::NoModifier && k.mods != Qt::ShiftModifier)
    return;

  if (k.mods == Qt::ShiftModifier &&
      (k.key == Qt::Key_H || k.key == Qt::Key_J || k.key == Qt::Key_K ||
       k.key == Qt::Key_L)) {
    const int distance = 8 * countValue;
    switch (k.key) {
    case Qt::Key_H:
      core.moveVisualCursor(-distance, 0);
      break;
    case Qt::Key_J:
      core.moveVisualCursor(0, distance);
      break;
    case Qt::Key_K:
      core.moveVisualCursor(0, -distance);
      break;
    case Qt::Key_L:
      core.moveVisualCursor(distance, 0);
      break;
    default:
      break;
    }
    count = -1;
    return;
  }

  switch (k.key) {
  case Qt::Key_Escape:
    core.exitVisual();
    done();
    return;
  case Qt::Key_H:
    core.moveVisualCursor(-countValue, 0);
    break;
  case Qt::Key_J:
    core.moveVisualCursor(0, countValue);
    break;
  case Qt::Key_K:
    core.moveVisualCursor(0, -countValue);
    break;
  case Qt::Key_L:
    core.moveVisualCursor(countValue, 0);
    break;
  case Qt::Key_W:
    core.moveVisualWord(1);
    break;
  case Qt::Key_E:
    core.moveVisualWord(0);
    break;
  case Qt::Key_B:
    core.moveVisualWord(-1);
    break;
  case Qt::Key_Y:
    core.visualCopy();
    done();
    return;
  case Qt::Key_D:
  case Qt::Key_X:
    core.visualDelete();
    done();
    return;
  default:
    return;
  }

  count = -1;
}

void Interpreter::translate(QKeyEvent *eK, QInputMethodEvent *e) {
  Qt::Key key;
  Qt::KeyboardModifiers mod;

  if (eK) {
    key = static_cast<Qt::Key>(eK->key());
    mod = eK->modifiers();

    inputList.push_back({key, mod});
  }

  switch (core.getMode()) {
  case Mode::Normal:
    if (pendingOperator != Operator::None) {
      if (eK)
        operatorMotion({key, mod});
      else
        pendingOperator = Operator::None;
      break;
    }
    if (next) {
      next({key, mod});
      return;
    }

    normal({key, mod});
    break;
  case Mode::Insert:
    if (e) {
      insert(e->commitString());
      return;
    }

    switch (key) {
    case Qt::Key_Backspace:
      core.deleteChar();
      break;
    case Qt::Key_Escape:
      core.exitInsert();
      break;
    case Qt::Key_Return:
    case Qt::Key_Enter:
      core.insertChar(U'\n');
      break;
    default:
      insert(eK->text());
      break;
    }

    break;
  case Mode::Visual:
    visual({key, mod});
    break;
  case Mode::Lin: {
    if (key >= Qt::Key_0 && key <= Qt::Key_9) {
      count = count == -1 ? 0 : count;
      count = count * 10 + (key - Qt::Key_0);
      break;
    }
    if (key == Qt::Key_Escape) {
      core.cancelLin();
      done();
      break;
    }
    if (key == Qt::Key_Return || key == Qt::Key_Enter) {
      core.confirmLin();
      done();
      break;
    }
    if (key == Qt::Key_D || key == Qt::Key_X) {
      core.linDelete();
      done();
      break;
    }
    if (key == Qt::Key_Y) {
      core.linCopy();
      done();
      break;
    }

    const int countValue = count == -1 ? 1 : count;
    const int distance =
        (mod == Qt::ShiftModifier) ? 8 * countValue : countValue;
    if (key == Qt::Key_H)
      core.moveSelectedBlocks(-distance, 0);
    else if (key == Qt::Key_J)
      core.moveSelectedBlocks(0, distance);
    else if (key == Qt::Key_K)
      core.moveSelectedBlocks(0, -distance);
    else if (key == Qt::Key_L)
      core.moveSelectedBlocks(distance, 0);
    else
      break;

    count = -1;
    break;
  }
  }
}

void Interpreter::done() {
  // Reset
  inputList.clear();
  count = -1;
  next = nullptr;
}

void Interpreter::parseToMotion(Qt::Key k) {
  int n = count == -1 ? 1 : count;

  switch (k) {
  case Qt::Key_H:
    if (n == 1)
      core.moveCursorToPrevChar();
    else
      core.moveCursorByOffSet(-n, 0);
    break;
  case Qt::Key_J:
    core.moveCursorByOffSet(0, n);
    break;
  case Qt::Key_K:
    core.moveCursorByOffSet(0, -n);
    break;
  case Qt::Key_L:
    if (n == 1)
      core.moveCursorToNextChar();
    else
      core.moveCursorByOffSet(n, 0);
    break;
  default:
    break;
  }

  done();
}

} // namespace Lin::Edit
