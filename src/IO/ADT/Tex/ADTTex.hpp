#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>
#include <IO/ADT/Root/ADTRootMCNK.hpp>
#include <IO/ADT/Root/MH2O.hpp>

#include <array>
#include <cstdint>

namespace IO::ADT
{
  class ADTTex
  {
  public:
    ADTTex(std::uint32_t file_data_id);
    ADTTex(std::uint32_t file_data_id, Common::ByteBuffer const& buf);

    void Read(Common::ByteBuffer const& buf);
    void Write(Common::ByteBuffer& buf);

  private:
    std::uint32_t _file_data_id;

    Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> _version;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MDID> _diffuse_textures;
    Common::DataArrayChunk<std::uint32_t, ChunkIdentifiers::ADTTexChunks::MHID> _height_textures;


  };
}