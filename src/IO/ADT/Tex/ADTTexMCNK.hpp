#pragma once

#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/WorldConstants.hpp>
#include <IO/Common.hpp>
#include <IO/ADT/Tex/MCSH.hpp>

#include <bitset>

namespace IO::ADT
{
  class ADTTexMCNK
  {
  public:
    ADTTexMCNK();

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf);

    [[nodiscard]]
    bool IsInitialized() const { return true; };

  private:
    Common::DataArrayChunk
      <
        DataStructures::SMLayer
        , ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY
        , Common::FourCCEndian::LITTLE
        , 0
        , 4
      > _alpha_layers;

    MCSH _shadow_map;


  };
}