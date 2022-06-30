#pragma once
#include <IO/ADT/Tex/ADTTex.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <Config/CodeZones.hpp>


template<IO::Common::ClientVersion client_version>
IO::ADT::ADTTex<client_version>::ADTTex(std::uint32_t file_data_id)
: _file_data_id(file_data_id)
{
  this->_diffuse_textures.Initialize();
}

template<IO::Common::ClientVersion client_version>
IO::ADT::ADTTex<client_version>::ADTTex(std::uint32_t file_data_id
               , Common::ByteBuffer const& buf
               , MCAL::AlphaFormat alpha_format
               , bool fix_alphamap)
: _file_data_id(file_data_id)
{
  Read(buf, alpha_format, fix_alphamap);
}

template<IO::Common::ClientVersion client_version>
void IO::ADT::ADTTex<client_version>::Read(IO::Common::ByteBuffer const& buf
                  , MCAL::AlphaFormat alpha_format
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

    // commmon chunks
    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTCommonChunks::MVER:
      {
        Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{};
        version.Read(buf, chunk_header.size);
        EnsureF(CCodeZones::FILE_IO, version.data == 18, "Version must be 18.");
        continue;
      }
      case ChunkIdentifiers::ADTTexChunks::MCNK:
      {
        LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCNK (tex) (%d / 255), size: %d."
                 , chunk_counter, chunk_header.size);

        _chunks[chunk_counter++].Read(buf, chunk_header.size, alpha_format, fix_alphamap);
        continue;
      }
    }

    if constexpr(client_version <= Common::ClientVersion::BFA)
    {
      if (chunk_header.fourcc == ChunkIdentifiers::ADTTexChunks::MTEX)
      {
        this->_diffuse_textures.Read(buf, chunk_header.size);
        continue;
      }
    }
    else
    {
      switch(chunk_header.fourcc)
      {
        case ChunkIdentifiers::ADTTexChunks::MDID:
        {
          this->_diffuse_textures.Read(buf, chunk_header.size);
          continue;
        }
        case ChunkIdentifiers::ADTTexChunks::MHID:
        {
          this->_height_textures.Read(buf, chunk_header.size);
          continue;
        }
      }
    }

    // mop+
    if constexpr (client_version >= Common::ClientVersion::MOP)
    {
      if (chunk_header.fourcc == ChunkIdentifiers::ADTTexChunks::MTXP)
      {
        this->_texture_params.Read(buf, chunk_header.size);
        continue;
      }
    }

    // sl+
    if constexpr (client_version >= Common::ClientVersion::SL)
    {
      if (chunk_header.fourcc == ChunkIdentifiers::ADTTexChunks::MTCG)
      {
        this->_color_grading.Read(buf, chunk_header.size);
        continue;
      }
    }

    buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
    LogError("Encountered unknown ADT tex chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
 }
}

template<IO::Common::ClientVersion client_version>
void  IO::ADT::ADTTex<client_version>::Write(IO::Common::ByteBuffer& buf, MCAL::AlphaFormat alpha_format) const
{
  LogDebugF(LCodeZones::FILE_IO, "Writing ADT Tex. Filedata ID: %d.", _file_data_id);
  LogIndentScoped;

  InvariantF(CCodeZones::FILE_IO,  this->_diffuse_textures.IsInitialized()
    , "Attempted writing ADT file (tex) without diffuse textures initialized.");

  Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{18};
  version.Write(buf);

  this->_diffuse_textures.Write(buf);

  if constexpr (client_version >= Common::ClientVersion::BFA)
  {
    InvariantF(CCodeZones::FILE_IO
               , this->_height_textures.IsInitialized()
                 ? this->_diffuse_textures.Size() == this->_height_textures.Size() : true
               , "Number of diffuse and height textures must match.");

    this->_height_textures.Write(buf);
  }

  for (std::size_t i = 0; i < 256; ++i)
  {
    LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MCNK (tex) (%d / 255).", i);
    _chunks[i].Write(buf, alpha_format);
  }

  if (_texture_flags.IsInitialized())
  {
    InvariantF(CCodeZones::FILE_IO, _texture_flags.Size() == this->_diffuse_textures.Size()
               , "Texture flags array size must match the number of textures.");
    _texture_flags.Write(buf);
  }

  if constexpr (client_version >= Common::ClientVersion::MOP)
  {
    if (this->_texture_params.IsInitialized())
    {
      InvariantF(CCodeZones::FILE_IO, this->_texture_params.Size() == this->_diffuse_textures.Size()
                 , "Texture params array size must match the number of textures.");
      this->_texture_params.Write(buf);
    }
  }
  _texture_amplifier.Write(buf);

  if constexpr (client_version >= Common::ClientVersion::SL)
  {
    if (this->_color_grading.IsInitialized())
    {
      InvariantF(CCodeZones::FILE_IO, this->_color_grading.Size() == this->_diffuse_textures.Size()
                 , "Texture color grading array size must match the number of textures.");
      this->_color_grading.Write(buf);
    }
  }
}
