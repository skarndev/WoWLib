#pragma once

#include <Utils/Meta/Concepts.hpp>
#include <Validation/Contracts.hpp>

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

    // Size of the buffer
    [[nodiscard]]
    std::size_t Size() const { return _size; };
    
    // Current position in the buffer
    [[nodiscard]]
    std::size_t Tell() const { return _cur_pos; };

    // Associated internal data buffer
    [[nodiscard]]
    char* GetData() { return _data.get(); };

    // Report if pos is at EOF
    [[nodiscard]]
    bool IsEof() const { return _cur_pos == _size; };

    // Returns true if data is owned by this buffer
    [[nodiscard]]
    bool IsDataOnwed() const { return _is_data_owned; };

    // Seeks the position within current buffer
    // If resulted pos is larger than EOF, returns EOF.
    template
    <
      SeekDir seek_dir = SeekDir::Forward,
      SeekType seek_type = SeekType::Absolute
    >
    void Seek(std::size_t offset);

    // Peaks into the internal buffer at absolute position and returns read-only view.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    const T& Peek(std::size_t offset) const;

    // Reads a constant view representation of object at current position in the buffer.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    const T& ReadView() const;

    // Reads object representation from buffer at current position and returns its copy.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    T Read() const;

    // Reads object representation from buffer to an lhs object starting from current position.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Read(T& lhs) const;

    // Reads object representation from buffer to an lhs object without modifying buffer position.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Read(T& lhs, std::size_t offset) const;

    // Reads n bytes into provided char array starting at absolute pos
    void Read(char* dest, std::size_t offset, std::size_t n) const;

    // Reads n bytes into provided char array starting at current buffer pos
    void Read(char* dest, std::size_t n) const;

    // Writes n bytes into associated buffer starting at absolute pos (offset)
    void Write(const char* src, std::size_t n, std::size_t offset);

    // Writes n bytes into associated buffer starting at current buffer pos
    void Write(const char* src, std::size_t n);

    // Writes implicit life time type T into associated buffer starting at absolute pos
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Write(T const& data, std::size_t offset);

    // Writes implicit life time type T into associated buffer starting at current buffer pos
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Write(T const& data);

    // Reserves b bytes in the associated buffer
    // Can only be used for the cases when the associated buffer is owned by a ByteBuffer instance.
    // ReservePolicy::Strict (default) ensures only the amout of memory to store current buffer size + n requested extra bytes is allocated.
    // ReservePolicy::Double performs bucket allocations, and ensures that at least current buffer size + n requested extra bytes is allocated.
    template<ReservePolicy reserve_policy = ReservePolicy::Strict>
    void Reserve(std::size_t n);

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

#include <IO/ByteBuffer.inl>