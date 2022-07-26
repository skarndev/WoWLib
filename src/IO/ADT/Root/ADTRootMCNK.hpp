#ifndef IO_ADT_ROOT_ADTROOTMCNK_HPP
#define IO_ADT_ROOT_ADTROOTMCNK_HPP

#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/WorldConstants.hpp>
#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <Utils/Misc/ForceInline.hpp>

namespace IO::ADT
{
  /**
   * Implements a VersionTrait enabling terrain blend batches support for MCNKRoot.
   * Blend batches are responsible for seamlessly blending WMOs with terrain.
   */
  class MCNKRootBlendBatches
  {
  public:
    [[nodiscard]] FORCEINLINE auto& BlendBatches()
    {
      return _blend_batches;
    };

    [[nodiscard]] FORCEINLINE auto const& BlendBatches() const
    {
      return _blend_batches;
    };

  protected:
    Common::DataArrayChunk
      <
        DataStructures::MCBB
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCBB
        , Common::FourCCEndian::LITTLE
        , 0
        , 256
      > _blend_batches;

    bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header);
    void Write(Common::ByteBuffer& buf) const;
  };

  template<Common::ClientVersion client_version>
  class MCNKRoot : public Common::Traits::VersionTrait
                                          <
                                            MCNKRootBlendBatches
                                            , client_version
                                            , Common::ClientVersion::MOP
                                          >
  {
  public:
    MCNKRoot();

    void Read(Common::ByteBuffer const& buf, std::size_t size);
    void Write(Common::ByteBuffer& buf) const;

    [[nodiscard]]
    FORCEINLINE constexpr bool IsInitialized() const { return true; };

  private:
    DataStructures::SMChunk _header;
    Common::DataArrayChunk
      <
        float
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCVT
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _heightmap;

    Common::DataArrayChunk
      <
        Common::DataStructures::CArgb
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCLV
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _vertex_lighting;

    Common::DataArrayChunk
      <
        DataStructures::MCCVEntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCCV
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _vertex_color;

    Common::DataArrayChunk
      <
        DataStructures::MCNREntry
        , ChunkIdentifiers::ADTRootMCNKSubchunks::MCNR
        , Common::FourCCEndian::LITTLE
        , Common::WorldConstants::CHUNK_BUF_SIZE
        , Common::WorldConstants::CHUNK_BUF_SIZE
      > _normals;

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
    [[nodiscard]] FORCEINLINE auto const& Normals() const { return _normals; };
  };
}

#include <IO/ADT/Root/ADTRootMCNK.inl>

#endif // IO_ADT_ROOT_ADTROOTMCNK_HPP