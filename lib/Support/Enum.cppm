module;

#include "magic_enum/magic_enum.hpp"

export module Support:Enum;

namespace Lin {

template <typename, typename = void> struct has_None : std::false_type {};

template <typename E>
struct has_None<E, std::void_t<decltype(E::None)>> : std::true_type {};

template <typename E> constexpr bool has_None_v = has_None<E>::value;
export template <typename E>
  requires has_None_v<E>
inline std::string Enum(E e) {
  return magic_enum::enum_name(e).data();
}

export template <typename E>
  requires has_None_v<E>
inline E Enum(const std::string &sv) {
  auto e = magic_enum::enum_cast<E>(sv);
  if (e.has_value())
    return *e;

  return E::None;
}

} // namespace Lin
