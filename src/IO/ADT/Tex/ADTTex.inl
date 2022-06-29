#pragma once
#include <IO/ADT/Tex/ADTTex.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <Config/CodeZones.hpp>


template<IO::Common::FileHandlingPolicy file_policy>
IO::ADT::ADTTex<file_policy>::ADTTex(std::uint32_t file_data_id)
: _file_data_id(file_data_id)
{
  _diffuse_textures.Initialize();

  if constexpr (file_policy == Common::FileHandlingPolicy::FILEDATA_ID)
  {
    this->_height_textures.Initialize();
  }
}

template<IO::Common::FileHandlingPolicy file_policy>
IO::ADT::ADTTex<file_policy>::ADTTex(std::uint32_t file_data_id
               , Common::ByteBuffer const& buf
               , MCAL::AlphaFormat alpha_format
               , std::uint8_t n_alpha_layers
               , bool fix_alphamap)
: _file_data_id(file_data_id)
{
  Read(buf, alpha_format, n_alpha_layers, fix_alphamap);
}

template<IO::Common::FileHandlingPolicy file_policy>
void IO::ADT::ADTTex<file_policy>::Read(IO::Common::ByteBuffer const& buf
                  , MCAL::AlphaFormat alpha_format
                  , std::uint8_t n_alpha_layers
                  , bool fix_alphamap)
{
  LogDebugF(LCodeZones::FILE_IO, "Reading ADT Tex. Filedata ID: %d.", _file_data_id);
  LogIndentScoped;

  RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
  RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

  std::size_t chunk_counter = 0;

  while(!buf.IsEof())
  {
    auto& chunk_header = buf.ReadView<Common::ChunkHeader>();

    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTCommonChunks::MVER:
      {
        Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{};
        version.Read(buf, chunk_header.size);
        EnsureF(CCodeZones::FILE_IO, version.data == 18, "Version must be 18.");
        break;
      }
      case ChunkIdentifiers::ADTTexChunks::MDID:
      {
        _diffuse_textures.Read(buf, chunk_header.size);
        break;
      }
      case ChunkIdentifiers::ADTTexChunks::MHID:
      {
        this->_height_textures.Read(buf, chunk_header.size);
        break;
      }
      case ChunkIdentifiers::ADTTexChunks::MCNK:
      {
        LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCNK (tex) (%d / 255), size: %d."
                 , chunk_counter, chunk_header.size);

        _chunks[chunk_counter++].Read(buf, chunk_header.size, n_alpha_layers, alpha_format, fix_alphamap);
        break;
      }
      default:
      {
        buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
        LogError("Encountered unknown ADT tex chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
        break;
      }
    }
 }
}

template<IO::Common::FileHandlingPolicy file_policy>
IO::ADT::MCNKTex::WriteParams IO::ADT::ADTTex<file_policy>::Write(IO::Common::ByteBuffer& buf, MCAL::AlphaFormat alpha_format) const
{
  LogDebugF(LCodeZones::FILE_IO, "Writing ADT Tex. Filedata ID: %d.", _file_data_id);
  LogIndentScoped;

  InvariantF(CCodeZones::FILE_IO,  _diffuse_textures.IsInitialized()
    , "Attempted writing ADT file (tex) without diffuse textures initialized.");

  Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{18};
  version.Write(buf);

  if constexpr (file_policy == Common::FileHandlingPolicy::FILEDATA_ID)
  {
    InvariantF(CCodeZones::FILE_IO
               , this->_height_textures.IsInitialized()
                 ? _diffuse_textures.Size() == this->_height_textures.Size() : true
               , "Number of diffuse and height textures must match.");
  }

  _diffuse_textures.Write(buf);
  this->_height_textures.Write(buf);

  for (std::size_t i = 0; i < 256; ++i)
  {
    LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MCNK (tex) (%d / 255).", i);
    _chunks[i].Write(buf, alpha_format);
  }

  if (_texture_flags.IsInitialized())
  {
    InvariantF(CCodeZones::FILE_IO, _texture_flags.Size() == _diffuse_textures.Size()
               , "Texture flags array must match the number of textures.");
    _texture_flags.Write(buf);
  }

  if (_texture_params.IsInitialized())
  {
    InvariantF(CCodeZones::FILE_IO, _texture_params.Size() == _diffuse_textures.Size()
               , "Texture params array must match the number of textures.");
    _texture_params.Write(buf);
  }

  _texture_amplifier.Write(buf);

  return {};
}
