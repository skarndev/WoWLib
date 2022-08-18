#ifndef IO_ADT_TEX_ADTTEX_HPP
#define IO_ADT_TEX_ADTTEX_HPP

#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/Tex/ADTTexMCNK.hpp>
#include <IO/ADT/Tex/MCAL.hpp>

#include <array>
#include <cstdint>
#include <type_traits>

namespace IO::ADT
{

  struct ADTTexReadContext
  {
    MCAL::AlphaFormat alpha_format;
    bool fix_alphamap;
  };

  struct ADTTexWriteContext
  {
    MCAL::AlphaFormat alpha_format;
    bool fix_alphamap;
  };

  /**
   * Enables support for FileDataID-based storage of textures.
   */
  class ADTTexTextureStorageFDID : public Common::Traits::AutoIOTraitInterface
                                          <
                                            ADTTexTextureStorageFDID
                                            , ADTTexReadContext
                                            , ADTTexWriteContext
                                          >
  {
  protected:
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MDID> _diffuse_textures;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MHID> _height_textures;

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&ADTTexTextureStorageFDID::_diffuse_textures>
        , Common::Traits::TraitEntry<&ADTTexTextureStorageFDID::_height_textures>
      >
      , ADTTexReadContext
      , ADTTexWriteContext
    > _auto_trait {};
  };

  /**
   * Enables support for filepath-based storage of textures.
   */
  class ADTTexTextureStorageFilepath : public Common::Traits::AutoIOTraitInterface
                                              <
                                                ADTTexTextureStorageFilepath
                                                , ADTTexReadContext
                                                , ADTTexWriteContext
                                              >
  {
  protected:
    Common::StringBlockChunk
    <
      Common::StringBlockChunkType::NORMAL
      , ChunkIdentifiers::ADTTexChunks::MTEX
    > _diffuse_textures;

  private:

    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&ADTTexTextureStorageFilepath::_diffuse_textures>
      >
      , ADTTexReadContext
      , ADTTexWriteContext
    > _auto_trait {};
  };

  /**
   * Enables support for texture parameters (scaling / height based blending)
   */
  class ADTTexTextureParameters : public Common::Traits::AutoIOTraitInterface
                                         <
                                           ADTTexTextureParameters
                                           , ADTTexReadContext
                                           , ADTTexWriteContext
                                         >
  {
  protected:
    Common::DataArrayChunk<DataStructures::SMTextureParams, ChunkIdentifiers::ADTTexChunks::MTXP> _texture_params;

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&ADTTexTextureParameters::_texture_params>
      >
      , ADTTexReadContext
      , ADTTexWriteContext
    > _auto_trait {};
  };

  /**
   * Enables support for the color grading feature.
   */
  class ADTTexColorGrading : public Common::Traits::AutoIOTraitInterface
                                    <
                                      ADTTexColorGrading
                                      , ADTTexReadContext
                                      , ADTTexWriteContext
                                    >
  {
  protected:
    Common::DataArrayChunk<DataStructures::MTCG, ChunkIdentifiers::ADTTexChunks::MTCG> _color_grading;

  private:
    static constexpr
    Common::Traits::AutoIOTrait
    <
      Common::Traits::TraitEntries
      <
        Common::Traits::TraitEntry<&ADTTexColorGrading::_color_grading>
      >
    > _auto_trait {};
  };

  template<Common::ClientVersion client_version>
  class ADTTex : public Common::Traits::AutoIOTraits
                        <
                          Common::Traits::IOTraits
                          <
                            Common::Traits::IOTrait
                            <
                              std::conditional_t
                              <
                                client_version < Common::ClientVersion::BFA
                                , ADTTexTextureStorageFilepath
                                , ADTTexTextureStorageFDID
                              >
                            >
                            , Common::Traits::IOTrait
                              <
                                Common::Traits::VersionTrait
                                <
                                  ADTTexColorGrading,
                                  Common::ClientVersion::SL
                                >
                              >
                          >
                          , ADTTexReadContext
                          , ADTTexWriteContext
                        >
              , public Common::Traits::AutoIOTraitInterface
                       <
                         ADTTex<client_version>
                         , ADTTexReadContext
                         , ADTTexWriteContext
                         , Common::Traits::TraitType::File
                       >
  {
  static_assert(client_version >= Common::ClientVersion::CATA && "Split files did not exist before Cataclysm.");
  public:
    explicit ADTTex(std::uint32_t file_data_id);
    ADTTex(std::uint32_t file_data_id
           , Common::ByteBuffer const& buf
           , MCAL::AlphaFormat alpha_format
           , bool fix_alphamap);

  private:

    std::uint32_t _file_data_id;

    std::array<MCNKTex, Common::WorldConstants::CHUNKS_PER_TILE> _chunks;

    Common::DataArrayChunk<DataStructures::SMTextureFlags, ChunkIdentifiers::ADTTexChunks::MTXF> _texture_flags;
    Common::DataChunk<std::uint8_t, ChunkIdentifiers::ADTTexChunks::MAMP> _texture_amplifier;

    static constexpr
    Common::Traits::AutoIOTrait
      <
        Common::Traits::TraitEntries
          <
            Common::Traits::TraitEntry<&ADTTex::_chunks>
            , Common::Traits::TraitEntry<&ADTTex::_texture_flags>
            , Common::Traits::TraitEntry<&ADTTex::_texture_amplifier>
          >
      > _auto_trait {};
  };
}

#include <IO/ADT/Tex/ADTTex.inl>
#endif