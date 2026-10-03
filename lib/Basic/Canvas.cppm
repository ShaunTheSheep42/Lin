module;

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <glaze/core/context.hpp>

export module Basic.Canvas;

import Basic.Char;
import Basic.Block;
import Basic.Geometry;

namespace Lin {

export class Canvas {
public:
  Canvas() { reset(); }
  Canvas(Canvas &&c) noexcept = default;
  Canvas &operator=(Canvas &&) noexcept = default;

  void reset();
  bool haveBlocks() const { return blocks.size() > 0; }
  const Block *getBlock(uint64_t id) const;
  const Block *getBlockAt(Position p) const;
  uint64_t getBlockIdAt(Position p) const;
  uint64_t getGroupIdAt(Position p) const;
  std::vector<std::pair<uint64_t, Position>> getBlockAnchors() const;

  void updateBlock(uint64_t id, std::unique_ptr<Block> newBlock);
  void addBlock(std::unique_ptr<Block> block);
  void deleteBlock(uint64_t id);
  void setBlock(uint64_t id, std::unique_ptr<Block> block);

private:
  Canvas(const Canvas &) = delete;
  Canvas &operator=(const Canvas &) = delete;

  std::unordered_map<uint64_t, std::unique_ptr<Block>> blocks;
};

export glz::error_ctx EncodeCanvas(const Canvas &canvas, std::string &json);
export glz::error_ctx DecodeCanvas(std::string_view json, Canvas &canvas);

} // namespace Lin
