module;

#include <format>
#include <string>

export module Support:Version;

export namespace Lin {

struct Version {
  int major;
  int minor;
  int patch;
};

std::string ToString(Version v) {
  return std::format("{}.{}.{}", v.major, v.minor, v.patch);
}

} // namespace Lin
