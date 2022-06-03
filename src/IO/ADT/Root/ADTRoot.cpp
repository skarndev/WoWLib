#include <IO/ADT/Root/ADTRoot.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <Validation/Log.hpp>
#include <Validation/Contracts.hpp>
#include <Config/CodeZones.hpp>

using namespace IO::ADT;
using namespace IO::ADT::ChunkIdentifiers;
using namespace IO::Common;


ADTRoot::ADTRoot(std::uint32_t file_data_id)
  : _file_data_id(file_data_id)
{


}

void ADTRoot::Read(ByteBuffer const& buf)
{
  LogDebugF(LCodeZones::ADT_IO, "Reading ADT Root. Filedata ID: %d.", _file_data_id);
  RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
  RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");
  
  unsigned chunk_counter = 0;

  while (!buf.IsEof())
  {
    ChunkHeader const& chunk_header = buf.ReadView<ChunkHeader>();

    LogDebugF(LCodeZones::ADT_IO, "Loading ADT root chunk %c%c%c%c."
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[3]
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[2]
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[1]
      , reinterpret_cast<const char*>(&chunk_header.fourcc)[0]);

    switch (chunk_header.fourcc)
    {
      case ADTCommonChunks::MVER:
        version.Read(buf);
        break;
      case ADTRootChunks::MHDR:
        header.Read(buf);
        break;
      case ADTRootChunks::MFBO:
        flight_bounds.Read(buf);
        break;
      case ADTRootChunks::MCNK:
        chunks[chunk_counter++].Read(buf, chunk_header.size);
        break;

      // blend mesh related
      case ADTRootChunks::MBMH:
        blend_mesh_headers.Read(buf);
        break;
      case ADTRootChunks::MBBB:
        blend_mesh_bounding_boxes.Read(buf);
        break;
      case ADTRootChunks::MBNV:
        blend_mesh_vertices.Read(buf);
        break;
      case ADTRootChunks::MBMI:
        blend_mesh_indices.Read(buf);
        break;

      default:
      {
        const char* fourcc = reinterpret_cast<const char*>(&chunk_header.fourcc);
        buf.Seek<ByteBuffer::SeekDir::Forward, ByteBuffer::SeekType::Relative>(chunk_header.size);
        LogError("Encountered unknown ADT root chunk %c%c%c%c.", fourcc[3], fourcc[2], fourcc[1], fourcc[0]);
        break;
      }
    
    }

  }

  LogDebugF(LCodeZones::ADT_IO, "Done ADT Root. Filedata ID: %d.", _file_data_id);
  EnsureF(CCodeZones::FILE_IO, chunk_counter == 256, "Expected exactly 256 MCNKs to be read, got %d instead.", chunk_counter);
  EnsureF(CCodeZones::FILE_IO, buf.IsEof(), "Not all chunks have been parsed in the file.");
}

void ADTRoot::Write(ByteBuffer& buf)
{

}
