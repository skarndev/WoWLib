#include <type_traits>

namespace Utils::Meta::Concepts
{
  template<typename T>
  concept ImplicitLifetimeTypeSingular = std::is_scalar_v<T> || std::is_class_v<T>
    && std::is_trivially_default_constructible_v<T> && std::is_trivially_destructible_v<T>;

  template<typename T>
  concept ImplicitLifetimeType = ImplicitLifetimeTypeSingular<T> 
    || std::is_array_v<T> && ImplicitLifetimeTypeSingular<std::remove_all_extents_t<T>>;
}
