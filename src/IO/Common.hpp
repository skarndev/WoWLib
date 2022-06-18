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

  enum class FourCCEndian
  {
    LITTLE = 0, // commonly used order of chars in bytes (little endian, chars sre written right to left in the file)
    BIG = 1 // use in m2 (big endian, chars are written left to right in the file)
  };
   
  template <StringLiteral fourcc, FourCCEndian reverse = FourCCEndian::LITTLE>
  static constexpr std::uint32_t FourCC = static_cast<bool>(reverse) ? (fourcc.value[3] << 24 | fourcc.value[2] << 16 | fourcc.value[1] << 8 | fourcc.value[0])
       : (fourcc.value[0] << 24 | fourcc.value[1] << 16 | fourcc.value[2] << 8 | fourcc.value[3]);

  template <std::uint32_t fourcc_int, FourCCEndian reverse = FourCCEndian::LITTLE>
  static constexpr char FourCCStr[5] = { static_cast<bool>(reverse) ? fourcc_int & 0xFF : (fourcc_int >> 24) & 0xFF,
                                         static_cast<bool>(reverse) ? (fourcc_int >> 8) & 0xFF : (fourcc_int >> 16) & 0xFF,
                                         static_cast<bool>(reverse) ? (fourcc_int >> 16) & 0xFF : (fourcc_int >> 8) & 0xFF,
                                         static_cast<bool>(reverse) ? (fourcc_int >> 24) & 0xFF : fourcc_int & 0xFF,
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

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed = FourCCEndian::LITTLE>
  struct DataChunk
  {
    typedef std::conditional_t<sizeof(T) <= sizeof(std::size_t), T, T const&> InterfaceType;

    void Initialize()
    {
      RequireF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      std::memset(&data, 0, sizeof(T));
      _is_initialized = true;
    };

    void Initialize(InterfaceType data_block)
    {
      data = data_block;
      _is_initialized = true;
    };

    void Read(ByteBuffer const& buf, std::size_t size)
    {
      LogDebugF(LCodeZones::FILE_IO, "Reading chunk: %s, size: %d"
        , FourCCStr<fourcc, fourcc_reversed>
        , sizeof(T));

      RequireF(CCodeZones::FILE_IO, !(size % sizeof(T)), "Provided size is not the same as the size of underlying structure.");

      buf.Read(data);
      _is_initialized = true;
    }

    void Write(ByteBuffer& buf) const
    {
      if (!_is_initialized) [[unlikely]]
        return;

      LogDebugF(LCodeZones::FILE_IO, "Writing chunk: %s, size: %d"
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

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed = FourCCEndian::LITTLE, std::int64_t size_min = -1, std::int64_t size_max = -1>
  struct DataArrayChunk
  {
    typedef std::conditional_t<size_max == size_min && size_max >= 0, std::array<T, size_max>, std::vector<T>> ArrayImplT;

    void Initialize() 
    { 
      RequireF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      _is_initialized = true;
    };

    void Initialize(T const& data_block, std::size_t n) 
    { 
      RequireF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      RequireF(LCodeZones::FILE_IO, (size_min < 0 || n >= size_min, size_max < 0 || n <= size_max),
        "Attempted to initialize size-constrained chunk with mismatching size (%d), min: %d, max: %d.", n, size_min, size_max);

      if constexpr (std::is_same_v<ArrayImplT, std::vector<T>>)
      {
        _data.resize(n);
        std::fill(_data.begin(), _data.end(), data_block);
        _is_initialized = true;
      }
      else
      {
        RequireF(LCodeZones::FILE_IO, n == _data.size(), "Attempted to initialize static chunk with non-matching size (%d).", n);
        std::fill(_data.begin(), _data.end(), data_block);
        _is_initialized = true;
      }
    };

    void Initialize(ArrayImplT const& data_array)
    {
      RequireF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
      _data = data_array;
      _is_initialized = true;
    }

    void Read(ByteBuffer const& buf, std::size_t size)
    {
      LogDebugF(LCodeZones::FILE_IO, "Reading chunk: %s, size: %d"
        , FourCCStr<fourcc, fourcc_reversed>
        , size);

      RequireF(CCodeZones::FILE_IO, !(size % sizeof(T)),
        "Provided size is not evenly divisible divisible by the size of underlying structure.");

      std::size_t n_elements;

      if constexpr (std::is_same_v<ArrayImplT, std::vector<T>>)
      {
        n_elements = size / sizeof(T);
        _data.resize(n_elements);
      }
      else
      {
        n_elements = _data.size();
      }

      EnsureF(LCodeZones::FILE_IO, (size_min < 0 || n_elements >= size_min, size_max < 0 || n_elements <= size_max),
        "Expected to read satisfying size constraint (min: %d, max: %d), got size %d instead.", size_min, size_max, n_elements);

      buf.Read(_data.begin(), _data.end());

      _is_initialized = true;

    }

    void Write(ByteBuffer& buf) const
    {
      if (!_is_initialized) [[unlikely]]
        return;

      LogDebugF(LCodeZones::FILE_IO, "Writing array chunk: %s, length: %d, size: %d"
        , FourCCStr<fourcc, fourcc_reversed>
        , _data.size()
        , _data.size() * sizeof(T));

      RequireF(LCodeZones::FILE_IO, (size_min < 0 || _data.size() >= size_min, size_max < 0 || _data.size() <= size_max),
        "Expected to write chunk with size constraint (min: %d, max : %d), got size %d instead.", size_min, size_max, _data.size());

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

    T& Add() requires (std::is_same_v<ArrayImplT, std::vector<T>>)
    { 
      _is_initialized = true;
      T& ret = _data.emplace_back();
      std::memset(&ret, 0, sizeof(T));
      return ret;
    };

    void Remove(std::size_t index)  requires (std::is_same_v<ArrayImplT, std::vector<T>>)
    { 
      RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds remove of underlying chunk vector.");
      RequireF(CCodeZones::FILE_IO, _is_initialized, "Attempted removing on uninitialized chunk.");
      _data.erase(_data.begin() + index);
    }

    [[nodiscard]]
    T& At(std::size_t index) 
    { 
      RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
      RequireF(CCodeZones::FILE_IO, _is_initialized, "Attempted element access on uninitialized chunk.");
      return _data[index]; 
    }

    [[nodiscard]]
    ArrayImplT::iterator begin() { return _data.begin(); };

    [[nodiscard]]
    ArrayImplT::iterator end() { return _data.end(); };

    [[nodiscard]]
    ArrayImplT::const_iterator cbegin() const { return _data.cbegin(); };

    [[nodiscard]]
    ArrayImplT::const_iterator cend() const { return _data.end(); };

    [[nodiscard]]
    constexpr T& operator[](std::size_t index) 
    {
      RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
      return _data[index];
    }


  private:
    ArrayImplT _data;
    bool _is_initialized = false;
  };

}
