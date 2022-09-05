#pragma once

/**
 * Holds the the data types relevant for metaprogramming.
 */
namespace Utils::Meta::DataTypes
{
  /**
   * Universal type container.
   * @tparam T Types.
   */
  template<typename...T>
  struct TypePack {};

  /**
   * Universal container of compile-time constants (non-type template parameters).
   * @tparam T Compile-time constants. (non-type template parameters).
   */
  template<auto...T>
  struct ConstPack {};

  /**
   * Empty type to be used to indicate absence of type.
   */
  struct NoType : public std::false_type {};

  /**
   * A return structure of type-based lookup agorithms. Represents a type and its position in the pack.
   * @tparam T Any type.
   * @tparam idx Index of type in the pack.
   */
  template<typename T, std::size_t idx>
  struct TypeIndex
  {
    using type = T; ///> Result type. Utils::Meta::DataTypes::NoType is typically used if element was not found.
    static constexpr std::size_t index = idx; ///> Position of type in the pack. std::numeric_limits<std::size_t>::max() if not found.

  };

  /**
   * A return structure of NTTP-based lookup algorithms. Represents a compile-time variable and its position in the pack.
   * @tparam var Any NTTP.
   * @tparam idx Index of NTTP in the pack.
   */
  template<auto var, std::size_t idx>
  struct NTTPIndex
  {
    static constexpr auto value = var; ///> Result value. Instance of Utils::Meta::DataTypes::NoType if not found (typically).
    static constexpr std::size_t index = idx; ///> Position of NTTP in the pack. std::numeric_limits<std::size_t>::max() if not found.
  };
}