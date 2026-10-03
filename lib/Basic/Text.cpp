module;

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Serialization.hpp"

module Basic.Text;

import Basic.Geometry;
import Basic.Char;
import Basic.String;
import Basic.Block;
import Debug;

namespace Lin {
namespace {

int Width(Char ch) { return IsFullWidth(ch) ? 2 : 1; }

bool IsNewline(Char ch) { return ToRawChar(ch) == MakeChar(U'\n'); }

bool IsContent(Char ch) { return ch != WhiteSpace; }

void EnsureRow(String &text, int y) {
  Check(y >= 0, "Text row {} is before its anchor", y);
  if (static_cast<std::size_t>(y) >= text.size())
    text.resize(static_cast<std::size_t>(y) + 1);
}

void EnsureColumn(std::vector<Char> &line, int x) {
  Check(x >= 0, "Text column {} is before its anchor", x);
  if (static_cast<std::size_t>(x) >= line.size())
    line.resize(static_cast<std::size_t>(x) + 1, WhiteSpace);
}

int HeadAt(const std::vector<Char> &line, int x) {
  if (x > 0 && static_cast<std::size_t>(x) < line.size() &&
      line[static_cast<std::size_t>(x)] == PlaceHolder)
    return x - 1;
  return x;
}

int SegmentStart(const std::vector<Char> &line, int x) {
  int start = x;
  while (start > 0 && static_cast<std::size_t>(start - 1) < line.size() &&
         IsContent(line[static_cast<std::size_t>(start - 1)]))
    --start;
  return start;
}

int SegmentEnd(const std::vector<Char> &line, int x) {
  int end = x;
  while (static_cast<std::size_t>(end) < line.size() &&
         IsContent(line[static_cast<std::size_t>(end)]))
    ++end;
  return end;
}

struct Segment {
  int start;
  int end;
  int boundary;
};

Segment FindSegment(const std::vector<Char> &line, int x, bool backward) {
  const int size = static_cast<int>(line.size());
  int probe = std::clamp(x, 0, size);

  if (backward) {
    if (probe < size && IsContent(line[static_cast<std::size_t>(probe)])) {
      probe = HeadAt(line, probe);
    } else {
      while (probe > 0 && !IsContent(line[static_cast<std::size_t>(probe - 1)]))
        --probe;
      if (probe > 0)
        probe = HeadAt(line, probe - 1);
      else {
        while (probe < size &&
               !IsContent(line[static_cast<std::size_t>(probe)]))
          ++probe;
      }
    }
  } else {
    if (probe < size && IsContent(line[static_cast<std::size_t>(probe)])) {
      probe = HeadAt(line, probe);
    } else {
      while (probe < size && !IsContent(line[static_cast<std::size_t>(probe)]))
        ++probe;
      if (probe == size) {
        probe = std::min(x, size);
        while (probe > 0 &&
               !IsContent(line[static_cast<std::size_t>(probe - 1)]))
          --probe;
        if (probe > 0)
          probe = HeadAt(line, probe - 1);
      }
    }
  }

  if (probe >= size || !IsContent(line[static_cast<std::size_t>(probe)])) {
    const int aligned = std::max(0, x);
    return {aligned, aligned, aligned};
  }

  const int start = SegmentStart(line, probe);
  const int end = SegmentEnd(line, probe);
  int boundary = std::clamp(x, start, end);
  if (backward) {
    if (x < size && IsContent(line[static_cast<std::size_t>(x)])) {
      const int head = HeadAt(line, x);
      boundary = head + Width(line[static_cast<std::size_t>(head)]);
    } else {
      boundary = std::min(x + 1, end);
    }
  } else if (boundary > start &&
             static_cast<std::size_t>(boundary) < line.size() &&
             line[static_cast<std::size_t>(boundary)] == PlaceHolder) {
    --boundary;
  }
  return {start, end, boundary};
}

int SplitLine(String &text, int y, Segment segment) {
  auto &line = text[static_cast<std::size_t>(y)];
  std::vector<Char> nextLine;
  if (segment.boundary < segment.end) {
    nextLine.resize(static_cast<std::size_t>(segment.start), WhiteSpace);
    nextLine.insert(nextLine.end(), line.begin() + segment.boundary,
                    line.begin() + segment.end);
    std::fill(line.begin() + segment.boundary, line.begin() + segment.end,
              WhiteSpace);
    if (segment.start == 0 && segment.boundary == 0)
      line.erase(line.begin(), line.begin() + segment.end);
  }
  while (!line.empty() && line.back() == WhiteSpace)
    line.pop_back();
  text.insert(text.begin() + y + 1, std::move(nextLine));
  return segment.start;
}

void RemoveCharacter(std::vector<Char> &line, int head) {
  if (head < 0 || static_cast<std::size_t>(head) >= line.size())
    return;

  const Char ch = line[static_cast<std::size_t>(head)];
  if (ch == WhiteSpace || ch == PlaceHolder)
    return;

  const int width = Width(ch);
  const int end = SegmentEnd(line, head + width);
  for (int column = head; column < end - width; ++column)
    line[static_cast<std::size_t>(column)] =
        line[static_cast<std::size_t>(column + width)];
  std::fill(line.begin() + end - width, line.begin() + end, WhiteSpace);
  while (!line.empty() && line.back() == WhiteSpace)
    line.pop_back();
}

void InsertWhiteSpace(std::vector<Char> &line, int x) {
  line.insert(line.begin() + x, WhiteSpace);
}

void InsertCharacter(std::vector<Char> &line, int x, Char ch) {
  const int width = Width(ch);
  EnsureColumn(line, x);

  if (line[static_cast<std::size_t>(x)] == WhiteSpace) {
    int gapEnd = x;
    while (static_cast<std::size_t>(gapEnd) < line.size() &&
           line[static_cast<std::size_t>(gapEnd)] == WhiteSpace)
      ++gapEnd;
    const int available = gapEnd - x;
    if (available < width) {
      line.insert(line.begin() + gapEnd,
                  static_cast<std::size_t>(width - available), WhiteSpace);
    }
  } else {
    line.insert(line.begin() + x, static_cast<std::size_t>(width), WhiteSpace);

    int gapStart = x + width;
    while (static_cast<std::size_t>(gapStart) < line.size() &&
           IsContent(line[static_cast<std::size_t>(gapStart)]))
      ++gapStart;
    int gapEnd = gapStart;
    while (static_cast<std::size_t>(gapEnd) < line.size() &&
           line[static_cast<std::size_t>(gapEnd)] == WhiteSpace)
      ++gapEnd;
    const int consume = std::min(width, gapEnd - gapStart);
    if (consume > 0)
      line.erase(line.begin() + gapEnd - consume, line.begin() + gapEnd);
  }

  EnsureColumn(line, x + width - 1);
  line[static_cast<std::size_t>(x)] = ch;
  if (width == 2)
    line[static_cast<std::size_t>(x + 1)] = PlaceHolder;
}

} // namespace

Text::Text(Position anchor, uint64_t groupId) : Block(anchor, groupId) {}
Text::Text(Position anchor) : Block(anchor) {}

Text Text::FromSerialized(uint64_t id, uint64_t groupId, Position anchor,
                          String payload) {
  Text text(anchor, groupId);
  text.id = id;
  text.payload = std::move(payload);
  return text;
}

std::unique_ptr<Block> Text::clone() const {
  return std::make_unique<Text>(*this);
}

bool Text::hitTest(Position p) const {
  const int y = p.y - anchor.y;
  const int x = p.x - anchor.x;
  if (x < 0 || y < 0 || static_cast<std::size_t>(y) >= payload.size())
    return false;

  const auto &line = payload[static_cast<std::size_t>(y)];
  return static_cast<std::size_t>(x) < line.size() &&
         line[static_cast<std::size_t>(x)] != WhiteSpace;
}

Char Text::getRawChar(Position p) const {
  const int y = p.y - anchor.y;
  const int x = p.x - anchor.x;
  Check(y >= 0 && static_cast<std::size_t>(y) < payload.size(),
        "Position {},{} is not on Text", p.x, p.y);
  const auto &line = payload[static_cast<std::size_t>(y)];
  Check(x >= 0 && static_cast<std::size_t>(x) < line.size(),
        "Position {},{} is not on Text", p.x, p.y);
  return line[static_cast<std::size_t>(x)];
}

Char Text::getChar(Position p) const {
  const Char ch = getRawChar(p);
  if (ch == PlaceHolder)
    return getRawChar({p.x - 1, p.y});
  return ch;
}

std::u32string Text::toString() const { return ToStdU32String(payload); }

bool Text::isEmpty() const {
  for (const auto &line : payload)
    for (Char ch : line)
      if (ch != WhiteSpace && ch != PlaceHolder)
        return false;
  return true;
}

glz::error_ctx EncodeText(const Text &text, std::string &json) {
  Serialization::BlockRecord record;
  record.type = "text";
  record.id = text.getId();
  record.groupId = text.getGroupId();
  record.anchorX = text.getAnchor().x;
  record.anchorY = text.getAnchor().y;
  record.payload.reserve(text.getPayload().size());
  for (const auto &line : text.getPayload()) {
    auto &encodedLine = record.payload.emplace_back();
    encodedLine.reserve(line.size());
    for (Char ch : line)
      encodedLine.push_back(static_cast<uint32_t>(ch));
  }
  return glz::write_json(record, json);
}

glz::error_ctx DecodeText(std::string_view json, Text &text) {
  Serialization::BlockRecord record;
  if (auto error = glz::read_json(record, json); error)
    return error;
  if (record.type != "text")
    return {0, glz::error_code::constraint_violated, "Expected a text block"};

  String payload;
  payload.reserve(record.payload.size());
  for (const auto &line : record.payload) {
    auto &decodedLine = payload.emplace_back();
    decodedLine.reserve(line.size());
    for (uint32_t ch : line)
      decodedLine.push_back(static_cast<Char>(ch));
  }
  text = Text::FromSerialized(record.id, record.groupId,
                              {record.anchorX, record.anchorY},
                              std::move(payload));
  return {};
}

void Text::updateChar(Position p, Char ch) {
  const int y = p.y - anchor.y;
  int x = p.x - anchor.x;
  Check(y >= 0 && x >= 0, "Position {},{} is before the Text anchor", p.x, p.y);
  Check(ch != PlaceHolder && ch != WhiteSpace,
        "Text control characters cannot be updated directly");

  EnsureRow(payload, y);
  auto &line = payload[static_cast<std::size_t>(y)];
  EnsureColumn(line, x);
  x = HeadAt(line, x);

  if (IsNewline(ch)) {
    const int oldWidth = line[static_cast<std::size_t>(x)] == WhiteSpace
                             ? 0
                             : Width(line[static_cast<std::size_t>(x)]);
    const Segment segment = FindSegment(line, x, false);
    const int split = std::clamp(x + oldWidth, segment.start, segment.end);
    Segment moved{segment.start, segment.end, split};
    if (oldWidth > 0)
      std::fill(line.begin() + x, line.begin() + x + oldWidth, WhiteSpace);
    SplitLine(payload, y, moved);
    return;
  }

  const Char old = line[static_cast<std::size_t>(x)];
  const int oldWidth = old == WhiteSpace ? 0 : Width(old);
  const int newWidth = Width(ch);
  if (newWidth > oldWidth) {
    int available = 0;
    while (available < newWidth - oldWidth &&
           static_cast<std::size_t>(x + oldWidth + available) < line.size() &&
           line[static_cast<std::size_t>(x + oldWidth + available)] ==
               WhiteSpace)
      ++available;
    const int extra = newWidth - oldWidth - available;
    if (extra > 0)
      line.insert(line.begin() + x + oldWidth + available,
                  static_cast<std::size_t>(extra), WhiteSpace);
  } else if (newWidth < oldWidth) {
    line.erase(line.begin() + x + newWidth, line.begin() + x + oldWidth);
  }

  EnsureColumn(line, x + newWidth - 1);
  line[static_cast<std::size_t>(x)] = ch;
  if (newWidth == 2)
    line[static_cast<std::size_t>(x + 1)] = PlaceHolder;
  else if (static_cast<std::size_t>(x + 1) < line.size() &&
           line[static_cast<std::size_t>(x + 1)] == PlaceHolder)
    line[static_cast<std::size_t>(x + 1)] = WhiteSpace;
}

EditResult Text::insertChar(Position p, Char ch, EditPosition ep) {
  const int y = p.y - anchor.y;
  int x = p.x - anchor.x;
  Check(y >= 0 && x >= 0, "Position {},{} is before the Text anchor", p.x, p.y);
  Check(ch != PlaceHolder, "PlaceHolder is generated by full-width characters");

  if (ToRawChar(ch) == MakeChar(U'\t'))
    ch = WhiteSpace;

  EnsureRow(payload, y);
  auto &line = payload[static_cast<std::size_t>(y)];
  EnsureColumn(line, x);
  x = HeadAt(line, x);

  if (IsNewline(ch)) {
    const Segment segment = FindSegment(line, x, ep == Backward);
    const int start = SplitLine(payload, y, segment);
    return {{anchor.x + start, anchor.y + y + 1}, Forward};
  }

  int insertX = x;
  if (ep == Backward) {
    if (line[static_cast<std::size_t>(x)] == WhiteSpace) {
      ++insertX;
    } else {
      insertX += Width(line[static_cast<std::size_t>(x)]);
    }
  }

  if (ch == WhiteSpace) {
    InsertWhiteSpace(line, insertX);
    if (ep == Forward)
      return {{anchor.x + insertX, anchor.y + y}, Forward};
    return {{anchor.x + insertX, anchor.y + y}, Backward};
  }

  InsertCharacter(line, insertX, ch);
  return {{anchor.x + insertX, anchor.y + y}, Backward};
}

EditResult Text::insertString(Position p, std::u32string s, EditPosition ep) {
  Position cursor = p;
  EditPosition cursorEp = ep;
  for (char32_t codePoint : s) {
    const auto result = insertChar(cursor, MakeChar(codePoint), cursorEp);
    cursor = result.nextCursorPos;
    cursorEp = result.ep;
  }
  return {cursor, cursorEp};
}

EditResult Text::deleteChar(Position p, EditPosition ep) {
  const int y = p.y - anchor.y;
  const int x = p.x - anchor.x;
  Check(y >= 0 && x >= 0, "Position {},{} is before the Text anchor", p.x, p.y);

  if (static_cast<std::size_t>(y) >= payload.size())
    return {p, ep == Backward ? Forward : Backward};

  auto &line = payload[static_cast<std::size_t>(y)];
  if (static_cast<std::size_t>(x) >= line.size())
    return {p, ep == Backward ? Forward : Backward};

  if (ep == Forward && x > 0) {
    if (line[static_cast<std::size_t>(x - 1)] == WhiteSpace) {
      line.erase(line.begin() + x - 1);
      return {{p.x - 1, p.y}, Forward};
    }

    const int previousHead = HeadAt(line, x - 1);
    if (IsContent(line[static_cast<std::size_t>(previousHead)])) {
      const int start = SegmentStart(line, previousHead);
      const int end = SegmentEnd(line, previousHead + 1);
      const bool hasPrevious = previousHead > start;
      const int width = Width(line[static_cast<std::size_t>(previousHead)]);
      const int hasNext = end > previousHead + width;
      RemoveCharacter(line, previousHead);
      if (hasNext)
        return {{anchor.x + previousHead, p.y}, Forward};
      if (hasPrevious)
        return {{anchor.x + previousHead - 1, p.y}, Backward};
      return {{anchor.x + previousHead, p.y}, Forward};
    }
  }

  if (line[static_cast<std::size_t>(x)] == WhiteSpace) {
    if (ep == Backward)
      return {p, Forward};
    return {p, Backward};
  }

  const int head = HeadAt(line, x);
  const int width = Width(line[static_cast<std::size_t>(head)]);
  const int start = SegmentStart(line, head);
  const int end = SegmentEnd(line, head + width);
  const bool hasPrevious = head > start;
  const bool hasNext = end > head + width;

  RemoveCharacter(line, head);

  if (ep == Backward) {
    if (hasPrevious)
      return {{anchor.x + head - 1, p.y}, Backward};
    return {{anchor.x + head, p.y}, Forward};
  }

  if (hasNext)
    return {{anchor.x + head, p.y}, Forward};
  if (hasPrevious)
    return {{anchor.x + head - 1, p.y}, Backward};
  return {{anchor.x + head, p.y}, Backward};
}

} // namespace Lin
