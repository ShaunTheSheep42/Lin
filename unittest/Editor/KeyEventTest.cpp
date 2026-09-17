#include "Editor/Context.h"
#include "Buffer/Canvas.h"
#include <QtTest/QtTest>

using namespace Lin;

class TestKeyEvent : public QObject {
  Q_OBJECT
private slots:
  void test_w_e_b_key_events() {
    Context ctx;
    QVERIFY(ctx.init(""));
    Canvas c;
    c.cursor = QPoint(10, 10);
    c.blockId = QVector<QVector<quint64>>(GetRows(c), QVector<quint64>(GetCols(c), 0));
    // prepare a block with three cells at (10,10),(10,11),(10,12)
    Block b;
    b.id = 1;
    b.semantics = "";
    for (int y = 10; y <= 12; ++y) {
      Cell cell; cell.row = 10; cell.col = y; cell.ch = QChar('x');
      b.content[QPoint(10, y)] = cell;
      c.blockId[10][y] = 1;
    }
    c.blocks[1] = b;
    ctx.openCanvasForTest(c, true);

    // ensure mode is Normal
    ctx.switchMode(Mode::Normal);

    // place cursor at 10,10
    ctx.moveCursorToPos(QPoint(10, 10));

    // simulate KeyPress 'w'
    QKeyEvent evW(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier, QString("w"));
    ctx.onKeyEvent(&evW);
    QCOMPARE(ctx.getCursorPos().y(), 11);

    // simulate KeyPress 'e'
    QKeyEvent evE(QEvent::KeyPress, Qt::Key_E, Qt::NoModifier, QString("e"));
    ctx.onKeyEvent(&evE);
    QCOMPARE(ctx.getCursorPos().y(), 12);

    // simulate KeyPress 'b' (backwards)
    QKeyEvent evB(QEvent::KeyPress, Qt::Key_B, Qt::NoModifier, QString("b"));
    ctx.onKeyEvent(&evB);
    QCOMPARE(ctx.getCursorPos().y(), 11);
  }
};

QTEST_MAIN(TestKeyEvent)
#include "KeyEventTest.moc"
