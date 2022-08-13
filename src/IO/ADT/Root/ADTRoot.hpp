#ifndef IO_ADT_ROOT_ADTROOT_HPP
#define IO_ADT_ROOT_ADTROOT_HPP
#include <IO/Common.hpp>
#include <IO/CommonTraits.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/Root/ADTRootMCNK.hpp>
#include <IO/ADT/Root/MH2O.hpp>

#include <array>
#include <cstdint>

namespace IO::ADT
{
  /**
   * Implements a VersionTrait enabling terrain blend batches support for ADTRoot.
   * Blend batches are responsible for seamlessly blending WMOs with terrain.
   */
  class BlendMeshes
  {
  protected:
    Common::DataArrayChunk<DataStructures::MBMH, ChunkIdentifiers::ADTRootChunks::MBMH> _blend_mesh_headers;
    Common::DataArrayChunk<DataStructures::MBBB, ChunkIdentifiers::ADTRootChunks::MBBB> _blend_mesh_bounding_boxes;
    Common::DataArrayChunk<DataStructures::MBNV, ChunkIdentifiers::ADTRootChunks::MBNV> _blend_mesh_vertices;
    Common::DataArrayChunk<std::uint16_t, ChunkIdentifiers::ADTRootChunks::MBMI> _blend_mesh_indices;

    bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header);
    void Write(Common::ByteBuffer& buf) const;
  };

  template<Common::ClientVersion client_version>
  class ADTRoot : public Common::Traits::IOTraits
                         <
                            Common::Traits::VersionTrait
                            <
                              BlendMeshes
                              , client_version
                              , Common::ClientVersion::MOP
                            >
                         >
                , public Common::Traits::AutoIOTraitInterface<ADTRoot<client_version>, Common::Traits::TraitType::File>
  {

  public:
    explicit ADTRoot(std::uint32_t file_data_id);
    ADTRoot(std::uint32_t file_data_id, Common::ByteBuffer const& buf);

    [[nodiscard]]
    std::uint32_t FileDataID() const { return _file_data_id; };
    
    void Write(Common::ByteBuffer& buf) const;

  private:
    std::uint32_t _file_data_id;

    Common::DataChunk<DataStructures::MVER, ChunkIdentifiers::ADTCommonChunks::MVER> _version;
    Common::DataChunk<DataStructures::MHDR, ChunkIdentifiers::ADTRootChunks::MHDR> _header;
    Common::SparseChunkArray<MCNKRoot<client_version>, 256, 256> _chunks;
    MH2O _liquids;
    Common::DataChunk<DataStructures::MFBO, ChunkIdentifiers::ADTRootChunks::MFBO> _flight_bounds;

    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntry
      <
        &ADTRoot::_version
        , Common::Traits::IOHandler
          <
            nullptr
            , [](ADTRoot* self, auto& version, Common::ByteBuffer const& buf, std::size_t size)
            {
              InvariantF(CCodeZones::FILE_IO, static_cast<std::uint32_t>(version) == 18, "Version must be 18");
            }
          >
      >
     , Common::Traits::TraitEntry<&ADTRoot::_header>
     , Common::Traits::TraitEntry
       <
        &ADTRoot::_chunks
        , Common::Traits::IOHandler
          <
            nullptr
            , [](ADTRoot* self, auto& chunks, Common::ByteBuffer const& buf, std::size_t size)
            {
              InvariantF(CCodeZones::FILE_IO
                         , chunks[0].IsInitialized() && chunks[255].IsInitialized()
                         , "Expected to read exactly 256 chunks.");
            }
          >
       >
     , Common::Traits::TraitEntry<&ADTRoot::_liquids>
     , Common::Traits::TraitEntry<&ADTRoot::_flight_bounds>
    > _auto_trait {};

  };

}

#include <IO/ADT/Root/ADTRoot.inl>

#endif // IO_ADT_ROOT_ADTROOT_HPP