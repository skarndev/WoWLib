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





