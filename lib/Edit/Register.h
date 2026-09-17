#pragma once

#include "Basic/Block.h"
#include <unordered_map>

namespace Lin::Edit {

using Register = std::variant<Block, Blocks, Chars>;
class RegisterManager {
public:
  RegisterManager();
  std::optional<Register> &getRegister(int name);
  void syncFromSystemClipboard();
  void syncToSystemClipboard();

private:
  // ':',1,2,3,4,5,6,7,8,9,0,';'
  // ':' for last command
  // ';' for system clipboard
  // 0~9 for common clipboard
  std::unordered_map<int, std::optional<Register>> payload;
};

} // namespace Lin::Edit
