#pragma once

#include <IO/Common.hpp>
#include <Utils/Meta/Future.hpp>

#include <algorithm>

namespace IO::Common
{
  // DataChunk

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed>
  inline DataChunk<T, fourcc, fourcc_reversed>::DataChunk(InterfaceType data_block)
  {
    data = data_block;
    _is_initialized = true;
  }

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed>
  inline void DataChunk<T, fourcc, fourcc_reversed>::Initialize()
  {
    InvariantF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
    std::memset(&data, 0, sizeof(T));
    _is_initialized = true;
  }

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed>
  inline void DataChunk<T, fourcc, fourcc_reversed>::Initialize(InterfaceType data_block)
  {
    data = data_block;
    _is_initialized = true;
  }

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed>
  inline void DataChunk<T, fourcc, fourcc_reversed>::Read(ByteBuffer const& buf, std::size_t size)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading chunk: %s, size: %d."
              , FourCCStr<fourcc, fourcc_reversed>
              , sizeof(T));

    RequireF(CCodeZones::FILE_IO, !(size % sizeof(T))
             , "Provided size is not the same as the size of underlying structure.");

    buf.Read(data);
    _is_initialized = true;
  }

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed>
  inline void DataChunk<T, fourcc, fourcc_reversed>::Write(ByteBuffer& buf) const
  {
    if (!_is_initialized) [[unlikely]]
      return;

    LogDebugF(LCodeZones::FILE_IO, "Writing chunk: %s, size: %d."
              , FourCCStr<fourcc, fourcc_reversed>
              , sizeof(T));

    ChunkHeader header{};
    header.fourcc = fourcc;
    header.size = sizeof(T);

    buf.Write(header);
    buf.Write(data);
  }

  // DataArrayChunk
  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Initialize()
  {
    InvariantF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
    _is_initialized = true;
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Initialize(T const& data_block
                                                                                         , std::size_t n)
  {
    InvariantF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
    RequireMF(LCodeZones::FILE_IO, (size_min == std::numeric_limits<std::size_t>::max() || n >= size_min
        , size_max == std::numeric_limits<std::size_t>::max() || n <= size_max),
        "Attempted to initialize size-constrained chunk with mismatching size (%d), min: %d, max: %d."
        , n, size_min, size_max);

    if constexpr (std::is_same_v<ArrayImplT, std::vector<T>>)
    {
      _data.resize(n);
      std::fill(_data.begin(), _data.end(), data_block);
      _is_initialized = true;
    }
    else
    {
      RequireF(LCodeZones::FILE_IO, n == _data.size()
               , "Attempted to initialize static chunk with non-matching size (%d).", n);
      std::fill(_data.begin(), _data.end(), data_block);
      _is_initialized = true;
    }
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Initialize(ArrayImplT const& data_array)
  {
    InvariantF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
    _data = data_array;
    _is_initialized = true;
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Read(ByteBuffer const& buf
                                                                                   , std::size_t size)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading array chunk: %s, size: %d."
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

    EnsureMF(LCodeZones::FILE_IO, (size_min == std::numeric_limits<std::size_t>::max()
                                  || n_elements >= static_cast<std::size_t>(size_min)
        , size_max == std::numeric_limits<std::size_t>::max()
          || static_cast<std::size_t>(n_elements) <= size_max),
        "Expected to read satisfying size constraint (min: %d, max: %d), got size %d instead."
            , size_min, size_max, n_elements);

    buf.Read(_data.begin(), _data.end());

    _is_initialized = true;
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Write(ByteBuffer& buf) const
  {
    if (!_is_initialized) [[unlikely]]
      return;

    LogDebugF(LCodeZones::FILE_IO, "Writing array chunk: %s, length: %d, size: %d."
              , FourCCStr<fourcc, fourcc_reversed>
              , _data.size()
              , _data.size() * sizeof(T));

    RequireF(LCodeZones::FILE_IO, (size_min == std::numeric_limits<std::size_t>::max() || _data.size() >= size_min
        , size_max == std::numeric_limits<std::size_t>::max() || _data.size() <= size_max),
        "Expected to write chunk with size constraint (min: %d, max : %d), got size %d instead."
        , size_min, size_max, _data.size());

    ChunkHeader header {};
    header.fourcc = fourcc;
    EnsureF(CCodeZones::FILE_IO, (_data.size() * sizeof(T)) <= std::numeric_limits<std::uint32_t>::max()
            , "Chunk size overflow.");
    header.size = static_cast<std::uint32_t>(_data.size() * sizeof(T));

    buf.Write(header);

    for (auto& element : _data)
    {
      buf.Write(element);
    }
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline T& DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Add()
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    _is_initialized = true;
    T& ret = _data.emplace_back();
    std::memset(&ret, 0, sizeof(T));
    return ret;
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Remove(std::size_t index)
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds remove of underlying chunk vector element.");
    InvariantF(CCodeZones::FILE_IO, _is_initialized, "Attempted removing on uninitialized chunk.");
    _data.erase(_data.begin() + index);
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Remove(typename ArrayImplT_::iterator it)
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    RequireF(CCodeZones::FILE_IO, it < _data.end(), "Out of bounds remove of underlying chunk vector element.");
    InvariantF(CCodeZones::FILE_IO, _is_initialized, "Attempted removing on uninitialized chunk.");
    _data.erase(it);
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T& DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::At(std::size_t index)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
    InvariantF(CCodeZones::FILE_IO, _is_initialized, "Attempted element access on uninitialized chunk.");
    return _data[index];
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T const& DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::At(std::size_t index) const
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
    InvariantF(CCodeZones::FILE_IO, _is_initialized, "Attempted element access on uninitialized chunk.");
    return _data[index];
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T& DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::operator[](std::size_t index)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
    return _data[index];
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  inline T const& DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::operator[](std::size_t index) const
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds access to underlying chunk vector.");
    return _data[index];
  }

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline void DataArrayChunk<T, fourcc, fourcc_reversed, size_min, size_max>::Clear()
  requires (std::is_same_v<ArrayImplT_, std::vector<T>>)
  {
    _data.clear();
  }

  // StringBlockChunk
  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Initialize()
  {
    RequireF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
    _is_initialized = true;
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Initialize(std::vector<std::string> const& strings)
  requires (type == StringBlockChunkType::NORMAL)
  {
    RequireF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
    _data = strings;
    _is_initialized = true;
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Initialize(std::vector<std::string> const& strings)
  requires (type == StringBlockChunkType::OFFSET)
  {
    RequireF(LCodeZones::FILE_IO, !_is_initialized, "Attempted to initialize an already initialized chunk.");
    _data.resize(strings.size());

    std::uint32_t cur_ofs = 0;
    for (auto const&& [i, string] : future::enumerate(strings))
    {
      _data[i] = std::make_pair(cur_ofs, string);
      cur_ofs += (string.size() + 1);
    }

    _is_initialized = true;
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Read(ByteBuffer const& buf, std::size_t size)
  requires (type == StringBlockChunkType::NORMAL)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading string chunk: %s, size: %d."
              , FourCCStr<fourcc, fourcc_reversed>
              , size);

    std::size_t end_pos = buf.Tell() + size;

    while (buf.Tell() != end_pos)
    {
      _data.emplace_back(buf.ReadString());
    }

    EnsureMF(LCodeZones::FILE_IO
            , (size_min == std::numeric_limits<std::size_t>::max()
               || _data.size()  >= static_cast<std::size_t>(size_min)
            , size_max == std::numeric_limits<std::size_t>::max()
               || _data.size() <= size_max)
                , "Expected to read satisfying size constraint (min: %d, max: %d), got size %d instead."
                , size_min, size_max, _data.size());

    _is_initialized = true;
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Read(ByteBuffer const& buf, std::size_t size)
  requires (type == StringBlockChunkType::OFFSET)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading string chunk: %s, size: %d."
              , FourCCStr<fourcc, fourcc_reversed>
              , size);

    std::size_t start_pos = buf.Tell();
    std::size_t end_pos = start_pos + size;

    while (buf.Tell() != end_pos)
    {
      _data.emplace_back(std::pair<std::uint32_t, std::string>{static_cast<std::uint32_t>(buf.Tell() - start_pos)
                                                               , buf.ReadString()});
    }

    EnsureMF(LCodeZones::FILE_IO
             , (size_min == std::numeric_limits<std::size_t>::max()
                || _data.size()  >= static_cast<std::size_t>(size_min)
             , size_max == std::numeric_limits<std::size_t>::max()
                || _data.size() <= size_max)
                 , "Expected to read satisfying size constraint (min: %d, max: %d), got size %d instead."
                 , size_min, size_max, _data.size());

    // make sure data is in ascending order
    std::sort(_data.begin(), _data.end(), [](auto const& a, auto const& b) -> bool { return a.first < b.first; });
    _is_initialized = true;
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Write(ByteBuffer& buf) const
  {
    if (!_is_initialized) [[unlikely]]
      return;

    LogDebugF(LCodeZones::FILE_IO, "Writing string chunk: %s, length: %d."
              , FourCCStr<fourcc, fourcc_reversed>
              , _data.size());

    InvariantMF(LCodeZones::FILE_IO, (size_min == std::numeric_limits<std::size_t>::max() || _data.size() >= size_min
        , size_max == std::numeric_limits<std::size_t>::max() || _data.size() <= size_max),
        "Expected to write chunk with size constraint (min: %d, max : %d), got size %d instead."
        , size_min, size_max, _data.size());

    std::size_t start_pos = buf.Tell();
    ChunkHeader header{fourcc, 0};
    buf.Write(header);

    if constexpr (type == StringBlockChunkType::NORMAL)
    {
      for (auto& string : _data)
      {
        buf.WriteString(string);
      }
    }
    else
    {
      for (auto& [_, string] : _data)
      {
        buf.WriteString(string);
      }
    }

    std::size_t end_pos = buf.Tell();
    header.size = static_cast<std::uint32_t>(end_pos - start_pos - sizeof(ChunkHeader));
    buf.Seek(start_pos);
    buf.Write(header);
    buf.Seek(end_pos);
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  std::size_t StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::ByteSize() const
  {
    std::size_t size = 0;

    if constexpr(type == StringBlockChunkType::NORMAL)
    {
      for (auto& string : _data)
      {
        size += (string.size() + 1);
      }
    }
    else
    {
      for (auto& [_, string] : _data)
      {
        size += (string.size() + 1);
      }
    }

    return size;
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Add(const std::string& string)
  {
    if constexpr (type == StringBlockChunkType::NORMAL)
    {
      // ensure we do not add the same string more than once
      if (std::find(_data.begin(), _data.end(), string) != _data.end())
      {
        return;
      }

      _data.push_back(string);
    }
    else
    {
      if (_data.empty()) [[unlikely]]
      {
        _data.emplace_back(std::pair<std::uint32_t, std::string>{0, string});
      }
      else
      {
        // ensure we do not add the same string more than once
        if (std::find_if(_data.begin(), _data.end(), [string](auto const& str) -> bool { return str == string; })
          != _data.end())
        {
          return;
        }

        _data.emplace_back(std::pair<std::uint32_t, std::string>
              {_data.back().first + _data.back().second.size() + 1, string}
            );
      }
    }
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Remove(std::size_t index)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds remove.");
    _data.erase(_data.begin() + index);
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Remove(typename ArrayImplT_::iterator it)
  {
    RequireF(CCodeZones::FILE_IO, it < _data.end(), "Out of bounds remove.");

    if constexpr (type == StringBlockChunkType::NORMAL)
    {
      _data.erase(it);
    }
    // fix the following offsets
    else
    {
      std::size_t index = std::distance(_data.begin(), it);
      std::size_t ofs = (*it).first;
      _data.erase(it);

      if (index < _data.size())
      {
        for (auto it_ = _data.begin() + index; it_ != _data.end(); ++it_)
        {
          (*it_).first = ofs;
          ofs += (*it).second.size() + 1;
        }

      }
    }
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Remove(typename ArrayImplT_::const_iterator it)
  {
    RequireF(CCodeZones::FILE_IO, it < _data.cend(), "Out of bounds remove.");

    if constexpr (type == StringBlockChunkType::NORMAL)
    {
      _data.erase(it);
    }
      // fix the following offsets
    else
    {
      std::size_t index = std::distance(_data.cbegin(), it);
      std::size_t ofs = (*it).first;
      _data.erase(it);

      if (index < _data.size())
      {
        for (auto it_ = _data.begin() + index; it_ != _data.end(); ++it_)
        {
          (*it_).first = ofs;
          ofs += (*it).second.size() + 1;
        }

      }
    }
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  void StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::Clear()
  {
    _data.clear();
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  typename ArrayImplT_::value_type const& StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::At(
      std::size_t index) const
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds removed.");
    return _data[index];
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  typename ArrayImplT_::value_type& StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::At(
      std::size_t index)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds removed.");
    return _data[index];
  }

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed
    , std::size_t size_min
    , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline typename ArrayImplT_::value_type const& StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::operator[](
      std::size_t index) const
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds removed.");
    return _data[index];
  }

  template
  <
      StringBlockChunkType type
      , std::uint32_t fourcc
      , FourCCEndian fourcc_reversed
      , std::size_t size_min
      , std::size_t size_max
  >
  template<typename..., typename ArrayImplT_>
  inline typename ArrayImplT_::value_type& StringBlockChunk<type, fourcc, fourcc_reversed, size_min, size_max>::operator[](
      std::size_t index)
  {
    RequireF(CCodeZones::FILE_IO, index < _data.size(), "Out of bounds removed.");
    return _data[index];
  }


}
