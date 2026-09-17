#include "Buffer/Block.h"
#include "Buffer/Canvas.h"
#include "Editor/Context.h"
#include "Editor/InputDirection.h"
#include "Editor/Recognize.h"
#include <QtTest/QtTest>

using namespace Lin;

class TestContextMove : public QObject {
  Q_OBJECT

private slots:
  void testMoveForward() {
    // 构造一个 Block，内容 "Hello world"
    Block b;
    int id = b.id;
    QString str = "Hello world";
    for (int i = 0; i < str.size(); ++i) {
      Cell c;
      c.row = 0;
      c.col = i;
      c.ch = str[i].unicode();
      b.cells[{QPoint(i, 0)}] = c;
    }

    Canvas canvas("forward_path");
    canvas.AddBlock(b);
    Context ctx;
    ctx.setInputDirection(Left2Right);

    // 光标在 "Hello" 内部
    canvas.moveCursorToPos(QPoint(1, 0));
    ctx.moveCursorForward(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(6, 0)); // "world" 开头

    // 光标在 "world" 内部
    canvas.moveCursorToPos(QPoint(7, 0));
    ctx.moveCursorForward(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(10, 0)); // "world" 末尾
  }

  void testMoveEnd() {
    Block b;
    int id = b.id;
    QString str = "abc def";
    for (int i = 0; i < str.size(); ++i) {
      Cell c;
      c.row = 0;
      c.col = i;
      c.ch = str[i].unicode();
      b.cells[{QPoint(i, 0)}] = c;
    }

    Canvas canvas("end_path");
    canvas.AddBlock(b);
    Context ctx;
    ctx.setInputDirection(Left2Right);

    // 光标在 "abc" 内部
    canvas.moveCursorToPos(QPoint(1, 0));
    ctx.moveCursorEnd(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(2, 0)); // "abc" 末尾

    // 光标在 "abc" 末尾
    canvas.moveCursorToPos(QPoint(2, 0));
    ctx.moveCursorEnd(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(6, 0)); // "def" 末尾
  }

  void testMoveBackward() {
    Block b;
    int id = b.id;
    QString str = "foo bar";
    for (int i = 0; i < str.size(); ++i) {
      Cell c;
      c.row = 0;
      c.col = i;
      c.ch = str[i].unicode();
      b.cells[{QPoint(i, 0)}] = c;
    }

    Canvas canvas("backward_path");
    canvas.AddBlock(b);
    Context ctx;
    ctx.setInputDirection(Left2Right);

    // 光标在 "bar" 内部
    canvas.moveCursorToPos(QPoint(5, 0));
    ctx.moveCursorBackward(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(4, 0)); // "bar" 开头

    // 光标在 "bar" 开头
    canvas.moveCursorToPos(QPoint(4, 0));
    ctx.moveCursorBackward(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(0, 0)); // "foo" 开头
  }

  void testSingleToken() {
    Block b;
    int id = b.id;
    QString str = "single";
    for (int i = 0; i < str.size(); ++i) {
      Cell c;
      c.row = 0;
      c.col = i;
      c.ch = str[i].unicode();
      b.cells[{QPoint(i, 0)}] = c;
    }

    Canvas canvas("single_path");
    canvas.AddBlock(b);
    Context ctx;
    ctx.setInputDirection(Left2Right);

    // 光标在 token 内部
    canvas.moveCursorToPos(QPoint(2, 0));
    ctx.moveCursorForward(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(5, 0)); // token 末尾

    canvas.moveCursorToPos(QPoint(2, 0));
    ctx.moveCursorEnd(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(5, 0)); // token 末尾

    canvas.moveCursorToPos(QPoint(5, 0));
    ctx.moveCursorBackward(canvas.getBlock(id).id);
    QCOMPARE(canvas.getCursorPos(), QPoint(0, 0)); // token 开头
  }
};

QTEST_MAIN(TestContextMove)
#include "TestContextMove.moc"
