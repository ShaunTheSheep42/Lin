module;

#include <cstdlib>
#include <optional>
#include <string>

export module Support:Env;

namespace Lin::Env {

export std::optional<std::string> Get(const std::string &env) {
  if (const char *val = std::getenv(env.c_str()))
    return std::string(val);

  return std::nullopt;
}

// Expand the environment variables in the string
// Support format: "${VAR_NAME}"
export std::string Expand(const std::string &ss) {
  std::string result;
  result.reserve(ss.size());

  for (size_t i = 0; i < ss.size(); ++i) {
    if (ss[i] != '$') {
      result += ss[i];
      continue;
    }

    std::string varName;

    size_t end = 0;
    // ${VAR}
    if (i + 1 < ss.size() && ss[i + 1] == '{') {
      end = ss.find('}', i + 2);

      // Not closed, remain as is.
      if (end == std::string::npos) {
        result += ss[i];
        continue;
      }

      varName = ss.substr(i + 2, end - (i + 2));
    }

    auto val = Get(varName);
    if (val.has_value()) {
      result += val.value();
      i = end; // Jump to '}'
    } else {
      // Environment variables do not exist, leave them as is.
      result += ss[i];
    }
  }

  return result;
}

} // namespace Lin::Env
