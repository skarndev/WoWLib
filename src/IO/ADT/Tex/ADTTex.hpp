#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/Tex/ADTTexMCNK.hpp>
#include <IO/ADT/Tex/MCAL.hpp>

#include <array>
#include <cstdint>

namespace IO::ADT
{
  class ADTTex
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

    Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> _version;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MDID> _diffuse_textures;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MHID> _height_textures;

    std::array<MCNKTex, 16 * 16> _chunks;

    Common::DataArrayChunk<DataStructures::SMTextureFlags, ChunkIdentifiers::ADTTexChunks::MTXF> _texture_flags;
    Common::DataArrayChunk<DataStructures::SMTextureParams, ChunkIdentifiers::ADTTexChunks::MTXP> _texture_params;
    Common::DataChunk<std::uint8_t, ChunkIdentifiers::ADTTexChunks::MAMP> _texture_amplifier;



  };
}