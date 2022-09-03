#ifndef UTILS_META_TEMPLATES_HPP
#define UTILS_META_TEMPLATES_HPP

#include <boost/hana/string.hpp>

#include <array>
#include <vector>
#include <type_traits>
#include <algorithm>
#include <limits>
#include <string_view>


namespace Utils::Meta::Templates
{
  namespace details
  {
    /**
     * Used for generating a unique empty type paramterized for T. Service struct for OptionalBase and alike.
     * @tparam T Any type.
     */
    template<typename T>
    struct Empty{};
  }

  /**
   * Utility to provide an optional base for types.
   * @tparam T Type to use as a base for another type.
   * @tparam use constexpr boolean or expression convertible to boolean to determine whether to use this base or not.
   */
  template<typename T, bool use>
  using OptionalBase = typename std::conditional_t<use, T, details::Empty<T>>;

  /**
   * Make std::array given a homogenous list of literals.
   * @tparam Type std::array underlying type.
   * @tparam T Literals.
   * @param t Homogenous list of literals.
   * @return std::array<Type, sizeof(T)>
   */
  template<typename Type, typename ... T>
  constexpr auto MakeArray(T&&... t) -> std::array<Type, sizeof...(T)>
  {
    return { {std::forward<T>(t)...} };
  }


  /**
   * Utility structure to allow passing string literals as non-type template parameters.
   * @tparam n Number of characters in string (normally deduced).
   */
  template<std::size_t n>
  struct StringLiteral
  {
    constexpr StringLiteral(const char(&str)[n])
    {
      std::copy_n(str, n - 1, value);
    }

    constexpr StringLiteral(const char* str, [[maybe_unused]] std::size_t n_chars)
    {
      std::copy_n(str, n - 1, value);
    }

    char value[n - 1];
    const std::size_t size = n - 1;

    template<std::size_t other_n>
    constexpr bool operator==(StringLiteral<other_n> const& other) const
    {
      if constexpr (other_n != n)
        return false;

      for (std::size_t i = 0; i < n - 1; ++i)
      {
        if (value[i] != other.value[i])
          return false;
      }
      return true;
    }
  };

  /**
   * Compute if two values are equal to each other and pass the provided type.
   * To be used with std::disjunction / std::conjunction and alike.
   * @tparam v1 First value.
   * @tparam v2 Second value.
   * @tparam T Type to store.
   * @tparam do_compute Enables computation of expression. If false, expression is always false.
   */
  template<auto v1, decltype(v1) v2, typename T, bool do_compute = true>
  struct ValuesEqualT : std::bool_constant<do_compute && v1 == v2>
  {
    using type = T;
  };

  /**
   * DefaultType<T>::value is always true.
   * To be used with std::disjunction / std::conjunction and alike.
   * @tparam T Type to store.
   */
  template <typename T>
  struct DefaultType : std::true_type
  {
    using type = T;
  };

  /**
   * Helper struct meant to be inherited from in order to create array wrappers.
   * If both size_min and size_max are the same, the chunk array is optimized
   * as a std::array with fixed number of elements, except when both
   * are std::numeric_limits<std::size_t>::max() (default). In that case, the array is
   * dynamic (vector).
   *
   * The array implements an interface simialr to stl containers except providing
   * no exceptions. All validation is performed with contracts in debug mode.
   * @tparam T Value type of the underlying array.
   * @tparam size_min Minimum amount of elements stored in the array. std::size_t::max means a variable bound.
   * @tparam size_max Maximum amount of elements stored in the array. std::size_t::max means a variable bound.
   */
  template
  <
    typename T
    , std::size_t size_min = std::numeric_limits<std::size_t>::max()
    , std::size_t size_max = std::numeric_limits<std::size_t>::max()
  >
  struct ConstrainedArray
  {
    using ValueType = T;
    using ArrayImplT = std::conditional_t<size_max == size_min && size_max < std::numeric_limits<std::size_t>::max()
      , std::array<T, size_max>, std::vector<T>>;
    using iterator = typename ArrayImplT::iterator;
    using const_iterator = typename ArrayImplT::const_iterator;

