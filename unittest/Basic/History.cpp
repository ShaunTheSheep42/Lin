#include <catch2/catch_all.hpp>

#include <memory>
#include <string>
#include <vector>

import Basic.Canvas;
import Basic.Geometry;
import Basic.History;
import Basic.Text;

namespace Lin {

TEST_CASE("History applies batch changes forward and backward in order",
          "[history]") {
  Canvas canvas;
  auto first = std::make_unique<Text>(Position{0, 0});
  first->insertString({0, 0}, U"one");
  const uint64_t firstId = first->getId();
  auto second = std::make_unique<Text>(Position{0, 1});
  second->insertString({0, 1}, U"two");
  const uint64_t secondId = second->getId();

  History history;
  std::vector<Change> changes;
  changes.push_back(Change::Added(*first));
  changes.push_back(Change::Added(*second));
  history.pushChange(Change::Batch(std::move(changes)));
  canvas.addBlock(std::move(first));
  canvas.addBlock(std::move(second));

  REQUIRE(history.undo(canvas));
  REQUIRE_FALSE(canvas.haveBlocks());
  REQUIRE(history.redo(canvas));
  REQUIRE(canvas.getBlock(firstId) != nullptr);
  REQUIRE(canvas.getBlock(secondId) != nullptr);
  REQUIRE_FALSE(history.redo(canvas));

  std::string json;
  REQUIRE_FALSE(EncodeHistory(history, json));
  History restored;
  REQUIRE_FALSE(DecodeHistory(json, restored));
  REQUIRE(restored.undo(canvas));
  REQUIRE_FALSE(canvas.haveBlocks());
  REQUIRE(restored.redo(canvas));
  REQUIRE(canvas.getBlock(firstId) != nullptr);
  REQUIRE(canvas.getBlock(secondId) != nullptr);
}

} // namespace Lin
