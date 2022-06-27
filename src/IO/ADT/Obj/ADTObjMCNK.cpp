#include <IO/ADT/Obj/ADTObjMCNK.hpp>

using namespace IO::ADT;


void MCNKObj::Read(IO::Common::ByteBuffer const& buf, std::size_t size)
{
  std::size_t end_pos = buf.Tell() + size;

  while (buf.Tell() != end_pos)
  {
    auto& chunk_header = buf.ReadView<Common::ChunkHeader>();

    switch (chunk_header.fourcc)
    {
      case ChunkIdentifiers::ADTObj0MCNKSubchunks::MCRD:
      {
        _model_references.Read(buf, chunk_header.size);
        break;
      }
      case ChunkIdentifiers::ADTObj0MCNKSubchunks::MCRW:
      {
        _map_object_references.Read(buf, chunk_header.size);
        break;
      }
    }
  }
}

void MCNKObj::Write(IO::Common::ByteBuffer& buf)
{
  InvariantF(CCodeZones::FILE_IO, _model_references.IsInitialized() && _map_object_references.IsInitialized()
           , "Model and map objects references must be initialized");

  std::size_t pos = buf.Tell();

  // writing chunk header to initialize buffer bytes
  Common::ChunkHeader chunk_header {ChunkIdentifiers::ADTObj1Chunks::MCNK, 0};
  buf.Write(chunk_header);

  _model_references.Write(buf);
  _map_object_references.Write(buf);

  std::size_t end_pos = buf.Tell();
  buf.Seek(pos);
  chunk_header.size = static_cast<std::uint32_t>(end_pos - pos);
  buf.Write(chunk_header);
  buf.Seek(end_pos);
}
