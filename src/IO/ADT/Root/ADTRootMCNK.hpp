#pragma once
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/WorldConstants.hpp>
#include <IO/Common.hpp>
#include <Utils/Misc/ForceInline.hpp>

namespace IO::ADT
{
  class MCNKRoot
  {
  public:
    MCNKRoot();

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf);

    [[nodiscard]]
    FORCEINLINE bool IsInitialized() const { return true; };

  private:
    DataStructures::SMChunk _header;
    Common::DataArrayChunk
      <
        float
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCVT
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _heightmap;

    Common::DataArrayChunk
      <
        Common::DataStructures::CArgb
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCLV
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _vertex_lighting;

    Common::DataArrayChunk
      <
        DataStructures::MCCVEntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCCV
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _vertex_color;

    Common::DataArrayChunk
      <
        DataStructures::MCNREntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCNR
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNKBUFSIZE
        , Common::WorldConstants::CHUNKBUFSIZE
      > _normals;

    Common::DataArrayChunk
      <
        DataStructures::MCBB
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCBB
        , Common::FourCCEndian::LITTLE
        , 0
        , 256
      > _blend_batches;

    Common::DataChunk<DataStructures::MCLQ, ChunkIdentifiers::ADTRootMCNKSubchunks::MCLQ> _tbc_water;
    Common::DataArrayChunk<DataStructures::MCSE, ChunkIdentifiers::ADTRootMCNKSubchunks::MCSE> _sound_emitters;
    Common::DataChunk<std::uint64_t, ChunkIdentifiers::ADTRootMCNKSubchunks::MCDD> _groundeffect_disable;

  // getters
  public:
    [[nodiscard]] FORCEINLINE auto& Heightmap() { return _heightmap; };
    [[nodiscard]] FORCEINLINE auto const& Heightmap() const { return _heightmap; };

    [[nodiscard]] FORCEINLINE auto& VertexLighting() { return _vertex_lighting; };
    [[nodiscard]] FORCEINLINE auto const& VertexLighting() const { return _vertex_lighting; };

    [[nodiscard]] FORCEINLINE auto& VertexColor() { return _vertex_color; };
    [[nodiscard]] FORCEINLINE auto const& VertexColor() const { return _vertex_color; };

    [[nodiscard]] FORCEINLINE auto& Normals() { return _normals; };
    [[nodiscard]] FORCEINLINE auto const& Normalsr() const { return _normals; };

    [[nodiscard]] FORCEINLINE auto& BlendBatches() { return _blend_batches; };
    [[nodiscard]] FORCEINLINE auto const& BlendBatches() const { return _blend_batches; };


  };
}