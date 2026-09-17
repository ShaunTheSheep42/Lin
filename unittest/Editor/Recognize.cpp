#include "Editor/Recognize.h"
#include "Buffer/Block.h"
#include "Editor/InputDirection.h"
#include <QtTest/QtTest>

using namespace Lin;

class TestRecognizer : public QObject {
  Q_OBJECT

private slots:
  void testTokenizeLeft2Right() {
    SimpleRecognizer recognizer;

    Block b;
    Cell c1;
    c1.row = 0;
    c1.col = 0;
    c1.ch = U'H';
    b.cells[{QPoint(0, 0)}] = c1;

    Cell c2;
    c2.row = 0;
    c2.col = 1;
    c2.ch = U'i';
    b.cells[{QPoint(1, 0)}] = c2;

    Cell c3;
    c3.row = 0;
    c3.col = 2;
    c3.ch = U' ';
    b.cells[{QPoint(2, 0)}] = c3;

    Cell c4;
    c4.row = 0;
    c4.col = 3;
    c4.ch = U'!';
    b.cells[{QPoint(3, 0)}] = c4;

    auto tokens = recognizer.tokenize(b);

    QCOMPARE(tokens.size(), 1);
    QCOMPARE(tokens[0].text, QString("Hi"));
    QCOMPARE(tokens[0].startPos, QPoint(0, 0));
    QCOMPARE(tokens[0].endPos, QPoint(1, 0));
  }

  void testTokenizeUp2Down() {
    SimpleRecognizer recognizer;
    recognizer.setInputDirection(Up2Down);

    Block b;
    Cell c1;
    c1.row = 0;
    c1.col = 0;
    c1.ch = U'A';
    b.cells[{QPoint(0, 0)}] = c1;

    Cell c2;
    c2.row = 1;
    c2.col = 0;
    c2.ch = U'B';
    b.cells[{QPoint(0, 1)}] = c2;

    Cell c3;
    c3.row = 2;
    c3.col = 0;
    c3.ch = U'C';
    b.cells[{QPoint(0, 2)}] = c3;

    auto tokens = recognizer.tokenize(b);

    QCOMPARE(tokens.size(), 1);
    QCOMPARE(tokens[0].text, QString("ABC"));
    QCOMPARE(tokens[0].startPos, QPoint(0, 0));
    QCOMPARE(tokens[0].endPos, QPoint(0, 2));
  }
};

QTEST_MAIN(TestRecognizer)
#include "Recognize.moc"
