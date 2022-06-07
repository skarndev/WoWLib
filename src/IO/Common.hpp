#pragma once

#include <IO/ByteBuffer.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Validation/Contracts.hpp>
#include <Validation/Log.hpp>
#include <unordered_map>
#include <fstream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cstdint>
#include <algorithm>

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

  template <std::uint32_t fourcc_int, bool reverse = false>
  static constexpr char FourCCStr[5] = { reverse ? fourcc_int & 0xFF : (fourcc_int >> 24) & 0xFF,
                                         reverse ? (fourcc_int >> 8) & 0xFF : (fourcc_int >> 16) & 0xFF,
                                         reverse ? (fourcc_int >> 16) & 0xFF : (fourcc_int >> 8) & 0xFF,
                                         reverse ? (fourcc_int >> 24) & 0xFF : fourcc_int & 0xFF,
                                         '\0'
                                       };

  inline std::string FourCCToStr(std::uint32_t fourcc, bool reverse = false)
  {
    char fourcc_str[5] = { static_cast<char>(reverse ? fourcc & 0xFF : (fourcc >> 24) & 0xFF),
                           static_cast<char>(reverse ? (fourcc >> 8) & 0xFF : (fourcc >> 16) & 0xFF),
                           static_cast<char>(reverse ? (fourcc >> 16) & 0xFF : (fourcc >> 8) & 0xFF),
                           static_cast<char>(reverse ? (fourcc >> 24) & 0xFF : fourcc & 0xFF),
                           '\0'
                         };

    return std::string(&fourcc_str[0]);
  }

  struct ChunkHeader
  {
    std::uint32_t fourcc;
    std::uint32_t size;
  };

  template<Utils::Meta::Concepts::PODType T>
  struct DataChunk
  {
    typedef std::conditional_t<sizeof(T) <= sizeof(std::size_t), T, T const&> InterfaceType;

    void Initialize()
    {
      RequireF(LCodeZones::ADT_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      _is_initialized = true;
    };

    void Initialize(InterfaceType data_block)
    {
      data = data_block;
      _is_initialized = true;
    };

    void Read(ByteBuffer const& buf)
    {
      buf.Read(data);
      _is_initialized = true;
    }

    template<std::uint32_t fourcc, bool fourcc_reversed = false>
    void Write(ByteBuffer& buf) const
    {
      if (!_is_initialized)
        return;

      LogDebugF(LCodeZones::ADT_IO, "Writing chunk: %s, size: %d"
        , FourCCStr<fourcc, fourcc_reversed>
        , sizeof(T));

      ChunkHeader header{};
      header.fourcc = fourcc;
      header.size = sizeof(T);

      buf.Write(data);
    }

    [[nodiscard]]
    std::size_t ByteSize() const { return sizeof(T); };

    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };
   
    T data;

  private:
    bool _is_initialized = false;
  };

  template<Utils::Meta::Concepts::PODType T>
  struct DataArrayChunk
  {
    void Initialize() 
    { 
      RequireF(LCodeZones::ADT_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      _is_initialized = true;
    };

    void Initialize(T const& data_block, std::size_t n) 
    { 
      RequireF(LCodeZones::ADT_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      std::fill(_data.begin(), _data.end(), data_block); 
      _is_initialized = true;
    };

    void Initialize(std::vector<T> const& data_vec) 
    {
      RequireF(LCodeZones::ADT_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      _data = data_vec;
      _is_initialized = true;
    }

    void Read(ByteBuffer const& buf)
    {
      for (auto& element : _data)
      {
        buf.Read(element);
      }
    }

    template<std::uint32_t fourcc, bool fourcc_reversed = false>
    void Write(ByteBuffer& buf) const
    {
      if (!_is_initialized)
        return;

      LogDebugF(LCodeZones::ADT_IO, "Writing array chunk: %s, length: %d, size: %d"
        , FourCCStr<fourcc, fourcc_reversed>
        , _data.size()
        , _data.size() * sizeof(T));

      ChunkHeader header{};
      header.fourcc = fourcc;
      EnsureF(CCodeZones::FILE_IO, (_data.size() * sizeof(T)) <= std::numeric_limits<std::uint32_t>::max(), "Chunk size overflow.");
      header.size = static_cast<std::uint32_t>(_data.size() * sizeof(T));

      buf.Write(header);

      for (auto& element : _data)
      {
        buf.Write(element);
      }
    }

    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };

    [[nodiscard]]
    std::size_t Size() const { return _data.size(); };

    [[nodiscard]]
    std::size_t ByteSize() const { return _data.size() * sizeof(T); };

    T& Add() { return _data.emplace_back(); };

    void Remove(std::size_t index) 
    { 
      RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds remove of underlying chunk vector.");
      _data.erase(_data.begin() + index);
    }

    T& At(std::size_t index) 
    { 
      RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
      return _data[index]; 
    }

    T* begin() { return &*_data.begin(); };
    T* end() { return &*_data.end(); };
    const T* cbegin() const { return &*_data.cbegin(); };
    const T* cend() const { return &*_data.end(); };

    constexpr T& operator[](std::size_t index) 
    {
      RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
      return _data[index];
    }


  private:
    std::vector<T> _data;
    bool _is_initialized = false;
  };

}

//#include <IO/Common.inl>