#pragma once

#include <IO/ByteBuffer.hpp>
#include <fstream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cstdint>

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

  class IComplexChunk;

  class IDataChunk
  {
  public:
    IDataChunk() = default;
    virtual void Read(ByteBuffer const& buf, std::uint32_t size) = 0;
    virtual void Write(ByteBuffer& buf) = 0;
    
    [[nodiscard]]
    virtual std::uint32_t size() const = 0;

    [[nodiscard]]
    virtual std::uint32_t byte_size() const = 0;

  };

  template<typename T>
  class DataChunk : public IDataChunk
  {
  public:
    DataChunk() = default;

    void Read(ByteBuffer const& buf, std::uint32_t size) override;
    void Write(ByteBuffer& buf) override;

    [[nodiscard]] 
    std::vector<T>& data() { return _data; };

    [[nodiscard]]
    std::uint32_t size() const override { return _data.size(); };

    [[nodiscard]]
    std::uint32_t byte_size() const override;

  private:
    std::vector<T> _data;
  };


  class IComplexChunk
  {
  public:
    IComplexChunk() = default; 

    virtual void Read(ByteBuffer const& buf, std::uint32_t size) = 0;
    virtual void Write(ByteBuffer& buf) const = 0;

    [[nodiscard]]
    virtual std::uint32_t size() const = 0;

    [[nodiscard]]
    virtual std::uint32_t byte_size() const = 0;
  };


  class IChunkedFile
  {
  public:
    IChunkedFile(std::uint32_t file_data_id);

    virtual void Read(ByteBuffer const& buf) = 0;
    virtual void Read(std::fstream& fstream) = 0;
    virtual void Write(ByteBuffer& buf) const = 0;
    virtual void Write(std::fstream& fstream) const = 0;

    [[nodiscard]]
    std::uint32_t file_data_id() const { return _file_data_id; };

  private:
    std::uint32_t _file_data_id;
  };

     
}

#include <IO/Common.inl>