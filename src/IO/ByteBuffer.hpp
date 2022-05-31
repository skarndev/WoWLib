#pragma once

#include <Utils/Meta/Concepts.hpp>
#include <bertrand/bertrand.hpp>

#include <cstdint>
#include <fstream>
#include <ostream>
#include <concepts>
#include <type_traits>
#include <limits>

namespace IO::Common
{
  class ByteBuffer
  {
    enum class SeekDir
    {
      Forward,
      Backwards
    };

    enum class SeekType
    {
      Absolute,
      Relative
    };

    enum class ReservePolicy
    {
      Strict,
      Double
    };

  public:
    explicit ByteBuffer(const char* data, std::size_t size);
    explicit ByteBuffer(char* data, std::size_t size);
    explicit ByteBuffer(std::fstream& stream, std::size_t size);
    explicit ByteBuffer(std::fstream& stream);
    explicit ByteBuffer(std::size_t size = 0);
    ~ByteBuffer();

    [[nodiscard]]
    std::size_t Size() const { return _size; };

    [[nodiscard]]
    std::size_t Tell() const { return _cur_pos; };

    [[nodiscard]]
    char* GetData() { return _data.get(); };

    [[nodiscard]]
    bool IsEof() const { return _cur_pos == _size; };

    // Seeks the position within current buffer
    // If resulted pos is larger than EOF, returns EOF.
    // If direction is SeekDir::Backwards and resulted pos is less than 0, returns 0.
    template
    <
      SeekDir seek_dir = SeekDir::Forward,
      SeekType seek_type = SeekType::Absolute
    >
    void Seek(std::size_t offset)
    {
      if constexpr (seek_type == SeekType::Absolute)
      {
        if constexpr (seek_dir == SeekDir::Forward)
        {
          Require(offset <= _size, "Seek overflow.");
          _cur_pos = offset;
        }
        else
        {
          Require(offset <= _size, "Seek underflow.");
          _cur_pos = _size - offset;
        }
      }
      else
      {
        if constexpr (seek_dir == SeekDir::Forward)
        {
          Require(std::numeric_limits<std::size_t>::max() - _cur_pos >= offset, "Seek overflow.");
          Ensure(_cur_pos + offset <= _size, "Seek beyond EOF.");
          _cur_pos = _cur_pos + offset;
        }
        else
        {
          Ensure(offset <= _cur_pos, "Seek underflow.");
          _cur_pos = _cur_pos - offset;
        }
      }
    }

    // Peaks into the internal buffer at absolute position and returns read-only view.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    const T& Peek(std::size_t offset) const
    {
      Require(std::numeric_limits<std::size_t>::max() - offset >= _cur_pos, "Buffer pos overflow.");
      Ensure(_cur_pos + offset + sizeof(T) <= _size, "Requested read larger than EOF.");
      return *reinterpret_cast<T const*>(_data.get() + offset);
    }

    // Reads a constant view representation of object at current position in the buffer.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    const T& ReadView() const
    {
      Require(std::numeric_limits<std::size_t>::max() - sizeof(T) >= _cur_pos, "Buffer pos overflow.");
      Ensure(_cur_pos + sizeof(T) <= _size, "Requested read larger than EOF.");

      const std::size_t pos = _cur_pos;
      _cur_pos += sizeof(T);

      return *reinterpret_cast<T const*>(_data.get() + pos);
    }

    // Reads object representation from buffer at current position and returns its copy.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    T Read() const
    {
      Require(std::numeric_limits<std::size_t>::max() - sizeof(T) >= _cur_pos, "Buffer pos overflow.");
      Ensure(_cur_pos + sizeof(T) <= _size, "Requested read larger than EOF.");

      const std::size_t pos = _cur_pos;
      _cur_pos += sizeof(T);

      return *reinterpret_cast<T const*>(_data.get() + pos);
    }

    // Reads object representation from buffer to an lhs object starting from current position.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Read(T& lhs) const
    {
      Require(std::numeric_limits<std::size_t>::max() - sizeof(T) >= _cur_pos, "Buffer pos overflow.");
      Ensure(_cur_pos + sizeof(T) <= _size, "Requested read larger than EOF.");

      const std::size_t pos = _cur_pos;
      _cur_pos += sizeof(T);

      std::memcpy(&lhs, reinterpret_cast<T const*>(_data.get() + pos), sizeof(T));
    }

    // Reads object representation from buffer to an lhs object without modifying buffer position.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Read(T& lhs, std::size_t offset) const
    {
      Require(std::numeric_limits<std::size_t>::max() - offset >= _size, "Buffer pos overflow.");
      Require(offset + sizeof(T) <= _size, "Requested read larger than EOF.");
      std::memcpy(&lhs, reinterpret_cast<T const*>(_data.get() + offset), sizeof(T));
    }

    // Reads n bytes into provided char array starting at absolute pos
    void Read(char* dest, std::size_t offset, std::size_t n) const;

    // Reads n bytes into provided char array starting at current buffer pos
    void Read(char* dest, std::size_t n) const;

    // Writes n bytes into associated buffer starting at absolute pos (offset)
    void Write(char* src, std::size_t n, std::size_t offset);

    // Writes n bytes into associated buffer starting at current buffer pos
    void Write(char* src, std::size_t n);

    // Writes implicit life time type T into associated buffer starting at absolute pos
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Write(T& data, std::size_t offset)
    {
      Require(std::numeric_limits<std::size_t>::max() - offset >= sizeof(T), "Buffer size overflow on writing.");

      if ((offset + sizeof(T)) > _size)
      {
        Reserve(offset + sizeof(T) - _size);
      }

      std::memcpy(_data.get() + offset, &data, sizeof(T));
    }

    // Writes implicit life time type T into associated buffer starting at current buffer pos
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Write(T& data)
    {
      Require(std::numeric_limits<std::size_t>::max() - _cur_pos >= sizeof(T), "Buffer size overflow on writing.");

      if ((_cur_pos + sizeof(T)) > _size)
      {
        Reserve(_cur_pos + sizeof(T) - _size);
      }

      std::memcpy(_data.get() + _cur_pos, &data, sizeof(T));

      _cur_pos += sizeof(T);
    }

    // Reserves b bytes in the associated buffer
    // Can only be used for the cases when the associated buffer is owned by a ByteBuffer instance.
    // ReservePolicy::Strict (default) ensures only the amout of memory to store current buffer size + n requested extra bytes is allocated.
    // ReservePolicy::Double performs bucket allocations, and ensures that at least current buffer size + n requested extra bytes is allocated.
    template<ReservePolicy reserve_policy = ReservePolicy::Strict>
    void Reserve(std::size_t n)
    {
      Require(std::numeric_limits<std::size_t>::max() - _size >= n, "Buffer size overflow on attempt to alloc more memory.");
      Invariant(_is_data_owned, "Attempted reserve on a non-owned buffer.");

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
            Ensure(std::numeric_limits<std::size_t>::max() - new_size >= _buf_size, "Buffer size overflow on attempt to alloc more memory.");
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

    // Flushes associated buffer into std::fstream
    void Flush(std::fstream& stream) const;

    // Flushes associated buffer into std::ostream
    void Flush(std::ostream& stream) const;

   
  private:
    bool _is_data_owned;
    mutable std::size_t _cur_pos;
    std::size_t _size;
    std::size_t _buf_size;
    std::unique_ptr<char> _data;

  };
}