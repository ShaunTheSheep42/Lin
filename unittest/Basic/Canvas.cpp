#include "Basic/Canvas.h"
#include "Basic/Block.h"
#include "Basic/Char.h"
#include <QtTest/QtTest>

using namespace Lin;

class TestCanvas : public QObject {
  Q_OBJECT

private slots:
  void testAddUpdateDeleteBlock() {
    Canvas canvas("path");
    Block b(QPoint(1, 1), {{U'A'}});
    canvas.addBlock(b);
    QCOMPARE(canvas.getBlock(b.getId())->toString(), QString("A"));

    Block newBlock = b;
    newBlock.updateChar(QPoint(1, 1), MakeChar(U'B'));
    canvas.updateBlock(b.getId(), newBlock);
    QCOMPARE(canvas.getBlock(b.getId())->toString(), QString("B"));

    canvas.deleteBlock(b.getId());
    QCOMPARE(canvas.getBlocks().size(), 0u);
  }

  void testGetBlockByPointAndGroupPropagation() {
    Canvas canvas("point");
    Block b(QPoint(1, 1), {{U'C'}});
    canvas.addBlock(b);

    const Block *byPoint = canvas.getBlock(QPoint(1, 1));
    QVERIFY(byPoint != nullptr);
    QCOMPARE(byPoint->toString(), QString("C"));

    // After addBlock, updateGroupId() is called; adjacent empty cells should be
    // filled by BFS propagation
    QCOMPARE(canvas.getGroupId(QPoint(1, 1)), b.getGroupId());

    // Check propagation to a neighboring empty cell (in-bounds)
    QPoint neighbor(1, 2);
    QVERIFY(neighbor.x() >= 1 && neighbor.x() <= Canvas::Width);
    QVERIFY(neighbor.y() >= 1 && neighbor.y() <= Canvas::Height);
    QCOMPARE(canvas.getGroupId(neighbor), b.getGroupId());
  }

  void testCursorMovement() {
    Canvas canvas("cursor");
    canvas.moveCursorToPos(QPoint(10, 20));
    QCOMPARE(canvas.getCursorPos(), QPoint(10, 20));

    canvas.moveCursorToPos(QPoint(-5, -5));
    QCOMPARE(canvas.getCursorPos(), QPoint(1, 1));

    canvas.moveCursorToPos(QPoint(Canvas::Width + 10, Canvas::Height + 10));
    QCOMPARE(canvas.getCursorPos(), QPoint(Canvas::Width, Canvas::Height));

    canvas.moveCursorByOffSet(5, 5);
    QCOMPARE(canvas.getCursorPos(), QPoint(Canvas::Width, Canvas::Height));

    canvas.moveCursorByOffSet(-999, -999);
    QCOMPARE(canvas.getCursorPos(), QPoint(1, 1));
  }

  void testReset() {
    Canvas canvas("reset");
    Block b(QPoint(1, 1), {{U'D'}});
    canvas.addBlock(b);
    canvas.moveCursorToPos(QPoint(10, 10));
    canvas.reset();

    QCOMPARE(canvas.getCursorPos(),
             QPoint(Canvas::Width / 2, Canvas::Height / 2));
    QCOMPARE(canvas.getBlocks().size(), 0u);

    // After reset, blockId and groupId should be zero at some sample positions
    QCOMPARE(canvas.getBlockId(QPoint(1, 1)), static_cast<quint64>(0));
    QCOMPARE(canvas.getGroupId(QPoint(1, 1)), static_cast<quint64>(0));
  }

  void testEncodeDecodeValid() {
    Canvas canvas("encode");
    Block b(QPoint(1, 1), {{U'E'}});
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    const Block *blk = decoded.getBlock(b.getId());
    QVERIFY(blk != nullptr);
    QCOMPARE(blk->toString(), QString("E"));

    // Path used by Decode is the encoded string as path; ensure original path
    // is preserved only in original
    QCOMPARE(canvas.getPath(), QString("encode"));
  }

  void testEncodeDecodeEmptyCanvas() {
    Canvas canvas("empty");
    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    QCOMPARE(decoded.getCursorPos(),
             QPoint(Canvas::Width / 2, Canvas::Height / 2));
    // No blocks present
    QCOMPARE(decoded.getBlocks().size(), 0u);
  }

  void testDecodeInvalidInput() {
    auto decodedOpt1 = Decode("");
    QVERIFY(!decodedOpt1.has_value());
    auto decodedOpt2 = Decode("not a json");
    QVERIFY(!decodedOpt2.has_value());
  }

