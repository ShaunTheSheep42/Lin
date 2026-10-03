#include <catch2/catch_all.hpp>

#include <memory>

import Basic.Canvas;
import Basic.Geometry;
import Basic.Text;
import Basic.ViewPort;

namespace Lin {

TEST_CASE("ViewPort follows cursor and maintains viewport dimensions",
          "[viewport]") {
  Canvas canvas;
  ViewPort view(canvas, {10, 5});

  REQUIRE(view.getCursorPosition() == Position{0, 0});
  REQUIRE(view.getSize() == Size{10, 5});
  REQUIRE(view.contains({0, 0}));
  REQUIRE_FALSE(view.contains({10, 0}));

  view.moveCursorPosition(12, 6);
  REQUIRE(view.getCursorPosition() == Position{12, 6});
  REQUIRE(view.getTopLeft() == Position{3, 2});
  REQUIRE(view.contains({12, 6}));

  view.setCursorPosition({-2, -3});
  REQUIRE(view.getTopLeft() == Position{-2, -3});
  view.setTopLeft({20, 30});
  REQUIRE(view.getTopLeft() == Position{20, 30});
  REQUIRE_FALSE(view.contains(view.getCursorPosition()));
}

TEST_CASE("ViewPort queries block and group IDs from canvas hit tests",
          "[viewport]") {
  Canvas canvas;
  auto text = std::make_unique<Text>(Position{5, 7}, 42);
  text->insertString({5, 7}, U"abc");
  const uint64_t id = text->getId();
  canvas.addBlock(std::move(text));
  ViewPort view(canvas, {20, 10});

  view.setCursorPosition({6, 7});
  REQUIRE(view.getCursorPositionBlockId() == id);
  REQUIRE(view.getCursorPositionGroupId() == 42);
  REQUIRE(view.getBlockId({5, 7}) == id);
  REQUIRE(view.getGroupId({5, 7}) == 42);
  REQUIRE(view.getBlockId({100, 100}) == 0);
  REQUIRE(view.getGroupId({100, 100}) == 0);
}

} // namespace Lin
