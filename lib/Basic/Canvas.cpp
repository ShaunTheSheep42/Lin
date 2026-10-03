module;

#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "Serialization.hpp"

module Basic.Canvas;
import Basic.Block;
import Basic.Text;
import Debug;

namespace Lin {

const Block *Canvas::getBlock(uint64_t id) const {
  const auto it = blocks.find(id);
  return it == blocks.end() ? nullptr : it->second.get();
}

const Block *Canvas::getBlockAt(Position p) const {
  for (const auto &[id, block] : blocks) {
    if (block->hitTest(p))
      return block.get();
  }
  return nullptr;
}

uint64_t Canvas::getBlockIdAt(Position p) const {
  const Block *block = getBlockAt(p);
  return block == nullptr ? 0 : block->getId();
}

uint64_t Canvas::getGroupIdAt(Position p) const {
  const Block *block = getBlockAt(p);
  return block == nullptr ? 0 : block->getGroupId();
}

std::vector<std::pair<uint64_t, Position>> Canvas::getBlockAnchors() const {
  std::vector<std::pair<uint64_t, Position>> result;
  result.reserve(blocks.size());
  for (const auto &[id, block] : blocks)
    result.emplace_back(id, block->getAnchor());
  return result;
}

void Canvas::updateBlock(uint64_t id, std::unique_ptr<Block> newBlock) {
  Check(newBlock != nullptr, "Cannot update a block with a null pointer");
  Check(newBlock->getId() == id,
        "Can't update block {}, because the new block has a different id", id);

  const auto it = blocks.find(id);
  if (it == blocks.end())
    return;

  if (newBlock->isEmpty()) {
    blocks.erase(it);
    return;
  }

  it->second = std::move(newBlock);
}

void Canvas::addBlock(std::unique_ptr<Block> block) {
  Check(block != nullptr, "Cannot add a null block");
  if (block->isEmpty())
    return;

  const uint64_t id = block->getId();
  Check(!blocks.contains(id), "Cannot add duplicate block ID {}", id);
  blocks.emplace(id, std::move(block));
}

void Canvas::deleteBlock(uint64_t id) { blocks.erase(id); }

void Canvas::setBlock(uint64_t id, std::unique_ptr<Block> block) {
  if (block == nullptr) {
    deleteBlock(id);
    return;
  }
  Check(block->getId() == id,
        "Can't set block {}, because the new block has a different id", id);
  if (block->isEmpty()) {
    deleteBlock(id);
    return;
  }
  blocks.insert_or_assign(id, std::move(block));
}

void Canvas::reset() { blocks.clear(); }

glz::error_ctx EncodeCanvas(const Canvas &canvas, std::string &json) {
  Serialization::CanvasRecord record;
  const auto anchors = canvas.getBlockAnchors();
  record.blocks.reserve(anchors.size());
  for (const auto &[id, anchor] : anchors) {
    const Block *block = canvas.getBlock(id);
    if (block == nullptr)
      return {0, glz::error_code::constraint_violated,
              "Canvas block disappeared during serialization"};

    std::string blockJson;
    if (auto error = EncodeBlock(*block, blockJson); error)
      return error;
    Serialization::BlockRecord encoded;
    if (auto error = glz::read_json(encoded, blockJson); error)
      return error;
    record.blocks.push_back(std::move(encoded));
  }
  std::sort(
      record.blocks.begin(), record.blocks.end(),
      [](const auto &left, const auto &right) { return left.id < right.id; });
  return glz::write_json(record, json);
}

glz::error_ctx DecodeCanvas(std::string_view json, Canvas &canvas) {
  Serialization::CanvasRecord record;
  if (auto error = glz::read_json(record, json); error)
    return error;

  Canvas decoded;
  std::unordered_set<uint64_t> seen;
  seen.reserve(record.blocks.size());
  for (const auto &blockRecord : record.blocks) {
    if (!seen.insert(blockRecord.id).second)
      return {0, glz::error_code::constraint_violated,
              "Canvas contains duplicate block IDs"};

    std::string blockJson;
    if (auto error = glz::write_json(blockRecord, blockJson); error)
      return error;
    std::unique_ptr<Block> block;
    if (auto error = DecodeBlock(blockJson, block); error)
      return error;
    decoded.addBlock(std::move(block));
  }
  canvas = std::move(decoded);
  return {};
}

} // namespace Lin