  void testStressEncodeDecodeLargeCanvas() {
    Canvas canvas("stress");
    std::vector<std::vector<Char>> bigText;
    for (int r = 1; r <= Canvas::Height; ++r) {
      std::vector<Char> line;
      for (int c = 1; c <= Canvas::Width; ++c)
        line.push_back((r + c) % 2 == 0 ? U'A' : U'字');
      bigText.push_back(line);
    }
    Block b(QPoint(1, 1), bigText);
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    const Block *decodedBlock = decoded.getBlock(b.getId());
    QVERIFY(decodedBlock != nullptr);
    QCOMPARE(static_cast<int>(decodedBlock->getText().size()), Canvas::Height);

    // Check a couple of characters to ensure pattern preserved
    QCOMPARE(decodedBlock->getText()[0][0], U'A');
    QCOMPARE(decodedBlock->getText()[0][1], U'字');
  }

  void testMultipleBlocks() {
    Canvas canvas("multi");
    Block b1(QPoint(1, 1), {{U'X'}});
    Block b2(QPoint(5, 5), {{U'Y'}});
    Block b3(QPoint(10, 10), {{U'Z'}});
    canvas.addBlock(b1);
    canvas.addBlock(b2);
    canvas.addBlock(b3);

    QCOMPARE(canvas.getBlock(b1.getId())->toString(), QString("X"));
    QCOMPARE(canvas.getBlock(b2.getId())->toString(), QString("Y"));
    QCOMPARE(canvas.getBlock(b3.getId())->toString(), QString("Z"));

    // Ensure blockId matrix contains their ids at anchors
    QCOMPARE(canvas.getBlockId(QPoint(1, 1)), b1.getId());
    QCOMPARE(canvas.getBlockId(QPoint(5, 5)), b2.getId());
    QCOMPARE(canvas.getBlockId(QPoint(10, 10)), b3.getId());
  }

  void testUndoRedo() {
    Canvas canvas("undo");
    Block b(QPoint(1, 1), {{U'U'}});
    canvas.addBlock(b);
    QVERIFY(canvas.canUndo());
    canvas.undo();
    QCOMPARE(canvas.getBlocks().size(), 0u);
    QVERIFY(canvas.canRedo());
    canvas.redo();
    QCOMPARE(canvas.getBlocks().size(), 1u);
  }

  void testUndoRedoEmptyHistory() {
    Canvas canvas("undoempty");
    QVERIFY(!canvas.canUndo());
    QVERIFY(!canvas.canRedo());
  }

  void testGetCharEmptyCell() {
    // getChar calls getBlock which will qFatal on out-of-bounds.
    // Test an in-bounds empty cell returns 0.
    Canvas canvas("char");
    Block b(QPoint(1, 1), {{U'H'}});
    canvas.addBlock(b);

    QPoint emptyCell(2, 2);
    QVERIFY(emptyCell.x() >= 1 && emptyCell.x() <= Canvas::Width);
    QVERIFY(emptyCell.y() >= 1 && emptyCell.y() <= Canvas::Height);
    QCOMPARE(canvas.getChar(emptyCell), (Char)0);
  }

  void testEncodeDecodeSpecialChars() {
    Canvas canvas("special");
    Block b(QPoint(1, 1), {{U'😀'}});
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    QCOMPARE(decoded.getBlock(b.getId())->toString(), QString("😀"));
  }

  void testRejectEmptyBlock() {
    Canvas canvas("emptyblock");
    Block b(QPoint(1, 1), {});
    canvas.addBlock(b);
    QVERIFY(!canvas.haveBlocks());
    QVERIFY(canvas.getBlock(b.getId()) == nullptr);
  }

  void testEncodeDecodeComplexText() {
    Canvas canvas("complex");
    Block b(QPoint(1, 1), {{{U'A', U'字'}, {U'😀', U'B'}}});
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    QString s = decoded.getBlock(b.getId())->toString();
    QVERIFY(s.contains("A"));
    QVERIFY(s.contains("字"));
    QVERIFY(s.contains("😀"));
    QVERIFY(s.contains("B"));
  }

  void testEncodeDecodeMultiLineText() {
    Canvas canvas("multiline");
    Block b(QPoint(1, 1), {{{U'A', U'B'}, {U'C', U'D'}}});
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    QCOMPARE(decoded.getBlock(b.getId())->toString(), QString("AB\nCD"));
  }

