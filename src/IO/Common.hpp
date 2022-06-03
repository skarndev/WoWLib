#pragma once

#include <IO/ByteBuffer.hpp>
#include <Utils/Meta/Traits.hpp>
#include <unordered_map>
#include <fstream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cstdint>
#include <variant>
#include <optional>

namespace IO::Common
{
  namespace
  {
    template<std::size_t n>
    struct StringLiteral
    {
      constexpr StringLiteral(const char(&str)[n])
      {
        std::copy_n(str, n - 1, value);
      }

      char value[n - 1];
    };
  }
   
  template <StringLiteral fourcc, bool reverse = false>
  static constexpr std::uint32_t FourCC = reverse ? (fourcc.value[3] << 24 | fourcc.value[2] << 16 | fourcc.value[1] << 8 | fourcc.value[0])
       : (fourcc.value[0] << 24 | fourcc.value[1] << 16 | fourcc.value[2] << 8 | fourcc.value[3]);

  struct ChunkHeader
  {
    std::uint32_t fourcc;
    std::uint32_t size;
  };

  template<Utils::Meta::Concepts::PODType T>
  struct DataChunk
  {
    void Read(ByteBuffer const& buf)
    {
      buf.Read(data);
    }

    void Write(ByteBuffer& buf) const
    {
      buf.Write(data);
    }
   
    T data;
  };

  template<Utils::Meta::Concepts::PODType T>
  struct DataArrayChunk
  {
    void Read(ByteBuffer const& buf)
    {
      for (auto& element : _data)
      {
        buf.Read(element);
      }
    }

    void Write(ByteBuffer& buf) const
    {
      for (auto& element : _data)
      {
        buf.Write(element);
      }
    }

    [[nodiscard]]
    std::size_t Size() const { return _data.size(); };

    [[nodiscard]]
    std::size_t ByteSize() const { return _data.size() * sizeof(T); };

    T& Add() { return _data.emplace_back(); };

    void Remove(std::size_t index) 
    { 
      Require(index < _data.size(), "Out of bounds remove of underlying chunk vector.");
      _data.erase(_data.begin() + index);
    }

    T& At(std::size_t index) 
    { 
      Require(index < _data.size(), "Out of bounds access to underlying chunk vector.");
      return _data[index]; 
    }

    T* begin() { return &*_data.begin(); };
    T* end() { return &*_data.end(); };
    const T* cbegin() const { return &*_data.cbegin(); };
    const T* cend() const { return &*_data.end(); };

    constexpr T& operator[](std::size_t index) 
    {
      Require(index < _data.size(), "Out of bounds access to underlying chunk vector.");
      return _data[index];
    }


  private:
    std::vector<T> _data;
  };

}

//#include <IO/Common.inl>