module;

#include <span>
#include <string>

export module App;

namespace Lin {

export class App {
public:
  App(int argc, char *argv[]);
  ~App();
  int run();

private:
  void parseCommandLine();

  std::string configDir = "${XDG_CONFIG_HOME}/lin";
  std::string workDir = "${XDG_STATE_HOME}/lin";
  std::string dataDir = "${XDG_DATA_HOME}/lin";

  // Fallback
  std::string configDirFallback = "${HOME}/.config/state/lin";
  std::string workDirFallback = "${HOME}/.local/state/lin";
  std::string dataDirFallback = "${HOME}/.local/share/lin";

  bool initStatus = false;
  std::span<char *> args;
};

} // namespace Lin
