#include "Editor/Context.h"
#include "Buffer/Canvas.h"
#include <QDir>
#include <QFile>
#include <QTemporaryDir>
#include <QtTest/QtTest>

using namespace Lin;

static void addCellToCanvas(Canvas &c, int r, int col, QChar ch, int key = 1) {
  Block &b = c.blocks[key];
  if (b.id == 0) {
    b.id = key;
    b.semantics = "";
  }
  Cell cell;
  cell.row = r;
  cell.col = col;
  cell.ch = ch;
  b.content[QPoint(r, col)] = cell;
  if (c.blockId.isEmpty())
    c.blockId =
        QVector<QVector<quint64>>(GetRows(c), QVector<quint64>(GetCols(c), 0));
  c.blockId[r][col] = key;
}

class TestContext : public QObject {
  Q_OBJECT
private slots:
  void initTestCase() {}

  void cleanupTestCase() {}

  void test_move_cursor() {
    Context ctx;
    QVERIFY(ctx.init(""));
    // create an empty canvas instead of reading a file
    Canvas c_move;
    c_move.cursor = QPoint(0, 0);
    c_move.blockId = QVector<QVector<quint64>>(
        GetRows(c_move), QVector<quint64>(GetCols(c_move), 0));
    ctx.openCanvasForTest(c_move, true);

    QPoint p0 = ctx.getCursorPos();
    ctx.moveCursor(1, 0);
    QPoint p1 = ctx.getCursorPos();
    QCOMPARE(p1.x(), p0.x() + 1);
    ctx.moveCursor(0, 1);
    QPoint p2 = ctx.getCursorPos();
    QCOMPARE(p2.y(), p1.y() + 1);

    // bounds
    ctx.moveCursor(-9999, -9999);
    QPoint p3 = ctx.getCursorPos();
    QVERIFY(p3.x() >= 0 && p3.y() >= 0);
  }

  void test_hjkl_commands_via_command_manager() {
    Context ctx;
    QVERIFY(ctx.init(""));
    // create an empty canvas instead of reading a file
    Canvas c_hjkl;
    c_hjkl.cursor = QPoint(0, 0);
    c_hjkl.blockId = QVector<QVector<quint64>>(
        GetRows(c_hjkl), QVector<quint64>(GetCols(c_hjkl), 0));
    ctx.openCanvasForTest(c_hjkl, true);

    QPoint p0 = ctx.getCursorPos();
    // execute hjkl via CommandManager (use Key enums)
    ctx.executeCommand("MoveDown");
    QCOMPARE(ctx.getCursorPos().x(), p0.x() + 1);
    ctx.executeCommand("MoveUp");
    QCOMPARE(ctx.getCursorPos().x(), p0.x());
    ctx.executeCommand("MoveRight");
    QCOMPARE(ctx.getCursorPos().y(), p0.y() + 1);
    ctx.executeCommand("MoveLeft");
    QCOMPARE(ctx.getCursorPos().y(), p0.y());
  }

  void test_insert_backspace_and_undo_redo() {
    Context ctx;
    QVERIFY(ctx.init(""));
    // create an empty canvas instead of reading a file
    Canvas c_insert;
    c_insert.cursor = QPoint(0, 0);
    c_insert.blockId = QVector<QVector<quint64>>(
        GetRows(c_insert), QVector<quint64>(GetCols(c_insert), 0));
    ctx.openCanvasForTest(c_insert, true);

    // insert characters
    ctx.switchMode(Mode::Insert);
    QPoint pos = ctx.getCursorPos();
    ctx.insertChar(QChar('A'));
    ctx.insertChar(QChar('B'));
    ctx.insertChar(QChar('C'));

    Canvas *c = ctx.getCanvas();
    QVERIFY(c != nullptr);
    // ensure cells present
    QVERIFY(c->blockId[pos.x()][pos.y()] != 0);

    // backspace once
    ctx.deleteChar();
    // after backspace, one of the chars removed
    // undo backspace
    QVERIFY(ctx.undo());
    // redo backspace
    QVERIFY(ctx.redo());
  }

  void test_append_and_open() {
    Context ctx;
    QVERIFY(ctx.init(""));
    Canvas c_ao;
    c_ao.cursor = QPoint(0, 0);
    c_ao.blockId = QVector<QVector<quint64>>(
        GetRows(c_ao), QVector<quint64>(GetCols(c_ao), 0));
    ctx.openCanvasForTest(c_ao, true);

    QPoint before = ctx.getCursorPos();
    ctx.doAppend();
    QCOMPARE(ctx.getMode(), Mode::Insert);
    // cursor should have moved right
    QVERIFY(ctx.getCursorPos().y() >= before.y());

    ctx.switchMode(Mode::Normal);
    ctx.doOpenBelow();
    QCOMPARE(ctx.getMode(), Mode::Insert);
    QVERIFY(ctx.getCursorPos().x() >= before.x());
  }