  void testStressMultipleBlocks() {
    Canvas canvas("stressmulti");
    for (int i = 0; i < 30; ++i) {
      Block b(QPoint(i + 1, i + 1), {{U'A'}});
      canvas.addBlock(b);
    }
    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());
    QCOMPARE(static_cast<int>(decoded.getBlocks().size()), 30);
  }

  void testCursorExtremeMoves() {
    Canvas canvas("extreme");
    canvas.moveCursorToPos(QPoint(1, 1));
    canvas.moveCursorByOffSet(10000, 10000);
    QCOMPARE(canvas.getCursorPos(), QPoint(Canvas::Width, Canvas::Height));
    canvas.moveCursorByOffSet(-10000, -10000);
    QCOMPARE(canvas.getCursorPos(), QPoint(1, 1));
  }

  void testEncodeDecodeWithSemantics() {
    Canvas canvas("semantics");
    Block b(QPoint(1, 1), {{U'S'}});
    b.setSemantics("python code");
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    QCOMPARE(decoded.getBlock(b.getId())->getSemantics(),
             QString("python code"));
  }

  void testEncodeDecodeWithGroupId() {
    Canvas canvas("group");
    Block b(QPoint(1, 1), {{U'G'}});
    quint64 gid = b.getGroupId();
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    QCOMPARE(decoded.getBlock(b.getId())->getGroupId(), gid);
  }

  void testEncodeDecodeWithAnchor() {
    Canvas canvas("anchor");
    Block b(QPoint(5, 7), {{U'A'}});
    canvas.addBlock(b);

    QString encoded = Encode(canvas);
    auto decodedOpt = Decode(encoded);
    QVERIFY(decodedOpt.has_value());
    Canvas decoded = std::move(decodedOpt.value());

    QCOMPARE(decoded.getBlock(b.getId())->getAnchor(), QPoint(5, 7));
  }

  void testApplyChangeForwardBackward() {
    Canvas canvas("applychange");
    Block b(QPoint(1, 1), {{U'W'}});
    canvas.addBlock(b);
    canvas.undo(); // backward delete
    QCOMPARE(canvas.getBlocks().size(), 0u);
    canvas.redo(); // forward add
    QCOMPARE(canvas.getBlocks().size(), 1u);
  }

  // New tests to cover additional branches and logic
  void testCanPlaceConflictAndSelfOverlap() {
    Canvas canvas("canplace");
    Block b1(QPoint(2, 2), {{U'A', U'B'}});
    canvas.addBlock(b1);

    // Another block overlapping b1 should be rejected
    Block b2(QPoint(2, 2), {{U'C'}});
    QVERIFY(!canvas.canPlace(b2));

    // A block with same id as existing block should be allowed to "overlap"
    // (update itself)
    Block b1Updated = b1;
    b1Updated.updateChar(QPoint(2, 2), MakeChar(U'Z'));
    // canPlace should allow because existingId == b1.getId()
    QVERIFY(canvas.canPlace(b1Updated));
  }

  void testHistoryRedoClearedWhenNewOpAfterUndo() {
    Canvas canvas("history");
    Block b1(QPoint(1, 1), {{U'1'}});
    Block b2(QPoint(2, 2), {{U'2'}});
    canvas.addBlock(b1);
    canvas.addBlock(b2);

    // Now undo last add
    canvas.undo();
    QVERIFY(canvas.canRedo());

    // Add a new block after undo; this should clear redo history
    Block b3(QPoint(3, 3), {{U'3'}});
    canvas.addBlock(b3);
    QVERIFY(!canvas.canRedo());

    // Undo twice should remove two blocks (b3 and b1)
    QVERIFY(canvas.canUndo());
    canvas.undo(); // removes b3
    QVERIFY(canvas.canUndo());
    canvas.undo(); // removes b1
    QVERIFY(!canvas.canUndo());
  }

  void testUpdateGroupIdMultipleSeedsPropagation() {
    Canvas canvas("groupprop");
    // Place two blocks with different group ids separated by empty space
    Block left(QPoint(1, 1), {{U'L'}});
    Block right(QPoint(5, 1), {{U'R'}});
    canvas.addBlock(left);
    canvas.addBlock(right);

    // After updateGroupId, cells adjacent to each block should have
    // corresponding gid
    QCOMPARE(canvas.getGroupId(QPoint(1, 1)), left.getGroupId());
    QCOMPARE(canvas.getGroupId(QPoint(5, 1)), right.getGroupId());

    // Check a cell closer to left seed than right seed (e.g., (2,1)) gets left
    // gid
    QCOMPARE(canvas.getGroupId(QPoint(2, 1)), left.getGroupId());

    // Check a cell closer to right seed (e.g., (4,1)) gets right gid
    QCOMPARE(canvas.getGroupId(QPoint(4, 1)), right.getGroupId());
  }
};

QTEST_MAIN(TestCanvas)
#include "Canvas.moc"
