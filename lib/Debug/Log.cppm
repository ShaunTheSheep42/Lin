module;

#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/spdlog.h"
#include <memory>
#include <mutex>
#include <print>
#include <string>

export module Debug:Log;

namespace Lin {

namespace Log {

std::shared_ptr<spdlog::logger> logger;
std::mutex mtx;

export void Init(const std::string &logPath) try {
  const uint files = 3;
  const uint fileSize = 1048576 * 10; // 10MB

  auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  consoleSink->set_level(spdlog::level::debug);
  consoleSink->set_pattern(
      "[%Y-%m-%d %H:%M:%S] [thread %t] [%^%l%$] %v"); // With color

  auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
      logPath, fileSize, files);
  fileSink->set_level(spdlog::level::info);
  fileSink->set_pattern("[%Y-%m-%d %H:%M:%S] [thread %t] [%l] %v");

  logger = std::make_shared<spdlog::logger>(
      "Lin", spdlog::sinks_init_list{consoleSink, fileSink});
  logger->set_level(spdlog::level::debug);

  // Immediately flush logs at the ERROR level and above
  logger->flush_on(spdlog::level::err);

  spdlog::register_logger(logger);

  // https://github.com/gabime/spdlog/wiki/Error-handling
  // Spdlog will not throw exceptions while logging (since version 39cdd08)
  spdlog::set_error_handler([](const std::string &msg) {
    std::println(stderr, "LOG(Fallback): {}", msg);
  });

  spdlog::flush_every(std::chrono::seconds(3));
} catch (const spdlog::spdlog_ex &ex) {
  logger = nullptr;
  return;
}

export void Close() {
  if (!logger)
    spdlog::shutdown();
  logger = nullptr;
}

} // namespace Log

export template <typename... Args>
inline void DEBUG(fmt::format_string<Args...> fmt, Args &&...args) {
#ifndef NDEBUG
  if (Log::logger) {
    std::lock_guard<std::mutex> lock(Log::mtx);
    Log::logger->debug(fmt, std::forward<Args>(args)...);
    return;
  }

  std::println(stderr, "LOG(Fallback): {}",
               fmt::format(fmt, std::forward<Args>(args)...));
#endif
}

export template <typename... Args>
inline void Info(fmt::format_string<Args...> fmt, Args &&...args) {
  if (Log::logger) {
    std::lock_guard<std::mutex> lock(Log::mtx);
    Log::logger->info(fmt, std::forward<Args>(args)...);
    return;
  }

  std::println(stderr, "LOG(Fallback): {}",
               fmt::format(fmt, std::forward<Args>(args)...));
}

export template <typename... Args>
inline void Warn(fmt::format_string<Args...> fmt, Args &&...args) {
  if (Log::logger) {
    std::lock_guard<std::mutex> lock(Log::mtx);
    Log::logger->warn(fmt, std::forward<Args>(args)...);
    return;
  }

  std::println(stderr, "LOG(Fallback): {}",
               fmt::format(fmt, std::forward<Args>(args)...));
}

export template <typename... Args>
inline void Error(fmt::format_string<Args...> fmt, Args &&...args) {
  if (Log::logger) {
    std::lock_guard<std::mutex> lock(Log::mtx);
    Log::logger->error(fmt, std::forward<Args>(args)...);
    return;
  }

  std::println(stderr, "LOG(Fallback): {}",
               fmt::format(fmt, std::forward<Args>(args)...));
}

} // namespace Lin
