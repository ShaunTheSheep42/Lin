module;

#include <exception>
#include <filesystem>
#include <string>

module App;

import Debug;
import Support;

namespace Lin {

App::App(int argc, char *argv[]) try {
  workDir = !std::filesystem::exists(workDir) ? Env::Expand(workDirFallback)
                                              : Env::Expand(workDir);
  Log::Init(workDir + "/lin.log");
  Info("Lin Init...");

  configDir = !std::filesystem::exists(configDir)
                  ? Env::Expand(configDirFallback)
                  : Env::Expand(configDir);

  args = {argv + 1, static_cast<std::size_t>(argc - 1)};

  initStatus = true;
} catch (const std::exception &e) {
  Error("Init Error: {}", e.what());
}

App::~App() {
  Info("Lin Exit...");
  Log::Close();
}

int App::run() try {
  if (!initStatus)
    return 1;

  Info("Lin Running...");
  parseCommandLine();

  return 0;
} catch (const std::exception &e) {
  Error("Running Error: {}", e.what());
  return 1;
}

void App::parseCommandLine() {}

} // namespace Lin
