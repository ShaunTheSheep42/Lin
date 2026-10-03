module;

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <glaze/core/context.hpp>
export module Basic.Block;

import Support;
import Basic.Geometry;

namespace Lin {

export class Block {

public:
  Block(Position anchor, uint64_t groupId)
      : id(CreateOnlyID()), anchor(anchor), groupId(groupId) {}
  Block(Position anchor)
      : id(CreateOnlyID()), anchor(anchor), groupId(CreateOnlyID()) {}
  virtual ~Block() = default;
  virtual std::unique_ptr<Block> clone() const = 0;

  uint64_t getId() const { return id; }
  uint64_t getGroupId() const { return groupId; }
  Position getAnchor() const { return anchor; }
  void moveToPosition(Position p) { anchor = p; }
  virtual bool hitTest(Position p) const = 0;
  virtual bool isEmpty() const { return false; }

protected:
  uint64_t id;
  uint64_t groupId;
  Position anchor;
};

export glz::error_ctx EncodeBlock(const Block &block, std::string &json);
export glz::error_ctx DecodeBlock(std::string_view json,
                                  std::unique_ptr<Block> &block);

} // namespace Lin
