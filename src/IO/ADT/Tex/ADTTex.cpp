#include <IO/ADT/Tex/ADTTex.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

using namespace IO::ADT;

ADTTex::ADTTex(std::uint32_t file_data_id)
: _file_data_id(file_data_id)
{

}

ADTTex::ADTTex(std::uint32_t file_data_id
               , Common::ByteBuffer const& buf
               , MCAL::AlphaFormat alpha_format
               , std::uint8_t n_alpha_layers
               , bool fix_alphamap)
: _file_data_id(file_data_id)
{
  Read(buf, alpha_format, n_alpha_layers, fix_alphamap);
}

void ADTTex::Read(IO::Common::ByteBuffer const& buf
                  , MCAL::AlphaFormat alpha_format
                  , std::uint8_t n_alpha_layers
                  , bool fix_alphamap)
{
  std::size_t chunk_counter = 0;

  while(!buf.IsEof())
  {
    auto& chunk_header = buf.ReadView<Common::ChunkHeader>();

    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTCommonChunks::MVER:
      {
        _version.Read(buf, chunk_header.size);
        break;
      }
      case ChunkIdentifiers::ADTTexChunks::MDID:
      {
        _diffuse_textures.Read(buf, chunk_header.size);
        break;
      }
      case ChunkIdentifiers::ADTTexChunks::MHID:
      {
        _height_textures.Read(buf, chunk_header.size);
        break;
      }
      case ChunkIdentifiers::ADTTexChunks::MCNK:
      {
        _chunks[chunk_counter++].Read(buf, chunk_header.size, n_alpha_layers, alpha_format, fix_alphamap);
      }
    }
  }
}
