#pragma once
#include <array>

namespace Utils::Meta::Templates
{
  template<typename Type, typename ... T>
  constexpr auto MakeArray(T&&... t)->std::array<Type, sizeof...(T)>
  {
    return { {std::forward<T>(t)...} };
  }
}