#include <IO/ADT/Root/ADTRootMCNK.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

using namespace IO::ADT;
using namespace IO::ADT::ChunkIdentifiers;
using namespace IO::Common;

void MCNKRoot::Read(Common::ByteBuffer const& buf, std::size_t size)
{
  std::size_t end_pos = buf.Tell() + size;

  header.Read(buf);

  while (!buf.Tell() != end_pos)
  {
    ChunkHeader const& chunk_header = buf.ReadView<ChunkHeader>();

    LogDebugF(LCodeZones::ADT_IO, "   Loading ADT root MCNK sub-chunk %c%c%c%c."
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[3]
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[2]
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[1]
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[0]);


    switch (chunk_header.fourcc)
    {
      case ADTRootMCNKSubchunks::MCVT:
        heightmap.Read(buf);
        break;
      case ADTRootMCNKSubchunks::MCLV:
        vertex_lighting.Read(buf);
        break;
      case ADTRootMCNKSubchunks::MCCV:
        vertex_color.Read(buf);
        break;
      case ADTRootMCNKSubchunks::MCNR:
        normals.Read(buf);
        break;
      case ADTRootMCNKSubchunks::MCLQ:
        tbc_water.Read(buf);
        break;
      case ADTRootMCNKSubchunks::MCSE:
        sound_emitters.Read(buf);
        break;
      case ADTRootMCNKSubchunks::MCBB:
        blend_batches.Read(buf);
        break;
      case ADTRootMCNKSubchunks::MCDD:
        groundeffect_disable.Read(buf);
        break;
      default:
      {
        const char* fourcc = reinterpret_cast<const char*>(&chunk_header.fourcc);
        buf.Seek<ByteBuffer::SeekDir::Forward, ByteBuffer::SeekType::Relative>(chunk_header.size);
        LogError("Encountered unknown ADT root MCNK sub-chunk %c%c%c%c.", fourcc[3], fourcc[2], fourcc[1], fourcc[0]);
        break;
      }

    }

  }
}

void MCNKRoot::Write(Common::ByteBuffer& buf)
{

}