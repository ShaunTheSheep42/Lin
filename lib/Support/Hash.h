#pragma once

#include <QPoint>
#include <QString>

namespace std {

template <> struct hash<QPoint> {
  std::size_t operator()(const QPoint &p) const noexcept { return qHash(p); }

  std::size_t operator()(const QString &key) const noexcept {
    return qHash(key);
  }
};

} // namespace std
