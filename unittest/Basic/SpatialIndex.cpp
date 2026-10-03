#include <catch2/catch_all.hpp>

#include <cstdint>
#include <memory>
#include <vector>

import Basic.Block;
import Basic.Canvas;
import Basic.Geometry;
import Basic.SpatialIndex;

namespace Lin {
namespace {

class IndexTestBlock : public Block {
public:
  IndexTestBlock(uint64_t id, Position anchor) : Block(anchor, 0) {
    this->id = id;
  }

  std::unique_ptr<Block> clone() const override {
    return std::make_unique<IndexTestBlock>(*this);
  }

  bool hitTest(Position) const override { return false; }
};

} // namespace

TEST_CASE("SpatialIndex finds anchors within one chunk", "[spatial-index]") {
  SpatialIndex index;
  index.insert(1, {2, 3});
  index.insert(2, {10, 20});
  index.insert(3, {31, 31});

  REQUIRE(index.query({0, 0}, {31, 31}) == std::vector<uint64_t>{1, 2, 3});
  REQUIRE(index.query({3, 3}, {9, 9}).empty());
  REQUIRE(index.query({2, 3}, {2, 3}) == std::vector<uint64_t>{1});
}

TEST_CASE("SpatialIndex handles chunk boundaries and exact rectangle filtering",
          "[spatial-index]") {
  SpatialIndex index;
  index.insert(1, {31, 0});
  index.insert(2, {32, 0});
  index.insert(3, {63, 0});
  index.insert(4, {64, 0});

  REQUIRE(index.query({31, 0}, {32, 0}) == std::vector<uint64_t>{1, 2});
  REQUIRE(index.query({32, 0}, {63, 0}) == std::vector<uint64_t>{2, 3});
  REQUIRE(index.query({32, 0}, {32, 0}) == std::vector<uint64_t>{2});
}

TEST_CASE("SpatialIndex uses floor-based chunks for negative positions",
          "[spatial-index]") {
  SpatialIndex index;
  index.insert(1, {-1, -1});
  index.insert(2, {-32, -32});
  index.insert(3, {-33, -33});
  index.insert(4, {0, 0});

  REQUIRE(index.query({-32, -32}, {-1, -1}) == std::vector<uint64_t>{1, 2});
  REQUIRE(index.query({-33, -33}, {-33, -33}) == std::vector<uint64_t>{3});
  REQUIRE(index.query({0, 0}, {0, 0}) == std::vector<uint64_t>{4});
}

TEST_CASE("SpatialIndex updates duplicate IDs without stale entries",
          "[spatial-index]") {
  SpatialIndex index;
  index.insert(7, {1, 1});
  index.insert(7, {40, 1});

  REQUIRE(index.query({0, 0}, {31, 31}).empty());
  REQUIRE(index.query({32, 0}, {63, 31}) == std::vector<uint64_t>{7});
}

TEST_CASE("SpatialIndex removes and moves block IDs", "[spatial-index]") {
  SpatialIndex index;
  index.insert(1, {1, 1});
  index.insert(2, {33, 1});

  index.remove(1, {2, 1});
  REQUIRE(index.query({0, 0}, {31, 31}) == std::vector<uint64_t>{1});

  index.move(1, {1, 1}, {-1, -1});
  REQUIRE(index.query({0, 0}, {31, 31}).empty());
  REQUIRE(index.query({-1, -1}, {-1, -1}) == std::vector<uint64_t>{1});
  REQUIRE(index.query({32, 0}, {63, 31}) == std::vector<uint64_t>{2});

  index.remove(1, {-1, -1});
  REQUIRE(index.query({-1, -1}, {-1, -1}).empty());
}

TEST_CASE("SpatialIndex can move an unindexed ID and clear all data",
          "[spatial-index]") {
  SpatialIndex index;

  index.move(5, {100, 100}, {5, 6});
  REQUIRE(index.query({5, 6}, {5, 6}) == std::vector<uint64_t>{5});

  index.clear();
  REQUIRE(index.query({-100, -100}, {100, 100}).empty());
}

TEST_CASE("SpatialIndex rebuild replaces its contents from block anchors",
          "[spatial-index]") {
  Canvas canvas;
  canvas.addBlock(std::make_unique<IndexTestBlock>(11, Position{4, 7}));
  canvas.addBlock(std::make_unique<IndexTestBlock>(12, Position{-33, 50}));
  SpatialIndex index;
  index.insert(99, {0, 0});

  index.rebuild(canvas.getBlockAnchors());

  REQUIRE(index.query({4, 7}, {4, 7}) == std::vector<uint64_t>{11});
  REQUIRE(index.query({-33, 50}, {-33, 50}) == std::vector<uint64_t>{12});
  REQUIRE(index.query({0, 0}, {0, 0}).empty());
}

} // namespace Lin
