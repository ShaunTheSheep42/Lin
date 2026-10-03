module;

#include <cstdint>
#include <fstream>
#include <functional>
#include <iterator>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "Serialization.hpp"

module Basic.Buffer;

import Basic.Block;
import Basic.Canvas;
import Basic.History;
import Basic.Text;
import Basic.ViewPort;
import Debug;

namespace Lin {

Buffer::Buffer(std::string path) : viewPort(canvas), path(std::move(path)) {}

void Buffer::addBlock(std::unique_ptr<Block> block) {
  Check(block != nullptr, "Cannot add a null block to a buffer");
  if (block->isEmpty())
    return;
  Check(canvas.getBlock(block->getId()) == nullptr,
        "Cannot add duplicate block ID");
  history.pushChange(Change::Added(*block));
  canvas.addBlock(std::move(block));
}

void Buffer::updateBlock(uint64_t id, std::unique_ptr<Block> block) {
  Check(block != nullptr, "Cannot update a buffer block with a null pointer");
  Check(block->getId() == id,
        "Can't update block {}, because the new block has a different id", id);
  const Block *previous = canvas.getBlock(id);
  if (previous == nullptr)
    return;
  history.pushChange(Change::Updated(*previous, *block));
  canvas.setBlock(id, std::move(block));
}

bool Buffer::editBlock(uint64_t id, const std::function<void(Block &)> &edit) {
  Check(static_cast<bool>(edit), "Cannot edit a block with an empty callback");
  const Block *current = canvas.getBlock(id);
  if (current == nullptr)
    return false;

  std::unique_ptr<Block> replacement = current->clone();
  Check(replacement != nullptr, "Block clone returned null");
  edit(*replacement);
  Check(replacement->getId() == id, "Editing a block changed its identity");
  history.pushChange(Change::Updated(*current, *replacement));
  canvas.setBlock(id, std::move(replacement));
  return true;
}

void Buffer::deleteBlock(uint64_t id) {
  const Block *block = canvas.getBlock(id);
  if (block == nullptr)
    return;
  history.pushChange(Change::Deleted(*block));
  canvas.deleteBlock(id);
}

bool Buffer::undo() { return history.undo(canvas); }

bool Buffer::redo() { return history.redo(canvas); }

bool Buffer::canUndo() const { return history.canUndo(); }

bool Buffer::canRedo() const { return history.canRedo(); }

glz::error_ctx EncodeBuffer(const Buffer &buffer, std::string &json) {
  Serialization::BufferRecord record;
  record.version = {buffer.version.major, buffer.version.minor,
                    buffer.version.patch};

  std::string canvasJson;
  if (auto error = EncodeCanvas(buffer.canvas, canvasJson); error)
    return error;
  if (auto error = glz::read_json(record.canvas, canvasJson); error)
    return error;

  std::string historyJson;
  if (auto error = EncodeHistory(buffer.history, historyJson); error)
    return error;
  if (auto error = glz::read_json(record.history, historyJson); error)
    return error;

  return glz::write_json(record, json);
}

glz::error_ctx DecodeBuffer(std::string_view json, Buffer &buffer) {
  Serialization::BufferRecord record;
  if (auto error = glz::read_json(record, json); error)
    return error;
  if (record.version.major != buffer.version.major)
    return {0, glz::error_code::version_mismatch,
            "Unsupported buffer file version"};

  std::string canvasJson;
  if (auto error = glz::write_json(record.canvas, canvasJson); error)
    return error;
  Canvas canvas;
  if (auto error = DecodeCanvas(canvasJson, canvas); error)
    return error;

  std::string historyJson;
  if (auto error = glz::write_json(record.history, historyJson); error)
    return error;
  History history;
  if (auto error = DecodeHistory(historyJson, history); error)
    return error;

  buffer.canvas = std::move(canvas);
  buffer.history = std::move(history);
  buffer.version = {record.version.major, record.version.minor,
                    record.version.patch};
  return {};
}

glz::error_ctx Buffer::save() const {
  if (path.empty())
    return {0, glz::error_code::file_open_failure,
            "Cannot save a buffer with an empty path"};

  std::string json;
  if (auto error = EncodeBuffer(*this, json); error)
    return error;

  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file)
    return {0, glz::error_code::file_open_failure,
            "Could not open the buffer file for writing"};
  file.write(json.data(), static_cast<std::streamsize>(json.size()));
  file.flush();
  if (!file)
    return {0, glz::error_code::file_close_failure,
            "Could not write the complete buffer file"};
  return {};
}

glz::error_ctx Buffer::load() {
  if (path.empty())
    return {0, glz::error_code::file_open_failure,
            "Cannot load a buffer with an empty path"};

  std::ifstream file(path, std::ios::binary);
  if (!file)
    return {0, glz::error_code::file_open_failure,
            "Could not open the buffer file for reading"};
  const std::string json((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
  if (file.bad())
    return {0, glz::error_code::file_open_failure,
            "Could not read the complete buffer file"};
  return DecodeBuffer(json, *this);
}

} // namespace Lin
