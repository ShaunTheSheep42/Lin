#pragma once

#include <QPoint>
#include <utility>
#include <variant>
#include <vector>

namespace Lin::Edit {

// Block, Group and the Other
// For other regions
// For regular characters, start == end
// However, for surrogate pairs, this is not the case
using Region =
    std::variant<quint64, std::vector<quint64>, std::pair<QPoint, QPoint>>;

template <typename T> Region CreateRegion(T t);
template <typename T> T GetRegion();

} // namespace Lin::Edit
