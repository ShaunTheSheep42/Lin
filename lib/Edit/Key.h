#pragma once

#include <QKeyEvent>

namespace Lin::Edit {

struct Key {
  Qt::Key key;
  Qt::KeyboardModifiers mods;

  bool operator==(const Key &k) const noexcept {
    return key == k.key && mods == k.mods;
  }
};

} // namespace Lin::Edit
