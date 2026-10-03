module;

#include <deque>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Serialization.hpp"

module Basic.History;

import Basic.Block;
import Basic.Text;
import Debug;

namespace Lin {

Change Change::Added(const Block &block) {
  Change change;
  change.blockId = block.getId();
  change.after = block.clone();
  Check(change.after != nullptr, "Block clone returned null");
  return change;
}

Change Change::Deleted(const Block &block) {
  Change change;
  change.blockId = block.getId();
  change.before = block.clone();
  Check(change.before != nullptr, "Block clone returned null");
  return change;
}

Change Change::Updated(const Block &before, const Block &after) {
  Check(before.getId() == after.getId(),
        "Cannot record an update between different block IDs");
  Change change;
  change.blockId = before.getId();
  change.before = before.clone();
  Check(change.before != nullptr, "Block clone returned null");
  if (!after.isEmpty()) {
    change.after = after.clone();
    Check(change.after != nullptr, "Block clone returned null");
  }
  return change;
}

Change Change::Batch(std::vector<Change> changes) {
  Change change;
  change.operation = Operation::Batch;
  change.changes = std::move(changes);
  return change;
}

void History::pushChange(Change change) {
  if (change.operation == Change::Operation::Batch && change.changes.empty())
    return;
  history.erase(history.begin() + static_cast<std::ptrdiff_t>(index),
                history.end());
  history.push_back(std::move(change));

  if (history.size() > HistoryMaxSize) {
    history.pop_front();
    if (index > 0)
      --index;
  }
  index = history.size();
}

bool History::canUndo() const { return index > 0; }

bool History::canRedo() const { return index < history.size(); }

void History::apply(Change &change, Canvas &canvas, bool forward) {
  if (change.operation == Change::Operation::Batch) {
    if (forward) {
      for (auto &child : change.changes)
        apply(child, canvas, true);
    } else {
      for (auto child = change.changes.rbegin(); child != change.changes.rend();
           ++child)
        apply(*child, canvas, false);
    }
    return;
  }

  const auto &target = forward ? change.after : change.before;
  canvas.setBlock(change.blockId,
                  target ? target->clone() : std::unique_ptr<Block>{});
}

bool History::undo(Canvas &canvas) {
  if (!canUndo())
    return false;
  apply(history[--index], canvas, false);
  return true;
}

bool History::redo(Canvas &canvas) {
  if (!canRedo())
    return false;
  apply(history[index++], canvas, true);
  return true;
}

void History::clear() {
  history.clear();
  index = 0;
}

namespace {

glz::error_ctx EncodeBlockRecord(const Block &block,
                                 Serialization::BlockRecord &record) {
  std::string json;
  if (auto error = EncodeBlock(block, json); error)
    return error;
  return glz::read_json(record, json);
}

glz::error_ctx DecodeBlockRecord(const Serialization::BlockRecord &record,
                                 std::unique_ptr<Block> &block) {
  std::string json;
  if (auto error = glz::write_json(record, json); error)
    return error;
  return DecodeBlock(json, block);
}

glz::error_ctx EncodeChange(const Change &change,
                            Serialization::ChangeRecord &record) {
  if (change.operation == Change::Operation::Batch) {
    record.operation = "batch";
    record.changes.reserve(change.changes.size());
    for (const auto &child : change.changes) {
      auto &encoded = record.changes.emplace_back();
      if (auto error = EncodeChange(child, encoded); error)
        return error;
    }
    return {};
  }

  record.operation = "state";
  record.blockId = change.blockId;
  if (change.before) {
    record.before.emplace();
    if (auto error = EncodeBlockRecord(*change.before, *record.before); error)
      return error;
  }
  if (change.after) {
    record.after.emplace();
    if (auto error = EncodeBlockRecord(*change.after, *record.after); error)
      return error;
  }
  return {};
}

glz::error_ctx DecodeChange(const Serialization::ChangeRecord &record,
                            Change &change) {
  if (record.operation == "batch") {
    if (record.before || record.after || record.blockId != 0)
      return {0, glz::error_code::constraint_violated,
              "Malformed batch history entry"};
    change.operation = Change::Operation::Batch;
    change.changes.resize(record.changes.size());
    for (std::size_t i = 0; i < record.changes.size(); ++i) {
      if (auto error = DecodeChange(record.changes[i], change.changes[i]);
          error)
        return error;
    }
    return {};
  }

  if (record.operation != "state" || (!record.before && !record.after) ||
      !record.changes.empty())
    return {0, glz::error_code::constraint_violated,
            "Malformed state history entry"};
  if ((record.before && record.before->id != record.blockId) ||
      (record.after && record.after->id != record.blockId))
    return {0, glz::error_code::constraint_violated,
            "History block ID does not match its snapshot"};

  change.operation = Change::Operation::State;
  change.blockId = record.blockId;
  if (record.before) {
    if (auto error = DecodeBlockRecord(*record.before, change.before); error)
      return error;
  }
  if (record.after) {
    if (auto error = DecodeBlockRecord(*record.after, change.after); error)
      return error;
  }
  return {};
}

} // namespace

glz::error_ctx EncodeHistory(const History &history, std::string &json) {
  Serialization::HistoryRecord record;
  record.index = history.index;
  record.entries.reserve(history.history.size());
  for (const auto &change : history.history) {
    auto &encoded = record.entries.emplace_back();
    if (auto error = EncodeChange(change, encoded); error)
      return error;
  }
  return glz::write_json(record, json);
}

glz::error_ctx DecodeHistory(std::string_view json, History &history) {
  Serialization::HistoryRecord record;
  if (auto error = glz::read_json(record, json); error)
    return error;
  if (record.entries.size() > History::HistoryMaxSize ||
      record.index > record.entries.size())
    return {0, glz::error_code::constraint_violated,
            "History size or index is out of range"};

  History decoded;
  for (const auto &entry : record.entries) {
    Change change;
    if (auto error = DecodeChange(entry, change); error)
      return error;
    decoded.history.push_back(std::move(change));
  }
  decoded.index = record.index;
  history = std::move(decoded);
  return {};
}

} // namespace Lin
