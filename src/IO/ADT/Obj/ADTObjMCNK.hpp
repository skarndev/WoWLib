#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

#include <cstdint>

namespace IO::ADT
{
  class MCNKObj
  {
  public:
    MCNKObj() = default;

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf);

  private:
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0MCNKSubchunks::MCRD> _model_references;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTObj0MCNKSubchunks::MCRW> _map_object_references;
  };
}