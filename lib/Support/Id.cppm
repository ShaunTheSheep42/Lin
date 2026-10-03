module;

#include <chrono>
#include <cstdint>

export module Support:Id;
import :Random;

namespace Lin {

export uint64_t CreateOnlyID() {
  auto now = std::chrono::system_clock::now();
  auto timestamp =
      std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch())
          .count();

  // High 32 bits: Current timestamp (in seconds)
  // Low 32 bits: Random number
  return (static_cast<uint64_t>(timestamp) << 32) | Random::Number<uint32_t>();
}

} // namespace Lin
