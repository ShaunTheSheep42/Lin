module;

#include <cstdint>
#include <unordered_map>
#include <utility>
#include <vector>

export module Basic.SpatialIndex;

import Basic.Geometry;

namespace Lin {

export class SpatialIndex {
public:
  SpatialIndex() = default;

  void insert(uint64_t id, Position anchor);
  void remove(uint64_t id, Position anchor);
  void move(uint64_t id, Position oldAnchor, Position newAnchor);
  std::vector<uint64_t> query(Position topLeft, Position bottomRight) const;
  void rebuild(const std::vector<std::pair<uint64_t, Position>> &blockAnchors);
  void clear();

private:
  struct Entry {
    uint64_t id;
    Position anchor;
  };

  static uint64_t encode(int cx, int cy);
  static Position getChunkCoord(Position p);

  std::unordered_map<uint64_t, std::vector<Entry>> chunks;
  static constexpr int ChunkSize = 32;
};

} // namespace Lin
