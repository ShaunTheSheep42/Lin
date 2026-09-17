#include "Region.h"

namespace Lin::Edit {

template <typename T> Region CreateRegion(T t) { return Region{std::move(t)}; }

template <typename T> T GetRegion(const Region &r) {
  if (auto val = std::get_if<T>(&r))
    return *val;

  qFatal() << "Type Error for Region";
}

} // namespace Lin::Edit
