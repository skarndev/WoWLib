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

  namespace details
  {
    struct ADTRootWriteContext
    {
      std::size_t header_pos = 0;
      std::size_t liquid_pos = 0;
      std::size_t mfbo_pos = 0;
    };

  }

  template<Common::ClientVersion client_version>
  class ADTRoot : public Common::Traits::AutoIOTraits
                         <
                           Common::Traits::IOTraits
                           <
                             Common::Traits::VersionTrait
                             <
                               BlendMeshes
                               , client_version
                               , Common::ClientVersion::MOP
                             >
                           >
                           , Common::Traits::DefaultTraitContext
                           , details::ADTRootWriteContext
                         >
                , public Common::Traits::AutoIOTraitInterface
                         <
                           ADTRoot<client_version>
                           , Common::Traits::DefaultTraitContext
                           , details::ADTRootWriteContext
                           , Common::Traits::TraitType::File
                         >
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

  private:

    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry
        <
          &ADTRoot::_version
          , Common::Traits::IOHandlerRead
            <
              nullptr
              , [](ADTRoot* self, auto& ctx, auto& version, Common::ByteBuffer const& buf, std::size_t size)
              {
                InvariantF(CCodeZones::FILE_IO, static_cast<std::uint32_t>(version) == 18, "Version must be 18");
              }
            >
        >
       , Common::Traits::TraitEntry
         <
           &ADTRoot::_header
           , Common::Traits::IOHandlerRead<>
           , Common::Traits::IOHandlerWrite
           <
             [](ADTRoot* self, details::ADTRootWriteContext& ctx, auto& version, Common::ByteBuffer& buf)
             {
                ctx.header_pos = buf.Tell();
             }
           >
         >
       , Common::Traits::TraitEntry
         <
          &ADTRoot::_chunks
          , Common::Traits::IOHandlerRead
            <
              nullptr
              , [](ADTRoot* self, auto& chunks, Common::ByteBuffer const& buf, std::size_t size)
              {
                InvariantF(CCodeZones::FILE_IO
                           , std::all_of(chunks.begin(), chunks.end(), [](auto& chunk) { return chunk.IsInitialized(); })
                           , "Expected to read exactly 256 chunks.");

              }
            >
         >
       , Common::Traits::TraitEntry
         <
           &ADTRoot::_liquids
           , Common::Traits::IOHandlerRead<>
           , Common::Traits::IOHandlerWrite
             <
               [](ADTRoot* self, details::ADTRootWriteContext& ctx, auto& liquids, Common::ByteBuffer& buf)

               {
                 ctx.liquid_pos = buf.Tell();
               }
             >
         >
       , Common::Traits::TraitEntry<&ADTRoot::_flight_bounds>
      >
      , Common::Traits::DefaultTraitContext
      , details::ADTRootWriteContext
   > _auto_trait {};

  };

}

#include <IO/ADT/Root/ADTRoot.inl>

#endif // IO_ADT_ROOT_ADTROOT_HPP