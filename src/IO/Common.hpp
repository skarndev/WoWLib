#pragma once

#include <fstream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <cstdint>

namespace IO::Common
{
  struct ChunkHeader
  {
    std::uint32_t fourcc;
    std::uint32_t size;
  };

  class IComplexChunk;

  template<typename T>
  class DataChunk
  {
  public:
    DataChunk() = default;

    void Read(std::fstream const& fstream, std::uint32_t size) requires (std::is_trivial<T>::value && std::is_standard_layout<T>::value);
    void Read(std::fstream const& fstream, std::uint32_t size) requires std::derived_from<T, IComplexChunk>;
    void Write(std::fstream const& fstream) const requires (std::is_trivial<T>::value && std::is_standard_layout<T>::value);
    void Write(std::fstream const& fstream) const requires std::derived_from<T, IComplexChunk>;

    [[nodiscard]] 
    std::vector<T>& data() { return _data; };

    [[nodiscard]]
    std::uint32_t size() const { return _data.size(); };

    [[nodiscard]]
    std::uint32_t byte_size() const requires (std::is_trivial<T>::value && std::is_standard_layout<T>::value);

    [[nodiscard]]
    std::uint32_t byte_size() const requires std::derived_from<T, IComplexChunk>;

  private:
    std::vector<T> _data;
  };


  class IComplexChunk
  {
  public:
    IComplexChunk() = default; 

    virtual void Read(std::fstream const& fstream, std::uint32_t size) = 0;
    virtual void Write(std::fstream const& fstream) const = 0;

    [[nodiscard]]
    virtual std::uint32_t size() const = 0;

    [[nodiscard]]
    virtual std::uint32_t byte_size() const = 0;
  };


  class IChunkedFile
  {
  public:
    IChunkedFile(std::uint32_t file_data_id);

    virtual void Read(std::fstream const& fstream) = 0;
    virtual void Write(std::fstream const& fstream) const = 0;

    [[nodiscard]]
    std::uint32_t file_data_id() const { return _file_data_id; };

  private:
    std::uint32_t _file_data_id;
  };

     
}

#include <IO/Common.inl>