    /**
    * Returns the number of elements stored in the container.
    * @return Number of elements in the container.
    */
    [[nodiscard]]
    std::size_t Size() const { return _data.size(); };

    /**
     * Default constructs a new element in the underlying vector and returns reference to it (dynamic size only).
     * @return Reference to the constructed object.
     */
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    T& Add() requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    /**
     * Removes an element by its index in the underlying vector. Bounds checks are debug-only, no exceptions.
     * (dynamic arrays only).
     * @param index
     */
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(std::size_t index) requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    /**
     * Removes an element by its iterator in the underlying vector. Bounds checks are debug-only, no exceptions.
     * (dynamic arrays only).
     * @param it Iterator pointing to the element to remove.
     */
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(typename ArrayImplT_::iterator it) requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    /**
     *  Clears the underlying vector (dynamic size only).
     */
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Clear() requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    /**
     * Returns reference to the element of the underlying vector by its index.
     * @param index Index of the elemnt in the underlying vector in a valid range.
     * @return Reference to vector element.
     */
    [[nodiscard]]
    T& At(std::size_t index);

    /**
     * Returns a const reference to the element of the underlying vector by its index.
     * @param index Index of the elemnt in the underlying vector in a valid range.
     * @return Constant reference to vector element.
     */
    [[nodiscard]]
    T const& At(std::size_t index) const;

    [[nodiscard]]
    typename ArrayImplT::const_iterator begin() const { return _data.cbegin(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator end() const { return _data.cend(); };

    [[nodiscard]]
    typename ArrayImplT::iterator begin() { return _data.begin(); };

    [[nodiscard]]
    typename ArrayImplT::iterator end() { return _data.end(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator cbegin() const { return _data.cbegin(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator cend() const { return _data.cend(); };

    [[nodiscard]]
    T& operator[](std::size_t index);

    [[nodiscard]]
    T const& operator[](std::size_t index) const;


  protected:
    ArrayImplT _data;

  };

  /**
   * Implements a wrapper over enumerations intended to be used for versioned enum-like constructs.
   * @tparam client_version Version of game client.
   * @tparam ValueType Value type of the enum.
   */
  template<auto client_version, std::integral ValueType>
  requires (std::is_integral_v<decltype(client_version)> || std::is_enum_v<decltype(client_version)>)
  struct VersionedEnum
  {
    VersionedEnum() = default;
    VersionedEnum(ValueType value) : _data(value) {};

    [[nodiscard]]
    operator ValueType() const { return _data; };

    VersionedEnum& operator=(ValueType other) { _data = other; return *this; };

    template<auto other_client_version>
    [[nodiscard]]
    bool operator==(VersionedEnum<other_client_version, ValueType> const& other) const
    {
      return _data == static_cast<ValueType>(other);
    }

    template<auto other_client_version>
    [[nodiscard]]
    bool operator<(VersionedEnum<other_client_version, ValueType> const& other) const
    {
      return _data < static_cast<ValueType>(other);
    }

    template<auto other_client_version>
    [[nodiscard]]
    bool operator>(VersionedEnum<other_client_version, ValueType> const& other) const
    {
      return _data > static_cast<ValueType>(other);
    }

  protected:
    ValueType _data;
  };

  /**
   * Always false. Use for static_asserts etc.
   * @tparam T Any type.
   */
  template<typename T>
  struct AlwaysFalse : std::false_type {};

  /**
   * Always false. Use for static_asserts etc.
   * @tparam T Any type.
   */
  template<typename T>
  constexpr bool AlwaysFalse_V = AlwaysFalse<T>::value;

}

#include <Utils/Meta/Templates.inl>
#endif // UTILS_META_TEMPLATES_HPP