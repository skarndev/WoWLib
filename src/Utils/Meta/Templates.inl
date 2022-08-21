#pragma once
#include <Utils/Meta/Templates.hpp>
#include <Utils/Meta/Concepts.hpp>
#include <Config/CodeZones.hpp>

#include <cstring>

namespace Utils::Meta::Templates
{
  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline T& ConstrainedArray<T, size_min, size_max>::Add()
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    InvariantF(CCodeZones::FILE_IO, size_max - _data.size() >= 1, "Constrained array size overflow.");

    T& ret = _data.emplace_back();
    std::memset(&ret, 0, sizeof(T));
    return ret;
  }

  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline void ConstrainedArray<T, size_min, size_max>::Remove(std::size_t index)
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds remove of underlying chunk vector element.");
    _data.erase(_data.begin() + index);
  }

  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline void ConstrainedArray<T, size_min, size_max>::Remove(typename ArrayImplT_::iterator it)
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    RequireF(CCodeZones::FILE_IO, it < _data.end(), "Out of bounds remove of underlying vector element.");
    _data.erase(it);
  }

  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T& ConstrainedArray<T, size_min, size_max>::At(std::size_t index)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying vector.");
    return _data[index];
  }

  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T const& ConstrainedArray<T, size_min, size_max>::At(std::size_t index) const
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying vector.");
    return _data[index];
  }

  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T& ConstrainedArray<T, size_min, size_max>::operator[](std::size_t index)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying vector.");
    return _data[index];
  }

  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T const& ConstrainedArray<T, size_min, size_max>::operator[](std::size_t index) const
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying vector.");
    return _data[index];
  }

  template
  <
    typename T
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline void ConstrainedArray<T, size_min, size_max>::Clear()
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    _data.clear();
  }

}