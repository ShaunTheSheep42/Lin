module;

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include <glaze/core/context.hpp>

export module Basic.Text;

import Basic.Block;
import Basic.Geometry;
import Basic.Char;
import Basic.String;

namespace Lin {

export enum EditPosition { Forward, Backward };

export struct EditResult {
  Position nextCursorPos;
  EditPosition ep;
};

// All positions are absolute; there are no relative positions
export class Text : public Block {
public:
  Text(Position anchor, uint64_t groupId);
  Text(Position anchor);
  static Text FromSerialized(uint64_t id, uint64_t groupId, Position anchor,
                             String payload);

  std::unique_ptr<Block> clone() const override;
  bool hitTest(Position p) const override;

  Char getRawChar(Position p) const;
  Char getChar(Position p) const;
  const String &getPayload() const { return payload; }
  std::u32string toString() const;
  bool isEmpty() const override;

  // Setter
  void updateChar(Position p, Char ch);
  EditResult insertChar(Position p, Char ch, EditPosition ep = Forward);
  EditResult insertString(Position p, std::u32string s,
                          EditPosition ep = Forward);
  EditResult deleteChar(Position p, EditPosition ep = Backward);

private:
  String payload;
};

export glz::error_ctx EncodeText(const Text &text, std::string &json);
export glz::error_ctx DecodeText(std::string_view json, Text &text);

} // namespace Lin
