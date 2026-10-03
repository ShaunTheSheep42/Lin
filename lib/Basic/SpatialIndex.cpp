module;

#include <algorithm>
#include <cstdint>
#include <unordered_set>
#include <vector>

module Basic.SpatialIndex;

import Basic.Geometry;
import Debug;

namespace Lin {
namespace {

int FloorDivide(int value, int divisor) {
  const int quotient = value / divisor;
  const int remainder = value % divisor;
  return remainder < 0 ? quotient - 1 : quotient;
}

} // namespace

uint64_t SpatialIndex::encode(int cx, int cy) { return Encode(cx, cy); }

Position SpatialIndex::getChunkCoord(Position p) {
  return {FloorDivide(p.x, ChunkSize), FloorDivide(p.y, ChunkSize)};
}

void SpatialIndex::insert(uint64_t id, Position anchor) {
  for (auto chunk = chunks.begin(); chunk != chunks.end();) {
    auto &entries = chunk->second;
    entries.erase(
        std::remove_if(entries.begin(), entries.end(),
                       [id](const Entry &entry) { return entry.id == id; }),
        entries.end());
    if (entries.empty())
      chunk = chunks.erase(chunk);
    else
      ++chunk;
  }
  const Position chunkCoord = getChunkCoord(anchor);
  chunks[encode(chunkCoord.x, chunkCoord.y)].push_back({id, anchor});
}

void SpatialIndex::remove(uint64_t id, Position anchor) {
  const Position chunkCoord = getChunkCoord(anchor);
  const auto chunk = chunks.find(encode(chunkCoord.x, chunkCoord.y));
  if (chunk == chunks.end())
    return;

  auto &entries = chunk->second;
  const auto entry = std::find_if(
      entries.begin(), entries.end(), [id, anchor](const Entry &candidate) {
        return candidate.id == id && candidate.anchor.x == anchor.x &&
               candidate.anchor.y == anchor.y;
      });
  if (entry == entries.end())
    return;

  entries.erase(entry);
  if (entries.empty())
    chunks.erase(chunk);
}

void SpatialIndex::move(uint64_t id, Position oldAnchor, Position newAnchor) {
  const Position oldChunkCoord = getChunkCoord(oldAnchor);
  const auto oldChunk = chunks.find(encode(oldChunkCoord.x, oldChunkCoord.y));
  if (oldChunk != chunks.end()) {
    auto &entries = oldChunk->second;
    const auto entry = std::find_if(
        entries.begin(), entries.end(),
        [id](const Entry &candidate) { return candidate.id == id; });
    if (entry != entries.end()) {
      Check(entry->anchor.x == oldAnchor.x && entry->anchor.y == oldAnchor.y,
            "SpatialIndex move for block {} has an outdated old anchor", id);
      entries.erase(entry);
      if (entries.empty())
        chunks.erase(oldChunk);

      const Position newChunkCoord = getChunkCoord(newAnchor);
      chunks[encode(newChunkCoord.x, newChunkCoord.y)].push_back(
          {id, newAnchor});
      return;
    }
  }

  for (const auto &[key, chunk] : chunks) {
    if (key == encode(oldChunkCoord.x, oldChunkCoord.y))
      continue;
    const auto existing =
        std::find_if(chunk.begin(), chunk.end(), [id](const Entry &candidate) {
          return candidate.id == id;
        });
    Check(existing == chunk.end(),
          "SpatialIndex move for block {} has an outdated old anchor", id);
  }
  insert(id, newAnchor);
}

std::vector<uint64_t> SpatialIndex::query(Position topLeft,
                                          Position bottomRight) const {
  if (topLeft.x > bottomRight.x || topLeft.y > bottomRight.y)
    return {};

  const Position firstChunk = getChunkCoord(topLeft);
  const Position lastChunk = getChunkCoord(bottomRight);
  std::vector<uint64_t> result;

  for (int cy = firstChunk.y; cy <= lastChunk.y; ++cy) {
    for (int cx = firstChunk.x; cx <= lastChunk.x; ++cx) {
      const auto chunk = chunks.find(encode(cx, cy));
      if (chunk == chunks.end())
        continue;

      for (const Entry &entry : chunk->second) {
        const Position p = entry.anchor;
        if (p.x >= topLeft.x && p.x <= bottomRight.x && p.y >= topLeft.y &&
            p.y <= bottomRight.y)
          result.push_back(entry.id);
      }
    }
  }

  std::sort(result.begin(), result.end());
  return result;
}

void SpatialIndex::rebuild(
    const std::vector<std::pair<uint64_t, Position>> &blockAnchors) {
  clear();
  std::unordered_set<uint64_t> seen;
  seen.reserve(blockAnchors.size());
  for (const auto &[id, anchor] : blockAnchors) {
    if (seen.insert(id).second) {
      const Position chunkCoord = getChunkCoord(anchor);
      chunks[encode(chunkCoord.x, chunkCoord.y)].push_back({id, anchor});
    } else {
      insert(id, anchor);
    }
  }
}

void SpatialIndex::clear() { chunks.clear(); }

} // namespace Lin
