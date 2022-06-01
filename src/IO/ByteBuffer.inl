#pragma once
#include <IO/ByteBuffer.hpp>
#include <Validation/Contracts.hpp>
#include <Config/CodeZones.hpp>


template
<
  IO::Common::ByteBuffer::SeekDir seek_dir,
  IO::Common::ByteBuffer::SeekType seek_type
>
void IO::Common::ByteBuffer::Seek(std::size_t offset)
{
  if constexpr (seek_type == SeekType::Absolute)
  {
    if constexpr (seek_dir == SeekDir::Forward)
    {
      RequireF(CCodeZones::FILE_IO, offset <= _size, "Seek overflow.");
      _cur_pos = offset;
    }
    else
    {
      RequireF(CCodeZones::FILE_IO, offset <= _size, "Seek underflow.");
      _cur_pos = _size - offset;
    }
  }
  else
  {
    if constexpr (seek_dir == SeekDir::Forward)
    {
      RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - _cur_pos >= offset, "Seek overflow.");
      EnsureF(CCodeZones::FILE_IO, _cur_pos + offset <= _size, "Seek beyond EOF.");
      _cur_pos = _cur_pos + offset;
    }
    else
    {
      Ensure(offset <= _cur_pos, "Seek underflow.");
      _cur_pos = _cur_pos - offset;
    }
  }
}

template<Utils::Meta::Concepts::ImplicitLifetimeType T>
const T& IO::Common::ByteBuffer::Peek(std::size_t offset) const
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - offset >= _cur_pos, "Buffer pos overflow.");
  EnsureF(CCodeZones::FILE_IO, _cur_pos + offset + sizeof(T) <= _size, "Requested read larger than EOF.");
  return *reinterpret_cast<T const*>(_data.get() + offset);
}

template<Utils::Meta::Concepts::ImplicitLifetimeType T>
const T& IO::Common::ByteBuffer::ReadView() const
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - sizeof(T) >= _cur_pos, "Buffer pos overflow.");
  EnsureF(CCodeZones::FILE_IO, _cur_pos + sizeof(T) <= _size, "Requested read larger than EOF.");

  const std::size_t pos = _cur_pos;
  _cur_pos += sizeof(T);

  return *reinterpret_cast<T const*>(_data.get() + pos);
}

template<Utils::Meta::Concepts::ImplicitLifetimeType T>
T IO::Common::ByteBuffer::Read() const
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - sizeof(T) >= _cur_pos, "Buffer pos overflow.");
  EnsureF(CCodeZones::FILE_IO, _cur_pos + sizeof(T) <= _size, "Requested read larger than EOF.");

  const std::size_t pos = _cur_pos;
  _cur_pos += sizeof(T);

  return *reinterpret_cast<T const*>(_data.get() + pos);
}

template<Utils::Meta::Concepts::ImplicitLifetimeType T>
void IO::Common::ByteBuffer::Read(T& lhs) const
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - sizeof(T) >= _cur_pos, "Buffer pos overflow.");
  EnsureF(CCodeZones::FILE_IO, _cur_pos + sizeof(T) <= _size, "Requested read larger than EOF.");

  const std::size_t pos = _cur_pos;
  _cur_pos += sizeof(T);

  std::memcpy(&lhs, reinterpret_cast<T const*>(_data.get() + pos), sizeof(T));
}

template<Utils::Meta::Concepts::ImplicitLifetimeType T>
void IO::Common::ByteBuffer::Read(T& lhs, std::size_t offset) const
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - offset >= _size, "Buffer pos overflow.");
  RequireF(CCodeZones::FILE_IO, offset + sizeof(T) <= _size, "Requested read larger than EOF.");
  std::memcpy(&lhs, reinterpret_cast<T const*>(_data.get() + offset), sizeof(T));
}

template<Utils::Meta::Concepts::ImplicitLifetimeType T>
void IO::Common::ByteBuffer::Write(T& data, std::size_t offset)
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - offset >= sizeof(T), "Buffer size overflow on writing.");

  if ((offset + sizeof(T)) > _size)
  {
    Reserve(offset + sizeof(T) - _size);
  }

  std::memcpy(_data.get() + offset, &data, sizeof(T));
}

template<Utils::Meta::Concepts::ImplicitLifetimeType T>
void IO::Common::ByteBuffer::Write(T& data)
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - _cur_pos >= sizeof(T), "Buffer size overflow on writing.");

  if ((_cur_pos + sizeof(T)) > _size)
  {
    Reserve(_cur_pos + sizeof(T) - _size);
  }

  std::memcpy(_data.get() + _cur_pos, &data, sizeof(T));

  _cur_pos += sizeof(T);
}

template<IO::Common::ByteBuffer::ReservePolicy reserve_policy>
void IO::Common::ByteBuffer::Reserve(std::size_t n)
{
  RequireF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - _size >= n, "Buffer size overflow on attempt to alloc more memory.");
  InvariantF(CCodeZones::FILE_IO, _is_data_owned, "Attempted reserve on a non-owned buffer.");

  if constexpr (reserve_policy == ReservePolicy::Strict)
  {
    if (_buf_size < _size + n)
    {
      _buf_size = _size + n;
      auto realloced_buffer = new char[_buf_size];
      std::memcpy(realloced_buffer, _data.get(), _size);
      _data.reset(realloced_buffer);
    }
    _size += n;

  }
  else if constexpr (reserve_policy == ReservePolicy::Double)
  {
    if (_buf_size < _size + n)
    {
      std::size_t required_at_least = _size + n;
      std::size_t new_size = _buf_size;

      while (new_size < required_at_least)
      {
        EnsureF(CCodeZones::FILE_IO, std::numeric_limits<std::size_t>::max() - new_size >= _buf_size, "Buffer size overflow on attempt to alloc more memory.");
        new_size += _buf_size;
      }

      auto realloced_buffer = new char[new_size];
      std::memcpy(realloced_buffer, _data, _size);
      _data.reset(realloced_buffer);

      _buf_size = new_size;
    }
    _size += n;
  }
}