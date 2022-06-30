#include <IO/ADT/Tex/ADTTexMCNK.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <IO/Common.hpp>


using namespace IO::ADT;
using namespace IO::Common;

void MCNKTex::Write(IO::Common::ByteBuffer& buf, MCAL::AlphaFormat alpha_format) const
{
  InvariantF(CCodeZones::FILE_IO, _alpha_layers.IsInitialized(), "MCLY should be initialized to write TEX MCNK.");
  _alpha_layers.Write(buf);
  _alphamaps.Write(buf, alpha_format, _alpha_layers);

  if (_shadowmap.IsInitialized())
  {
    _shadowmap.Write(buf);
  }
}

void MCNKTex::Read(IO::Common::ByteBuffer const& buf
                   , std::size_t size
                   , MCAL::AlphaFormat alpha_format
                   , bool fix_alphamap)
{
  std::size_t end_pos = buf.Tell() + size;

  while (buf.Tell() != end_pos)
  {
    auto const& chunk_header = buf.ReadView<ChunkHeader>();

    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTTexMCNKSubchunks::MCLY:
      {
        _alpha_layers.Read(buf, chunk_header.size);
        break;
      }
      case ChunkIdentifiers::ADTTexMCNKSubchunks::MCAL:
      {
        InvariantF(CCodeZones::FILE_IO, _alpha_layers.IsInitialized(), "MCLY should be processed first.");
        _alphamaps.Read(buf, chunk_header.size, alpha_format, _alpha_layers, fix_alphamap);
        break;
      }
      case ChunkIdentifiers::ADTTexMCNKSubchunks::MCSH:
      {
        _shadowmap.Read(buf, chunk_header.size, fix_alphamap);
        break;
      }
    }
  }
}
