module;

#include <cassert>
#include <format>
#include <print>

export module Debug;
export import :Log;

namespace Lin {

export template <typename... Args>
inline void Crash(std::format_string<Args...> fmt, Args &&...args) {
  Error("Crash! {}", std::vformat(fmt.get(), std::make_format_args(args...)));
  std::abort();
}

export template <typename... Args>
inline void Check(bool condition, std::format_string<Args...> fmt,
                  Args &&...args) {
  if (condition)
    return;

#ifndef NDEBUG
  std::println(stderr, fmt, args...);
  std::abort();
#endif

  Crash(fmt, args...);
}

} // namespace Lin
