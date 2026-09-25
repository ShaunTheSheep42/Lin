module;

#include <limits>
#include <random>

export module Support:Random;

namespace lin {

export class Random {
public:
  template <typename T> static T Number() { return Get().number<T>(); }

private:
  Random() : gen(rd()) {}

  static Random &Get() {
    thread_local static Random random;
    return random;
  }

  template <typename T> T number() {
    std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(),
                                          std::numeric_limits<T>::max());
    return dist(gen);
  }

  std::random_device rd;
  std::mt19937 gen;
};

} // namespace lin
