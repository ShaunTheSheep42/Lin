#pragma once

#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include <glaze/glaze.hpp>

namespace Lin::Serialization {

struct BlockRecord {
  std::string type;
  uint64_t id = 0;
  uint64_t groupId = 0;
  int anchorX = 0;
  int anchorY = 0;
  std::vector<std::vector<uint32_t>> payload;
};

struct CanvasRecord {
  std::vector<BlockRecord> blocks;
};

struct ChangeRecord {
  std::string operation;
  uint64_t blockId = 0;
  std::optional<BlockRecord> before;
  std::optional<BlockRecord> after;
  std::vector<ChangeRecord> changes;
};

struct HistoryRecord {
  std::vector<ChangeRecord> entries;
  std::size_t index = 0;
};

struct VersionRecord {
  int major = 1;
  int minor = 0;
  int patch = 0;
};

struct BufferRecord {
  VersionRecord version;
  CanvasRecord canvas;
  HistoryRecord history;
};

} // namespace Lin::Serialization

template <> struct glz::meta<Lin::Serialization::BlockRecord> {
  using T = Lin::Serialization::BlockRecord;
  static constexpr auto value = glz::object(
      "type", &T::type, "id", &T::id, "groupId", &T::groupId, "anchorX",
      &T::anchorX, "anchorY", &T::anchorY, "payload", &T::payload);
};

template <> struct glz::meta<Lin::Serialization::CanvasRecord> {
  using T = Lin::Serialization::CanvasRecord;
  static constexpr auto value = glz::object("blocks", &T::blocks);
};

template <> struct glz::meta<Lin::Serialization::ChangeRecord> {
  using T = Lin::Serialization::ChangeRecord;
  static constexpr auto value =
      glz::object("operation", &T::operation, "blockId", &T::blockId, "before",
                  &T::before, "after", &T::after, "changes", &T::changes);
};

template <> struct glz::meta<Lin::Serialization::HistoryRecord> {
  using T = Lin::Serialization::HistoryRecord;
  static constexpr auto value =
      glz::object("entries", &T::entries, "index", &T::index);
};

template <> struct glz::meta<Lin::Serialization::VersionRecord> {
  using T = Lin::Serialization::VersionRecord;
  static constexpr auto value =
      glz::object("major", &T::major, "minor", &T::minor, "patch", &T::patch);
};

template <> struct glz::meta<Lin::Serialization::BufferRecord> {
  using T = Lin::Serialization::BufferRecord;
  static constexpr auto value = glz::object("version", &T::version, "canvas",
                                            &T::canvas, "history", &T::history);
};
