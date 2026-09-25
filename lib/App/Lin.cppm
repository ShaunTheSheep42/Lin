module;

#include "CLI/CLI.hpp"

export module App;

export class Lin {
public:
  Lin(int argc, char *argv[]);
  int run();

private:
};
