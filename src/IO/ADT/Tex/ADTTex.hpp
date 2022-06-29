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
  class ADTTexWithMHID
  {
  protected:
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MHID> _height_textures;
  };

  class ADTTexWithoutMHID {};

  template<Common::FileHandlingPolicy file_policy>
  class ADTTex
      : public std::conditional_t<file_policy == Common::FileHandlingPolicy::FILEDATA_ID
                                  , ADTTexWithMHID
                                  , ADTTexWithoutMHID
                                 >
  {
  public:
    explicit ADTTex(std::uint32_t file_data_id);
    ADTTex(std::uint32_t file_data_id
           , Common::ByteBuffer const& buf
           , MCAL::AlphaFormat alpha_format
           , std::uint8_t n_alpha_layers
           , bool fix_alphamap);

    void Read(Common::ByteBuffer const& buf
              , MCAL::AlphaFormat alpha_format
              , std::uint8_t n_alpha_layers
              , bool fix_alphamap);

    MCNKTex::WriteParams Write(Common::ByteBuffer& buf, MCAL::AlphaFormat alpha_format) const;

  private:
    std::uint32_t _file_data_id;

    std::conditional_t<file_policy == Common::FileHandlingPolicy::FILEDATA_ID
                       , Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MDID>
                       , Common::DataArrayChunk<std::uint16_t, ChunkIdentifiers::ADTTexChunks::MDID>
                       > _diffuse_textures;

    std::array<MCNKTex, Common::WorldConstants::CHUNKS_PER_TILE> _chunks;

    Common::DataArrayChunk<DataStructures::SMTextureFlags, ChunkIdentifiers::ADTTexChunks::MTXF> _texture_flags;
    Common::DataArrayChunk<DataStructures::SMTextureParams, ChunkIdentifiers::ADTTexChunks::MTXP> _texture_params;
    Common::DataChunk<std::uint8_t, ChunkIdentifiers::ADTTexChunks::MAMP> _texture_amplifier;
  };
}

#include <IO/ADT/Tex/ADTTex.inl>
#endif