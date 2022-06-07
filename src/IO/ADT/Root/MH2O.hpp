#pragma once
#include <IO/ADT/DataStructures.hpp>
#include <IO/Common.hpp>

#include <vector>

namespace IO::ADT
{
  class LiquidChunk
  {
  public:
    [[nodiscard]]
    std::vector<DataStructures::SMLiquidInstance>& layers() { return _layers; };
    
    [[nodiscard]]
    DataStructures::SMLiquidChunkAttributes& attributes() { return _attributes; };

  private:
    std::vector<DataStructures::SMLiquidInstance> _layers;
    DataStructures::SMLiquidChunkAttributes _attributes;
  };

  class MH2O
  {
  public:
    MH2O() = default;

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf);

    [[nodiscard]]
    bool IsInitialized() const { return _is_initialized; }


  private:
    DataStructures::SMLiquidChunk _header;
    bool _is_initialized = false;

  };
}