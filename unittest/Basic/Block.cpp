#include "Basic/Block.h"
#include "Basic/Char.h"
#include "Basic/Cursor.h"
#include <QtTest/QtTest>

using namespace Lin;

class TestBlock : public QObject {
  Q_OBJECT

private slots:
  void testCreateIDUnique() {
    Block b1(QPoint(1, 1), {});
    Block b2(QPoint(1, 1), {});
    Block b3(QPoint(1, 1), {});
    QVERIFY(b2.getId() > b1.getId());
    QVERIFY(b3.getId() > b2.getId());
  }

  void testUpdateCharNormal() {
    Char ca = MakeChar(U'A');
    Char cb = MakeChar(U'B');
    Block b(QPoint(1, 1), {{ca}});
    b.updateChar(QPoint(1, 1), cb);
    QCOMPARE(b.toString(), QString("B"));
  }

  void testUpdateCharDoubleWidthInsertPlaceholder() {
    Char ca = MakeChar(U'A');
    Block b(QPoint(1, 1), {{ca}});
    b.updateChar(QPoint(1, 1), MakeChar(U'字', true));
    QVERIFY(IsDoubleWidth(b.getText()[0][0]));
    QVERIFY(IsPlaceHolder(b.getText()[0][1]));
  }

  void testUpdateCharReplaceDoubleWidthWithNormal() {
    Block b(QPoint(1, 1), {{MakeChar(U'字', true), PlaceHolder}});
    b.updateChar(QPoint(1, 1), U'C');
    QCOMPARE(b.getText()[0].size(), 1);
    QCOMPARE(b.toString(), QString("C"));
  }

  void testUpdateCharReplacePlaceholder() {
    Block b(QPoint(1, 1), {{MakeChar(U'字', true), PlaceHolder}});
    b.updateChar(QPoint(2, 1), MakeChar(U'D'));
    QCOMPARE(b.getText()[0].size(), 1);
    QCOMPARE(b.toString(), QString("D"));
  }

  void testUpdateCharInsertNewLineNormal() {
    Block b(QPoint(1, 1), {{U'A', U'B', U'C'}});
    b.updateChar(QPoint(2, 1), U'\n');
    QCOMPARE(b.getText().size(), 2);
    QCOMPARE(b.getText()[1][0], U'C');
  }

  void testUpdateCharInsertNewLineDoubleWidth() {
    Block b(QPoint(1, 1), {{MakeChar(U'字', true), PlaceHolder, U'X'}});
    b.updateChar(QPoint(1, 1), U'\n');
    QCOMPARE(b.getText().size(), 1);
    QCOMPARE(b.getText()[0][0], U'X');
  }

  void testInsertCharNormal() {
    Block b(QPoint(1, 1), {{U'A', U'B'}});
    b.insertChar(QPoint(2, 1), U'C');
    QCOMPARE(b.toString(), QString("ACB"));
  }

  void testInsertCharDoubleWidth() {
    Block b(QPoint(1, 1), {{U'A'}});
    b.insertChar(QPoint(1, 1), MakeChar(U'字', true), Backward);
    QVERIFY(IsDoubleWidth(b.getText()[0][1]));
    QVERIFY(IsPlaceHolder(b.getText()[0][2]));
  }

  void testInsertCharNewLine() {
    Block b(QPoint(1, 1), {{U'A', U'B'}});
    b.insertChar(QPoint(2, 1), U'\n');
    QCOMPARE(b.getText().size(), 2);
    QCOMPARE(b.getText()[1][0], U'B');
  }

  void testDeleteCharNormal() {
    Block b(QPoint(1, 1), {{U'A', U'B'}});
    b.deleteChar(QPoint(2, 1));
    QCOMPARE(b.toString(), QString("A"));
  }

  void testDeleteCharDoubleWidth() {
    Block b(QPoint(1, 1), {{MakeChar(U'字', true), PlaceHolder, U'X'}});
    b.deleteChar(QPoint(1, 1));
    QCOMPARE(b.getText()[0].size(), 1);
    QCOMPARE(b.toString(), QString("X"));
  }

  void testDeleteCharPlaceholder() {
    Block b(QPoint(1, 1), {{MakeChar(U'字', true), PlaceHolder}});
    b.deleteChar(QPoint(2, 1));
    QCOMPARE(b.getText().size(), 0);
  }

  void testEncodeDecodeComplex() {
    Block b(QPoint(1, 1),
            {{U'A'}, {MakeChar(U'字', true), PlaceHolder, U'😀'}});
    QJsonObject obj = Encode(b);
    Block decoded = Decode(obj);
    QCOMPARE(decoded.toString(), QString("A\n字😀"));
  }
};

QTEST_MAIN(TestBlock)
#include "Block.moc"
