#include "Lin/Config.h"
// #include <Python.h>

#include <QFile>
#include <QTextStream>
#include <optional>

namespace Lin {

std::optional<Config> LoadConfig(QString path) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    qWarning("Cannot open config file");
    return std::nullopt;
  }

  QTextStream in(&file);
  QString script = in.readAll();

  // PyRun_SimpleString(script.toUtf8().constData());
  return std::nullopt;
}

} // namespace Lin
