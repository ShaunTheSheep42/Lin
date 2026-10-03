#include <catch2/catch_all.hpp>

#include <cstdint>
#include <memory>
#include <string>

import Basic.Block;
import Basic.Canvas;
import Basic.Geometry;
import Basic.Text;

namespace Lin {
namespace {

class TestBlock : public Block {
public:
  TestBlock(uint64_t groupId, bool empty = false)
      : Block({0, 0}, groupId), empty(empty) {}

  TestBlock(uint64_t id, uint64_t groupId, bool empty)
      : Block({0, 0}, groupId), empty(empty) {
    this->id = id;
  }

  std::unique_ptr<Block> clone() const override {
    return std::make_unique<TestBlock>(*this);
  }

  bool hitTest(Position) const override { return true; }
  bool isEmpty() const override { return empty; }

private:
  bool empty;
};

std::unique_ptr<Block> MakeBlock(uint64_t groupId, bool empty = false) {
  return std::make_unique<TestBlock>(groupId, empty);
}

} // namespace

TEST_CASE("Canvas starts empty and reset clears all blocks", "[canvas]") {
  Canvas canvas;

  REQUIRE_FALSE(canvas.haveBlocks());
  REQUIRE(canvas.getBlockAnchors().empty());
  REQUIRE(canvas.getBlock(123) == nullptr);

  auto block = MakeBlock(1);
  const auto id = block->getId();
  canvas.addBlock(std::move(block));

  REQUIRE(canvas.haveBlocks());
  REQUIRE(canvas.getBlock(id) != nullptr);

  canvas.reset();

  REQUIRE_FALSE(canvas.haveBlocks());
  REQUIRE(canvas.getBlock(id) == nullptr);
}

TEST_CASE("Canvas adds blocks and exposes non-owning block lookup",
          "[canvas]") {
  Canvas canvas;
  auto block = MakeBlock(7);
  const auto id = block->getId();
  const auto groupId = block->getGroupId();

  canvas.addBlock(std::move(block));

  REQUIRE(canvas.haveBlocks());
  REQUIRE(canvas.getBlock(id) != nullptr);
  REQUIRE(canvas.getBlock(id)->getId() == id);
  REQUIRE(canvas.getBlock(id)->getGroupId() == groupId);
  REQUIRE(canvas.getBlockAnchors().size() == 1);
  REQUIRE(canvas.getBlockAnchors().front().first == id);
  REQUIRE(canvas.getBlockAnchors().front().second == Position{0, 0});
}

TEST_CASE("Canvas ignores empty blocks and deletes known or unknown IDs",
          "[canvas]") {
  Canvas canvas;
  auto empty = MakeBlock(1, true);
  const auto emptyId = empty->getId();
  canvas.addBlock(std::move(empty));

  REQUIRE_FALSE(canvas.haveBlocks());
  REQUIRE(canvas.getBlock(emptyId) == nullptr);

  auto block = MakeBlock(2);
  const auto id = block->getId();
  canvas.addBlock(std::move(block));
  canvas.deleteBlock(999);
  REQUIRE(canvas.getBlock(id) != nullptr);

  canvas.deleteBlock(id);
  REQUIRE_FALSE(canvas.haveBlocks());
  REQUIRE(canvas.getBlock(id) == nullptr);
}

TEST_CASE("Canvas replaces matching block identity and removes empty update",
          "[canvas]") {
  Canvas canvas;
  auto original = MakeBlock(10);
  const auto id = original->getId();
  const auto groupId = original->getGroupId();
  canvas.addBlock(std::move(original));

  canvas.updateBlock(id, std::make_unique<TestBlock>(id, groupId, false));
  REQUIRE(canvas.getBlock(id) != nullptr);
  REQUIRE(canvas.getBlock(id)->getId() == id);

  canvas.updateBlock(id, std::make_unique<TestBlock>(id, groupId, true));
  REQUIRE(canvas.getBlock(id) == nullptr);
  REQUIRE_FALSE(canvas.haveBlocks());
}

TEST_CASE("Canvas does not add a replacement for a missing block ID",
          "[canvas]") {
  Canvas canvas;
  auto replacement = MakeBlock(5);
  const auto id = replacement->getId();

  canvas.updateBlock(id, std::move(replacement));

  REQUIRE_FALSE(canvas.haveBlocks());
  REQUIRE(canvas.getBlock(id) == nullptr);
}

TEST_CASE("Canvas JSON round trip preserves block identity and content",
          "[canvas][json]") {
  Canvas canvas;
  auto text = std::make_unique<Text>(Position{-4, 9}, 12);
  text->insertString({-4, 9}, U"wide界");
  const auto id = text->getId();
  canvas.addBlock(std::move(text));

  std::string json;
  REQUIRE_FALSE(EncodeCanvas(canvas, json));

  Canvas decoded;
  REQUIRE_FALSE(DecodeCanvas(json, decoded));
  const auto *restored = dynamic_cast<const Text *>(decoded.getBlock(id));
  REQUIRE(restored != nullptr);
  REQUIRE(restored->toString() == U"wide界");
  REQUIRE(restored->getAnchor() == Position{-4, 9});
  REQUIRE(restored->getGroupId() == 12);
}

} // namespace Lin
