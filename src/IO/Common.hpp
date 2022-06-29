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

    return {&fourcc_str[0]};
  }

  enum class FileHandlingPolicy
  {
    FILENAME = 0,
    FILEDATA_ID = 1,
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

    CLASSIC_NEW = 100,
    TBC_NEW = 101,
    WOTLK_NEW = 102
  };

  struct ChunkHeader
  {
    std::uint32_t fourcc;
    std::uint32_t size;
  };

  template<Utils::Meta::Concepts::PODType T, std::uint32_t fourcc, FourCCEndian fourcc_reversed = FourCCEndian::LITTLE>
  struct DataChunk
  {
    typedef std::conditional_t<sizeof(T) <= sizeof(std::size_t), T, T const&> InterfaceType;

    DataChunk() = default;

    explicit DataChunk(InterfaceType data_block);

    void Initialize();

    void Initialize(InterfaceType data_block);

    void Read(ByteBuffer const& buf, std::size_t size);

    void Write(ByteBuffer& buf) const;

    [[nodiscard]]
    std::size_t ByteSize() const { return sizeof(T); };

    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };
   
    T data;

  private:
    bool _is_initialized = false;
  };

  template
  <
    Utils::Meta::Concepts::PODType T
    , std::uint32_t fourcc
    , FourCCEndian fourcc_reversed = FourCCEndian::LITTLE
    , std::size_t size_min = std::numeric_limits<std::size_t>::max()
    , std::size_t size_max = std::numeric_limits<std::size_t>::max()
  >
  struct DataArrayChunk
  {
    using ArrayImplT =  std::conditional_t<size_max == size_min && size_max < std::numeric_limits<std::size_t>::max()
      , std::array<T, size_max>, std::vector<T>>;
    using iterator = typename ArrayImplT::iterator;
    using const_iterator = typename ArrayImplT::const_iterator;

    void Initialize();

    void Initialize(T const& data_block, std::size_t n);

    void Initialize(ArrayImplT const& data_array);

    void Read(ByteBuffer const& buf, std::size_t size);

    void Write(ByteBuffer& buf) const;

    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; };

    [[nodiscard]]
    std::size_t Size() const { return _data.size(); };

    [[nodiscard]]
    std::size_t ByteSize() const { return _data.size() * sizeof(T); };

    template<typename..., typename ArrayImplT_ = ArrayImplT>
    T& Add() requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(std::size_t index) requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    template<typename..., typename ArrayImplT_ = ArrayImplT>
    void Remove(typename ArrayImplT_::iterator& it) requires (std::is_same_v<ArrayImplT_, std::vector<T>>);

    [[nodiscard]]
    T& At(std::size_t index);

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
    ArrayImplT _data;
    bool _is_initialized = false;
  };

  template
  <
     std::uint32_t fourcc
    , FourCCEndian fourcc_reversed = FourCCEndian::LITTLE
    , std::size_t size_min = std::numeric_limits<std::size_t>::max()
    , std::size_t size_max = std::numeric_limits<std::size_t>::max()
  >
  struct StringBlockChunk
  {
    StringBlockChunk() = default;

    void Initialize();

    void Initialize(std::vector<std::string> const& strings);

    void Read(ByteBuffer const& buf, std::size_t size);

    void Write(ByteBuffer& buf) const;

    [[nodiscard]]
    std::vector<std::string> const& Data() { return _data; };

  private:
    bool _is_initialized = false;
    std::vector<std::string> _data;
  };

}
#include <IO/Common.inl>
#endif // IO_COMMON_HPP