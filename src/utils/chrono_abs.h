#pragma once

#include <chrono>

namespace inamata {
namespace utils {

template <class Rep, class Period,
          class = typename std::enable_if<
              std::chrono::duration<Rep, Period>::min() <
              std::chrono::duration<Rep, Period>::zero()>::type>
constexpr inline std::chrono::duration<Rep, Period> chrono_abs(
    std::chrono::duration<Rep, Period> d) {
  return d >= d.zero() ? d : -d;
}

}  // namespace utils
}  // namespace inamata