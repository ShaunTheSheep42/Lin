#pragma once

#include "Basic/Canvas.h"
#include "Basic/Direction.h"
#include "Edit/Mode.h"
#include "Edit/Region.h"
#include <qtypes.h>
#include <optional>

namespace Lin {

struct RenderData {
  Mode mode;
  Canvas *mainCanvas;
  Canvas *tmpCanvas;
  std::optional<Block> block;
  EditPosition ep;
  Direction direction;
  std::optional<Edit::Region> visualRegion;
  bool visualRectangle = false;
};

} // namespace Lin
