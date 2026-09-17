#include "Lin.h"
#include "Edit/Core.h"
#include "UI/Theme.h"
#include <QObject>
#include <memory>

namespace Lin {

App::App(int argc, char *argv[]) : app(argc, argv) {
  parser.setApplicationDescription(
      "Lin, A pseudo-TUI editor for keyboard-driven, non-linear editor ");
  parser.addHelpOption();
  parser.addVersionOption();

  QCommandLineOption fileOpt(QStringList() << "f" << "file", "File to open",
                             "file");
  parser.addOption(fileOpt);

  if (!parser.parse(QCoreApplication::arguments()))
    qFatal() << parser.errorText();

  if (parser.isSet("help")) {
    parser.showHelp();
    return;
  }

  if (parser.isSet("version")) {
    parser.showVersion();
    return;
  }

  if (parser.isSet(fileOpt))
    filePath = parser.value(fileOpt);

  if (configPath.isEmpty())
    qFatal() << "No config path";

  ui = std::make_unique<UI::Core>(cfg, ThemeKind::Normal);
  edit = std::make_unique<Edit::Core>(cfg);
  edit->quit = [this] { app.quit(); };

  QObject::connect(ui.get(), &UI::Core::sendEvent, edit.get(),
                   &Edit::Core::handleEvent);
  QObject::connect(edit.get(), &Edit::Core::update, ui.get(),
                   &UI::Core::update);

  // 这里暂时不启用, 就使用默认的Config
  // 先完成主要的功能再搞配置
  // if (auto res = LoadConfig(configPath)) {
  //   cfg = res.value();
  //   qInfo() << "Load config file successfully";
  // } else {
  //   qFatal() << "Load config file failed";
  //   return false;
  // }
}

void App::run() {
  qInfo() << "App run now...";

  edit->openFile(filePath, true);
  ui->show();
  app.exec();
}

} // namespace Lin
