#pragma once

#include "Config.h"
#include "Edit/Core.h"
#include "UI/Core.h"
#include <QApplication>
#include <QCommandLineParser>
#include <memory>

namespace Lin {

class App {
public:
  App(int argc, char *argv[]);
  void run();

private:
  bool isTransparent() const { return cfg.isTransparent; }
  int getOpacity() const { return cfg.opacity; }

  const QString configPath =
      qEnvironmentVariable("XDG_CONFIG_HOME", QDir::homePath() + "/.config");

  QString filePath;
  Config cfg;
  QCommandLineParser parser;
  QApplication app;
  std::unique_ptr<Edit::Core> edit;
  std::unique_ptr<UI::Core> ui;
};

} // namespace Lin
