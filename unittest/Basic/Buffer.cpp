#include <catch2/catch_all.hpp>

#include <filesystem>
#include <memory>
#include <string>

import Basic.Buffer;
import Basic.Block;
import Basic.Char;
import Basic.Geometry;
import Basic.Text;

namespace Lin {

TEST_CASE("Buffer tracks block addition and deletion through undo and redo",
          "[buffer][history]") {
  Buffer buffer("sample.lin");
  auto text = std::make_unique<Text>(Position{4, 2});
  text->insertString({4, 2}, U"first");
  const uint64_t id = text->getId();

  REQUIRE(buffer.getPath() == "sample.lin");
  REQUIRE_FALSE(buffer.canUndo());
  buffer.addBlock(std::move(text));
  REQUIRE(buffer.canUndo());
  REQUIRE(buffer.getCanvas().getBlock(id) != nullptr);

  REQUIRE(buffer.undo());
  REQUIRE(buffer.getCanvas().getBlock(id) == nullptr);
  REQUIRE(buffer.canRedo());
  REQUIRE(buffer.redo());
  REQUIRE(buffer.getCanvas().getBlock(id) != nullptr);

  buffer.deleteBlock(id);
  REQUIRE(buffer.getCanvas().getBlock(id) == nullptr);
  REQUIRE(buffer.undo());
  REQUIRE(buffer.getCanvas().getBlock(id) != nullptr);
  REQUIRE(buffer.getCanvas().getBlock(id)->getId() == id);
}

TEST_CASE("Buffer undo restores previous block content and truncates redo",
          "[buffer][history]") {
  Buffer buffer("");
  auto original = std::make_unique<Text>(Position{0, 0});
  original->insertString({0, 0}, U"old");
  const uint64_t id = original->getId();
  buffer.addBlock(std::move(original));

  REQUIRE(buffer.editBlock(id, [](Block &block) {
    auto &text = dynamic_cast<Text &>(block);
    text.updateChar({0, 0}, MakeChar(U'n'));
    text.updateChar({1, 0}, MakeChar(U'e'));
    text.updateChar({2, 0}, MakeChar(U'w'));
  }));
  REQUIRE(
      dynamic_cast<const Text *>(buffer.getCanvas().getBlock(id))->toString() ==
      U"new");

  REQUIRE(buffer.undo());
  REQUIRE(
      dynamic_cast<const Text *>(buffer.getCanvas().getBlock(id))->toString() ==
      U"old");
  REQUIRE(buffer.redo());
  REQUIRE(
      dynamic_cast<const Text *>(buffer.getCanvas().getBlock(id))->toString() ==
      U"new");

  REQUIRE(buffer.undo());
  REQUIRE(buffer.editBlock(id, [](Block &block) {
    auto &text = dynamic_cast<Text &>(block);
    text.updateChar({0, 0}, MakeChar(U'o'));
    text.updateChar({1, 0}, MakeChar(U't'));
    text.updateChar({2, 0}, MakeChar(U'h'));
  }));
  REQUIRE_FALSE(buffer.canRedo());
  REQUIRE(
      dynamic_cast<const Text *>(buffer.getCanvas().getBlock(id))->toString() ==
      U"oth");
}

TEST_CASE("Buffer JSON and file round trips preserve document and undo history",
          "[buffer][json]") {
  const auto path =
      std::filesystem::temp_directory_path() / "lin-buffer-json-test.json";
  std::filesystem::remove(path);

  Buffer source(path.string());
  auto text = std::make_unique<Text>(Position{7, -3}, 91);
  text->insertString({7, -3}, U"A界\nZ");
  const auto id = text->getId();
  source.addBlock(std::move(text));

  std::string json;
  REQUIRE_FALSE(EncodeBuffer(source, json));
  REQUIRE(json.find("\"canvas\":{\"blocks\":[") != std::string::npos);
  REQUIRE(json.find("\"history\":{\"entries\":[") != std::string::npos);

  Buffer decoded("decoded.lin");
  REQUIRE_FALSE(DecodeBuffer(json, decoded));
  const auto *decodedText =
      dynamic_cast<const Text *>(decoded.getCanvas().getBlock(id));
  REQUIRE(decodedText != nullptr);
  REQUIRE(decodedText->getAnchor() == Position{7, -3});
  REQUIRE(decodedText->getGroupId() == 91);
  REQUIRE(decodedText->toString() == U"A界\nZ");
  REQUIRE(decoded.canUndo());
  REQUIRE(decoded.undo());
  REQUIRE_FALSE(decoded.getCanvas().haveBlocks());
  REQUIRE(decoded.redo());
  REQUIRE(decoded.getCanvas().getBlock(id) != nullptr);

  REQUIRE_FALSE(source.save());
  Buffer loaded(path.string());
  REQUIRE_FALSE(loaded.load());
  REQUIRE(loaded.getCanvas().getBlock(id) != nullptr);
  REQUIRE(loaded.canUndo());

  REQUIRE(DecodeBuffer("{", decoded));
  REQUIRE(dynamic_cast<const Text *>(decoded.getCanvas().getBlock(id))
              ->toString() == U"A界\nZ");
  std::filesystem::remove(path);
}

} // namespace Lin
