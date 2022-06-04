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

    template<std::uint32_t fourcc>
    void Write(ByteBuffer& buf) const
    {
      LogDebugF(LCodeZones::ADT_IO, "Writing chunk: %c%c%c%c, size: %d"
        , reinterpret_cast<const char*>(&fourcc)[3]
        , reinterpret_cast<const char*>(&fourcc)[2]
        , reinterpret_cast<const char*>(&fourcc)[1]
        , reinterpret_cast<const char*>(&fourcc)[0]
        , sizeof(T));

      ChunkHeader header{};
      header.fourcc = fourcc;
      header.size = sizeof(T);

      buf.Write(data);
    }
   
    T data;

  private:
    bool _is_initialised = false;
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

    template<std::uint32_t fourcc>
    void Write(ByteBuffer& buf) const
    {
      LogDebugF(LCodeZones::ADT_IO, "Writing array chunk: %c%c%c%c, length: %d, size: %d"
        , reinterpret_cast<const char*>(&fourcc)[3]
        , reinterpret_cast<const char*>(&fourcc)[2]
        , reinterpret_cast<const char*>(&fourcc)[1]
        , reinterpret_cast<const char*>(&fourcc)[0]
        , _data.size()
        , _data.size() * sizeof(T));

      ChunkHeader header{};
      header.fourcc = fourcc;
      header.size = _data.size() * sizeof(T);

      buf.Write(header);

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
    bool _is_initialised = false;
  };

}

//#include <IO/Common.inl>