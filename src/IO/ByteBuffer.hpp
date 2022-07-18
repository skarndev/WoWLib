#ifndef IO_BYTEBUFFER_HPP
#define IO_BYTEBUFFER_HPP

#include <Utils/Meta/Concepts.hpp>
#include <Validation/Contracts.hpp>

#include <cstdint>
#include <fstream>
#include <ostream>
#include <concepts>
#include <type_traits>
#include <limits>
#include <iterator>

namespace IO::Common
{

  class ByteBuffer
  {

  public:

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

    /**
     * Construct borrowed read-only ByteBuffer given pre-allocated storage.
     * @param data Raw data buffer (const).
     * @param size Size of raw data buffer.
     */
    explicit ByteBuffer(const char* data, std::size_t size);

    /**
     * Constrct borrowed ByteBuffer given pre-allocated storage.
     * @param data Raw data buffer.
     * @param size Size of raw data buffer.
     */
    explicit ByteBuffer(char* data, std::size_t size);

    /**
     * Construct self-owning ByteBuffer given a filestream.
     * @param stream File stream.
     * @param size Number of bytes to read from stream.
     */
    explicit ByteBuffer(std::fstream& stream, std::size_t size);

    /**
     * Construct self-owning ByteBuffer given a filestream. Stream is read till EOF.
     * @param stream File stream.
     */
    explicit ByteBuffer(std::fstream& stream);

    /**
     * Construct a self-owning ByteBuffer.
     * @param size Number of bytes to allocate initially.
     */
    explicit ByteBuffer(std::size_t size = 0);

    /**
     * Move constructor for ByteBuffer.
     * @param other R-value reference to another ByteBuffer.
     */
    ByteBuffer(ByteBuffer&& other) noexcept;

    /**
     * Copy constructor for ByteBuffer.
     * If another ByteBuffer is a borrowed ByteBuffer, the copy becomes self-owning.
     * @param other L-value const reference to another ByteBuffer.
     */
    ByteBuffer(ByteBuffer const& other);

    ~ByteBuffer();

    /**
     * Size of used storage in the buffer (aka size of file).
     * @return Size of the internal buffer.
     */
    [[nodiscard]]
    std::size_t Size() const { return _size; };

    /**
     * Size of allocated storage, equal or more than Size().
     * @return Size of allocated storage.
     */
    [[nodiscard]]
    std::size_t Capacity() const { return _buf_size; };
    
    /**
     * @return Current position in the buffer used for reading / writing.
     */
    [[nodiscard]]
    std::size_t Tell() const { return _cur_pos; };

    /**
     * Pointer to the internal buffer.
     * @return Pointer to the internal buffer.
     */
    [[nodiscard]]
    char* Data() { return _data.get(); };

    /**
     * @return Pointer to the internal buffer (const).
     */
    [[nodiscard]]
    const char* Data() const { return _data.get(); };

    /**
     * Checks if buffer pos is at the end of file.
     * @return True, if EOF was reached, else False.
     */
    [[nodiscard]]
    bool IsEof() const;

    /**
     * Checks if internal buffer is owned by this buffer. Owned buffers are self-destructible,
     * Borrowed buffers are simply released. User needs to take control of memory freeing.
     * @return True, if data is owned by ByteBuffer, else False.
     */
    [[nodiscard]]
    bool IsDataOnwed() const { return _is_data_owned; };

    /**
     * Moves current reading / writing position.
     * @tparam seek_dir Direction to move.
     * @tparam seek_type Type of movement, relative or absolute.
     * @param offset Number of bytes to move.
     */
    template
    <
      SeekDir seek_dir = SeekDir::Forward,
      SeekType seek_type = SeekType::Absolute
    >
    void Seek(std::size_t offset) const;

    /**
     * Peeks into the internal buffer at absolute position and returns read-only view.
     * @tparam T Structure to view.
     * @param offset Offset in bytes relative to current position.
     * @return Structure reference.
     */
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    [[nodiscard]] const T& Peek(std::size_t offset) const;

    /**
     * Reads a constant view representation of object at current position in the buffer.
     * @tparam T Structure to view.
     * @return Structure const reference.
     */
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    [[nodiscard]] const T& ReadView() const;

    /**
     * Reads object representation from buffer at current position and returns its copy.
     * @tparam T Structure to read.
     * @return Read structure.
     */
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    [[nodiscard]] T Read() const;

    // Reads object representation from buffer to a lhs object starting from current position.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Read(T& lhs) const;

    // Reads a range of object representations from buffer starting from current position, identified by byte size.
    template<typename T>
    void Read(T begin, T end) const requires std::contiguous_iterator<T>;

    // Reads object representation from buffer to an lhs object without modifying buffer position.
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Read(T& lhs, std::size_t offset) const;

    // Reads n bytes into provided char array starting at absolute pos
    void Read(char* dest, std::size_t offset, std::size_t n) const;

    // Reads n bytes into provided char array starting at current buffer pos
    void Read(char* dest, std::size_t n) const;

    // Read a null terminated string starting at current buffer pos
    [[nodiscard]]
    std::string_view ReadString() const;

    // Writes n bytes into associated buffer starting at absolute pos (offset)
    void Write(const char* src, std::size_t n, std::size_t offset);

    // Writes n bytes into associated buffer starting at current buffer pos
    void Write(const char* src, std::size_t n);

    // Writes implicit lifetime type T into associated buffer starting at absolute pos
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Write(T const& data, std::size_t offset);

    // Writes implicit lifetime type T into associated buffer starting at current buffer pos
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void Write(T const& data);

    // Writes a null terminated string into associated buffer starting at current buffer pos
    void WriteString(std::string const& data);

    // Writes n implicit lifetime type T objects into associated buffer starting at current buffer pos
    template<Utils::Meta::Concepts::ImplicitLifetimeType T>
    void WriteFill(T const& data, std::size_t n);

    // Writes a range of object representations to buffer
    template<typename T>
    void Write(T begin, T end) requires std::contiguous_iterator<T>;

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
#endif // IO_BYTEBUFFER_HPP
