module;

#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "Serialization.hpp"

module Basic.Block;

import Basic.Char;
import Basic.Geometry;
import Basic.String;
import Basic.Text;

namespace Lin {

glz::error_ctx EncodeBlock(const Block &block, std::string &json) {
  const auto *text = dynamic_cast<const Text *>(&block);
  if (text == nullptr)
    return {0, glz::error_code::feature_not_supported,
            "No JSON codec is registered for this block type"};
  return EncodeText(*text, json);
}

glz::error_ctx DecodeBlock(std::string_view json,
                           std::unique_ptr<Block> &block) {
  Serialization::BlockRecord record;
  if (auto error = glz::read_json(record, json); error)
    return error;
  if (record.type != "text")
    return {0, glz::error_code::feature_not_supported,
            "No JSON codec is registered for this block type"};

  String payload;
  payload.reserve(record.payload.size());
  for (const auto &line : record.payload) {
    auto &decodedLine = payload.emplace_back();
    decodedLine.reserve(line.size());
    for (uint32_t ch : line)
      decodedLine.push_back(static_cast<Char>(ch));
  }
  block = std::make_unique<Text>(Text::FromSerialized(
      record.id, record.groupId, {record.anchorX, record.anchorY},
      std::move(payload)));
  return {};
}

} // namespace Lin
