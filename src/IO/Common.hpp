#ifndef IO_COMMON_HPP
#define IO_COMMON_HPP

#include <IO/ByteBuffer.hpp>
#include <Utils/Meta/Traits.hpp>
#include <Validation/Contracts.hpp>
#include <Validation/Log.hpp>
#include <Config/CodeZones.hpp>

#include <unordered_map>
#include <fstream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cstdint>
#include <algorithm>
#include <limits>

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
    LITTLE = 0, // commonly used order of chars in bytes (little endian, chars sre written right to left in the file).
    BIG = 1 // used in m2 (big endian, chars are written left to right in the file).
  };

  // Converts string representation of FourCC to integer in compile time.
  template <StringLiteral fourcc, FourCCEndian reverse = FourCCEndian::LITTLE>
  static constexpr std::uint32_t FourCC = static_cast<bool>(reverse) ? (fourcc.value[3] << 24 | fourcc.value[2] << 16
       | fourcc.value[1] << 8 | fourcc.value[0])
       : (fourcc.value[0] << 24 | fourcc.value[1] << 16 | fourcc.value[2] << 8 | fourcc.value[3]);

  // Converts integer representation of FourCC to string at compile time.
  template <std::uint32_t fourcc_int, FourCCEndian reverse = FourCCEndian::LITTLE>
  static constexpr char FourCCStr[5] = { static_cast<bool>(reverse) ? fourcc_int & 0xFF : (fourcc_int >> 24) & 0xFF,
                                         static_cast<bool>(reverse) ? (fourcc_int >> 8) & 0xFF
                                          : (fourcc_int >> 16) & 0xFF,
                                         static_cast<bool>(reverse) ? (fourcc_int >> 16) & 0xFF
                                          : (fourcc_int >> 8) & 0xFF,
                                         static_cast<bool>(reverse) ? (fourcc_int >> 24) & 0xFF : fourcc_int & 0xFF,
                                         '\0'
                                       };

  // Converts integer representation of FourCC to string at runtime.
  inline std::string FourCCToStr(std::uint32_t fourcc, bool reverse = false)
  {
    char fourcc_str[5] = { static_cast<char>(reverse ? fourcc & 0xFF : (fourcc >> 24) & 0xFF),
                           static_cast<char>(reverse ? (fourcc >> 8) & 0xFF : (fourcc >> 16) & 0xFF),
                           static_cast<char>(reverse ? (fourcc >> 16) & 0xFF : (fourcc >> 8) & 0xFF),
                           static_cast<char>(reverse ? (fourcc >> 24) & 0xFF : fourcc & 0xFF),
                           '\0'
                         };

    return {&fourcc_str[0]};
  }

  enum class FileHandlingPolicy
  {
    // Files are referenced only by filenames (older clients, MPQ and early CASC)
    FILENAME = 0,
    // Files are referneced by FileDataIDs only (newer clients, Bfa+)
    FILEDATA_ID = 1,
    // Files can be referenced either by FileDataIDs or filenames (mostly Legion, during FDID transition stage).
    MIXED = 2
  };

  enum class ClientVersion
  {
    CLASSIC = 0,
    TBC = 1,
    WOTLK = 2,
    CATA = 3,
    MOP = 4,
    WOD = 5,
    LEGION = 6,
    BFA = 7,
    SL = 8,
    DF = 9,

    // Classic era remasters
    CLASSIC_NEW = 100,
    TBC_NEW = 101,
    WOTLK_NEW = 102
  };

  // Each file chunk starts with this control structure.
  struct ChunkHeader
  {
    std::uint32_t fourcc;
    std::uint32_t size;
  };

  /*
     DataChunk represents a common pattern within WoW files when a chunk contains
     exactly one element of underlying structure T, when header.size == sizeof(T).
     Example: ADT::MHDR and other header-like chunks.
     Convinience conversion operators are provided to cast it to the underlying T.
   */
  template<Utils::Meta::Concepts::PODType T //  Data structure that this chunk holds
      , std::uint32_t fourcc // FourCC identifier
      , FourCCEndian fourcc_reversed = FourCCEndian::LITTLE> // FourCC endianness
  struct DataChunk
  {
    typedef std::conditional_t<sizeof(T) <= sizeof(std::size_t), T, T const&> InterfaceType;

    DataChunk() = default;

    // Construct and initialize the data chunk with an existing structure (copy is made).
    explicit DataChunk(InterfaceType data_block);

    // Initialize the data chunk (underlying structure is default constructed).
    void Initialize();

    // Initialize the data chunk with an existing structure (copy is made).
    void Initialize(InterfaceType data_block);

    // Read the data chunk from ByteBuffer.
    void Read(ByteBuffer const& buf, std::size_t size);

    // Write the data chunk to ByteBuffer.
    void Write(ByteBuffer& buf) const;

    // Size of the data chunk in bytes that it would take when written to file.
    [[nodiscard]]
    std::size_t ByteSize() const { return sizeof(T); };

    // True if chunk was initialized (existing within a file and containing valid data).
    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };

    // These operators are intended for supporting convenient conversions to underlying type
    [[nodiscard]]
    operator T&() { return data; };

    [[nodiscard]]
    operator T const&() const { return data; };

    [[nodiscard]]
    operator T*() { return &data; };

    [[nodiscard]]
    operator const T*() const {return &data; };

    // Underlying data structure
    T data;

  private:
    bool _is_initialized = false;
  };

  /* DataArrayChunk represents a common patter within WoW files where
     a file chunk holds header.size / sizeof(T) instances of T.
     Size constraints are provided to validate and control the max and min
     number of elements in the underlying container, when it is required
     semantically by the file format.

     If both size_min and size_max are the same, the chunk array is optimized
     as a std::array with fixed number of elements, except when both
     are std::numeric_limits<std::size_t>::max() (default). In that case, the array is
     unconstrained (dynamic).

     The array implements an interface simialr to stl containers except providing
     no exceptions. All validation is performed with contracts in debug mode.
  */
  template
  <
    Utils::Meta::Concepts::PODType T // Data structure that this chunk holds
    , std::uint32_t fourcc // FourCC identifier
    , FourCCEndian fourcc_reversed = FourCCEndian::LITTLE // FourCC endianness
    , std::size_t size_min = std::numeric_limits<std::size_t>::max() // Minimum amount of elements stored in the array
    , std::size_t size_max = std::numeric_limits<std::size_t>::max() // Maximum amount of elements stored in the array
  >
  struct DataArrayChunk
  {
    using ArrayImplT =  std::conditional_t<size_max == size_min && size_max < std::numeric_limits<std::size_t>::max()
      , std::array<T, size_max>, std::vector<T>>;
    using iterator = typename ArrayImplT::iterator;
    using const_iterator = typename ArrayImplT::const_iterator;

    // Initialize an empty array chunk
    void Initialize();

    // Initialize the array chunk with n copies of underlying type T.
    void Initialize(T const& data_block, std::size_t n);

    // Initialize the array chunk with existing array of underlying type T (std::vector or std::array).
    void Initialize(ArrayImplT const& data_array);

    // Read the array cunk from ByteBuffer (also initializes it).
    void Read(ByteBuffer const& buf, std::size_t size);

    // Write contents of the array chunk into ByteBuffer.
    void Write(ByteBuffer& buf) const;

    // Returns true if chunk is initialized (has valid data and is present in file).
    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };

    // Returns the number of elements stored in the container.
    [[nodiscard]]
    std::size_t Size() const { return _data.size(); };

    // Returns the number of bytes that this array chunk would take in a file.
    [[nodiscard]]
    std::size_t ByteSize() const { return _data.size() * sizeof(T); };

    // Default constructs a new element in the underlying vector and returns reference to it (dynamic size only).
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    T& Add() requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    // Removes an element by its index in the underlying vector. Bounds checks are debug-only, no exceptions.
    // (dynamic size only).
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(std::size_t index) requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    // Removes an element by its iterator in the underlying vector. Bounds checks are debug-only, no exceptions.
    // (dynamic size only).
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(typename ArrayImplT_::iterator it) requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    // Clears the underlying vector (dynamic size only).
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Clear() requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    // Returns reference to the element of the underlying vector by its index.
    // Bounds checks are debug-only. No difference to [] operator.
    [[nodiscard]]
    T& At(std::size_t index);

    [[nodiscard]]
    T const& At(std::size_t index) const;

    [[nodiscard]]
    typename ArrayImplT::const_iterator begin() const { return _data.cbegin(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator end() const { return _data.cend(); };

    [[nodiscard]]
    typename ArrayImplT::iterator begin() { return _data.begin(); };

    [[nodiscard]]
    typename ArrayImplT::iterator end() { return _data.end(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator cbegin() const { return _data.cbegin(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator cend() const { return _data.cend(); };

    [[nodiscard]]
    T& operator[](std::size_t index);

    [[nodiscard]]
    T const& operator[](std::size_t index) const;

  private:
    // Array of data structures (either std::vector or std::array depending on size constraints).
    ArrayImplT _data;

    bool _is_initialized = false;
  };

  /* StringBlockChunk represents a common pattern within WoW files where a chunk is an
     array of 0-terminated strings. It provides similar interface and options to DataArrayChunk.
   */

  enum class StringBlockChunkType
  {
    // Simple array of null-terminated strings
    NORMAL = 0,

    // Offset map of null-terminated strings
    OFFSET = 1
  };

  template
  <
    StringBlockChunkType type
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed = FourCCEndian::LITTLE
    , std::size_t size_min = std::numeric_limits<std::size_t>::max()
    , std::size_t size_max = std::numeric_limits<std::size_t>::max()
  >
  struct StringBlockChunk
  {
    using ArrayImplT = std::conditional_t<type == StringBlockChunkType::NORMAL, std::vector<std::string>
        , std::vector<std::pair<std::uint32_t, std::string>>>;

    StringBlockChunk() = default;

    void Initialize();

    void Initialize(std::vector<std::string> const& strings) requires (type == StringBlockChunkType::NORMAL);
    void Initialize(std::vector<std::string> const& strings) requires (type == StringBlockChunkType::OFFSET);

    void Read(ByteBuffer const& buf, std::size_t size) requires (type == StringBlockChunkType::NORMAL);
    void Read(ByteBuffer const& buf, std::size_t size) requires (type == StringBlockChunkType::OFFSET);

    void Write(ByteBuffer& buf) const;

    // Returns true if chunk is initialized (has valid data and is present in file).
    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };

    // Returns the number of elements stored in the container.
    [[nodiscard]]
    std::size_t Size() const { return _data.size(); };

    // Returns the number of bytes that this array chunk would take in a file.
    [[nodiscard]]
    std::size_t ByteSize() const;

    // Pushes a string to the end of the underlying vector
    // Ensures uniqueness for the offset map variant
    void Add(std::string const& string);

    // Removes an element by its index in the underlying vector. Bounds checks are debug-only, no exceptions.
    void Remove(std::size_t index);

    // Removes an element by its iterator in the underlying vector. Bounds checks are debug-only, no exceptions.
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(typename ArrayImplT_::iterator it);

    // Removes an element by its iterator in the underlying vector. Bounds checks are debug-only, no exceptions.
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(typename ArrayImplT_::const_iterator it);

    // Clears the underlying vector.
    void Clear();

    // Returns reference to the element of the underlying vector by its index.
    // Bounds checks are debug-only. No difference to [] operator.
    template<typename..., typename ArrayImplT_ = ArrayImplT>
    [[nodiscard]]
    typename ArrayImplT_::value_type const& At(std::size_t index) const;

    template<typename..., typename ArrayImplT_ = ArrayImplT>
    [[nodiscard]]
    typename ArrayImplT_::value_type& At(std::size_t index);

    [[nodiscard]]
    typename ArrayImplT::const_iterator begin() const { return _data.cbegin(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator end() const { return _data.cend(); };

    [[nodiscard]]
    typename ArrayImplT::iterator begin() { return _data.begin(); };

    [[nodiscard]]
    typename ArrayImplT::iterator end() { return _data.end(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator cbegin() const { return _data.cbegin(); };

    [[nodiscard]]
    typename ArrayImplT::const_iterator cend() const { return _data.cend(); };

    template<typename..., typename ArrayImplT_ = ArrayImplT>
    [[nodiscard]]
    typename ArrayImplT_::value_type const& operator[](std::size_t index) const;

    template<typename..., typename ArrayImplT_ = ArrayImplT>
    [[nodiscard]]
    typename ArrayImplT_::value_type& operator[](std::size_t index);


  private:
    bool _is_initialized = false;
    ArrayImplT _data;
  };

}
#include <IO/Common.inl>
#endif // IO_COMMON_HPP