  void test_word_motions_inside_block() {
    Context ctx;
    QVERIFY(ctx.init(""));
    Canvas c_word;
    c_word.cursor = QPoint(0, 0);
    c_word.blockId = QVector<QVector<quint64>>(
        GetRows(c_word), QVector<quint64>(GetCols(c_word), 0));
    ctx.openCanvasForTest(c_word, true);

    Canvas *c = ctx.getCanvas();
    QVERIFY(c);
    // create a block with multiple cells
    addCellToCanvas(*c, 10, 10, 'x');
    addCellToCanvas(*c, 10, 11, 'y');
    addCellToCanvas(*c, 10, 12, 'z');
    ctx.moveCursorToPos(QPoint(10, 10));
    ctx.moveForward();
    QCOMPARE(ctx.getCursorPos().y(), 11);
    ctx.moveForward();
    QCOMPARE(ctx.getCursorPos().y(), 12);
    ctx.moveBackward();
    QCOMPARE(ctx.getCursorPos().y(), 11);
    ctx.moveEnd();
    QCOMPARE(ctx.getCursorPos().y(), 12);
  }

  void test_key_events_w_e_b() {
    Context ctx;
    QVERIFY(ctx.init(""));
    Canvas c_word;
    c_word.cursor = QPoint(10, 10);
    c_word.blockId = QVector<QVector<quint64>>(GetRows(c_word), QVector<quint64>(GetCols(c_word), 0));
    // build a block with three cells
    addCellToCanvas(c_word, 10, 10, 'x');
    addCellToCanvas(c_word, 10, 11, 'x');
    addCellToCanvas(c_word, 10, 12, 'x');
    c_word.blocks[1];
    ctx.openCanvasForTest(c_word, true);
    ctx.switchMode(Mode::Normal);
    ctx.moveCursorToPos(QPoint(10, 10));

    QKeyEvent evW(QEvent::KeyPress, Qt::Key_W, Qt::NoModifier, QString("w"));
    ctx.onKeyEvent(&evW);
    QCOMPARE(ctx.getCursorPos().y(), 11);

    QKeyEvent evE(QEvent::KeyPress, Qt::Key_E, Qt::NoModifier, QString("e"));
    ctx.onKeyEvent(&evE);
    QCOMPARE(ctx.getCursorPos().y(), 12);

    QKeyEvent evB(QEvent::KeyPress, Qt::Key_B, Qt::NoModifier, QString("b"));
    ctx.onKeyEvent(&evB);
    QCOMPARE(ctx.getCursorPos().y(), 11);
  }

  void test_visual_yank_delete_paste_move_and_undo_redo() {
    Context ctx;
    QVERIFY(ctx.init(""));
    Canvas c_ops;
    c_ops.cursor = QPoint(0, 0);
    c_ops.blockId = QVector<QVector<quint64>>(
        GetRows(c_ops), QVector<quint64>(GetCols(c_ops), 0));
    ctx.openCanvasForTest(c_ops, true);

    Canvas *c = ctx.getCanvas();
    QVERIFY(c);
    // prepare two cells
    addCellToCanvas(*c, 5, 5, 'A');
    addCellToCanvas(*c, 5, 6, 'B');
    ctx.moveCursorToPos(QPoint(5, 5));

    // Lin-mode arbitrary select (select points 5,5 and 5,6)
    ctx.enterSelect();
    ctx.toggleSelectionAt(QPoint(5, 6));
    ctx.doYank();

    // paste at another location
    ctx.moveCursorToPos(QPoint(6, 6));
    ctx.doPasteAfter();
    QVERIFY(c->blockId[6][6] != 0);

    // delete original selection
    ctx.moveCursorToPos(QPoint(5, 5));
    ctx.enterSelect();
    ctx.toggleSelectionAt(QPoint(5, 6));
    ctx.doDelete();
    QCOMPARE((int)c->blockId[5][5], 0);

    // undo delete
    QVERIFY(ctx.undo());
    QVERIFY(c->blockId[5][5] != 0);

    // redo delete
    QVERIFY(ctx.redo());
    QCOMPARE((int)c->blockId[5][5], 0);
  }
};

QTEST_MAIN(TestContext)
#include "Context.moc"
