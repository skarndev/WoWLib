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
  // Enables support for FileDataID-based storage of textures.
  class ADTTexTextureStorageFDID
  {
  protected:
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MDID> _diffuse_textures;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MHID> _height_textures;
  };

  // Enables support for filepath-based storage of textures.
  class ADTTexTextureStorageFilepath
  {
  protected:
    Common::StringBlockChunk<Common::StringBlockChunkType::NORMAL
      , ChunkIdentifiers::ADTTexChunks::MTEX> _diffuse_textures;
  };

  // Enables support for texture parameters (scaling / height based blending)
  class ADTTexTextureParameters
  {
  protected:
    Common::DataArrayChunk<DataStructures::SMTextureParams, ChunkIdentifiers::ADTTexChunks::MTXP> _texture_params;
  };
  class ADTTextNoTextureParameters {};

  // Enables support for the color grading feature
  class ADTTexColorGrading
  {
  protected:
    Common::DataArrayChunk<DataStructures::MTCG, ChunkIdentifiers::ADTTexChunks::MTCG> _color_grading;
  };
  class ADTTexNoColorGrading {};

  template<Common::ClientVersion client_version>
  class ADTTex
      : public std::conditional_t<client_version < Common::ClientVersion::BFA
                                  , ADTTexTextureStorageFilepath
                                  , ADTTexTextureStorageFDID
                                 >
      , public std::conditional_t<client_version >= Common::ClientVersion::MOP
                                  , ADTTexTextureParameters
                                  , ADTTextNoTextureParameters
                                 >
      , public std::conditional_t<client_version >= Common::ClientVersion::SL
                                  , ADTTexColorGrading
                                  , ADTTexNoColorGrading
                                 >
  {
  static_assert(client_version >= Common::ClientVersion::CATA && "Split files did not exist before Cataclysm.");
  public:
    explicit ADTTex(std::uint32_t file_data_id);
    ADTTex(std::uint32_t file_data_id
           , Common::ByteBuffer const& buf
           , MCAL::AlphaFormat alpha_format
           , bool fix_alphamap);

    void Read(Common::ByteBuffer const& buf
              , MCAL::AlphaFormat alpha_format
              , bool fix_alphamap);

    void Write(Common::ByteBuffer& buf, MCAL::AlphaFormat alpha_format) const;

  private:
    std::uint32_t _file_data_id;

    std::array<MCNKTex, Common::WorldConstants::CHUNKS_PER_TILE> _chunks;

    Common::DataArrayChunk<DataStructures::SMTextureFlags, ChunkIdentifiers::ADTTexChunks::MTXF> _texture_flags;
    Common::DataChunk<std::uint8_t, ChunkIdentifiers::ADTTexChunks::MAMP> _texture_amplifier;
  };
}

#include <IO/ADT/Tex/ADTTex.inl>
#endif