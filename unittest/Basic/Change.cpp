#include "Basic/Change.h"
#include "Basic/Block.h"
#include <QtTest/QtTest>

using namespace Lin;

class TestChange : public QObject {
  Q_OBJECT

private slots:
  void testEncodeDecodeAddWithNewBlock() {
    Block b(QPoint(1, 1), {{U'A', U'B'}});
    Change ch{Change::Add, std::nullopt, b};
    QJsonObject obj = EncodeChange(ch);
    Change decoded = DecodeChange(obj);

    QCOMPARE(decoded.op, Change::Add);
    QVERIFY(!decoded.oldBlock.has_value());
    QVERIFY(decoded.newBlock.has_value());
    QCOMPARE(decoded.newBlock->toString(), QString("AB"));
  }

  void testEncodeDecodeUpdateWithOldAndNewBlock() {
    Block oldB(QPoint(1, 1), {{U'A'}});
    Block newB(QPoint(1, 1), {{U'B'}});
    Change ch{Change::Update, oldB, newB};
    QJsonObject obj = EncodeChange(ch);
    Change decoded = DecodeChange(obj);

    QCOMPARE(decoded.op, Change::Update);
    QVERIFY(decoded.oldBlock.has_value());
    QVERIFY(decoded.newBlock.has_value());
    QCOMPARE(decoded.oldBlock->toString(), QString("A"));
    QCOMPARE(decoded.newBlock->toString(), QString("B"));
  }

  void testEncodeDecodeDeleteWithOldBlock() {
    Block b(QPoint(2, 2), {{U'字', U'X'}});
    Change ch{Change::Delete, b, std::nullopt};
    QJsonObject obj = EncodeChange(ch);
    Change decoded = DecodeChange(obj);

    QCOMPARE(decoded.op, Change::Delete);
    QVERIFY(decoded.oldBlock.has_value());
    QVERIFY(!decoded.newBlock.has_value());
    QCOMPARE(decoded.oldBlock->toString(), QString("字X"));
  }
};

QTEST_MAIN(TestChange)
#include "Change.moc"
