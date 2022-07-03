#pragma once

// This file is intended for implementations of features that are known to come in C++23 or later,
// but not yet available, in order to ease code updates later.
// they are defined in namespace future::, which is to be mass replaced by std:: later.

#include <tuple>
#include <cassert>
#include <functional>
#include <iomanip>
#include <iostream>
#include <list>
#include <string>
#include <vector>
#include <typeinfo>
#include <type_traits>
#include <utility>

namespace future
{
  // zip, courtesy of https://github.com/CommitThis/zip-iterator
  template <typename Iter>
  using select_access_type_for = std::conditional_t<
    std::is_same_v<Iter, std::vector<bool>::iterator> ||
    std::is_same_v<Iter, std::vector<bool>::const_iterator>,
    typename Iter::value_type,
    typename Iter::reference
  >;


  template <typename ... Args, std::size_t ... Index>
  auto any_match_impl(std::tuple<Args...> const& lhs,
    std::tuple<Args...> const& rhs,
    std::index_sequence<Index...>) -> bool
  {
    auto result = false;
    result = (... | (std::get<Index>(lhs) == std::get<Index>(rhs)));
    return result;
  }


  template <typename ... Args>
  auto any_match(std::tuple<Args...> const& lhs, std::tuple<Args...> const& rhs)
    -> bool
  {
    return any_match_impl(lhs, rhs, std::index_sequence_for<Args...>{});
  }



  template <typename ... Iters>
  class zip_iterator
  {
  public:

    using value_type = std::tuple<
      select_access_type_for<Iters>...
    >;

    zip_iterator() = delete;

    zip_iterator(Iters && ... iters)
      : m_iters{ std::forward<Iters>(iters)... }
    {
    }

    auto operator++() -> zip_iterator&
    {
      std::apply([](auto && ... args) { ((args += 1), ...); }, m_iters);
      return *this;
    }

    auto operator++(int) -> zip_iterator
    {
      auto tmp = *this;
      ++* this;
      return tmp;
    }

    auto operator!=(zip_iterator const& other)
    {
      return !(*this == other);
    }

    auto operator==(zip_iterator const& other)
    {
      auto result = false;
      return any_match(m_iters, other.m_iters);
    }

    auto operator*() -> value_type
    {
      return std::apply([](auto && ... args) {
        return value_type(*args...);
        }, m_iters);
    }

  private:
    std::tuple<Iters...> m_iters;
  };


  /* std::decay needed because T is a reference, and is not a complete type */
  template <typename T>
  using select_iterator_for = std::conditional_t<
    std::is_const_v<std::remove_reference_t<T>>,
    typename std::decay_t<T>::const_iterator,
    typename std::decay_t<T>::iterator>;



  template <typename ... T>
  class zipper
  {
  public:
    using zip_type = zip_iterator<select_iterator_for<T> ...>;

    template <typename ... Args>
    zipper(Args && ... args)
      : m_args{ std::forward<Args>(args)... }
    {
    }

    auto begin() -> zip_type
    {
      return std::apply([](auto && ... args) {
        return zip_type(std::begin(args)...);
        }, m_args);
    }
    auto end() -> zip_type
    {
      return std::apply([](auto && ... args) {
        return zip_type(std::end(args)...);
        }, m_args);
    }

  private:
    std::tuple<T ...> m_args;

  };


  template <typename ... T>
  auto zip(T && ... t)
  {
    return zipper<T ...>{std::forward<T>(t)...};
  }

  // enumerate

  namespace details
  {
    template <class T> std::add_lvalue_reference_t<T> declval();

    namespace adl
    {
      using std::begin;
      using std::end;
      template <class T> auto adlbegin(T &&t)
      {
        return begin(std::forward<T>(t));
      }
      template <class T> auto adlend(T &&t)
      {
        return end(std::forward<T>(t));
      }
    }
  }

/// Create an iterable enumerate wrapper around the iterable. The returned
/// iterator will return the index and the value.
///
/// \param t the iterable to iterate over.
/// \return an iterable enumerate wrapper.
/// \type T the iterable type to iterate over.
  template <class T,
      class TI = decltype(details::adl::adlbegin(details::declval<T>()))
  >
  constexpr auto enumerate(T &&t)
  {
    struct iterator
    {
      std::size_t i;
      TI iter;

      bool operator != (const iterator &other) const
      {
        return iter != other.iter;
      }

      iterator& operator++ ()
      {
        ++this->i;
        ++this->iter;
        return *this;
      }

      auto operator* () const
      {
        return std::tie(this->i, *this->iter);
      }
    };

    struct wrapper
    {
      T iterable;

      auto begin()
      {
        return iterator{0, details::adl::adlbegin(iterable)};
      }

      auto end()
      {
        return iterator{0, details::adl::adlend(iterable)};
      }
    };

    return wrapper{ std::forward<T>(t) };
  }

}





