#include <IO/ADT/Root/ADTRoot.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <Validation/Log.hpp>

using namespace IO::ADT;

ADTRoot::ADTRoot(std::uint32_t file_data_id)
{


}

void ADTRoot::Read(Common::ByteBuffer const& buf)
{
  Require(!buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
  Require(!buf.IsEof(), "Attempted to read ByteBuffer past EOF.");
  
  while (!buf.IsEof())
  {
    Common::ChunkHeader header = buf.Read<Common::ChunkHeader>();

    switch (header.fourcc)
    {
      case ChunkIdentifiers::ADTCommonChunks::MVER:
        _version.Read(buf);
        break;
      case ChunkIdentifiers::ADTRootChunks::MHDR:
        _header.Read(buf);
        break;
      case ChunkIdentifiers::ADTRootChunks::MFBO:
        _flight_bounds.Read(buf);
        break;

      // blend mesh related
      case ChunkIdentifiers::ADTRootChunks::MBMH:
        _blend_mesh_headers.Read(buf);
        break;
      case ChunkIdentifiers::ADTRootChunks::MBBB:
        _blend_mesh_bounding_boxes.Read(buf);
        break;
      case ChunkIdentifiers::ADTRootChunks::MBNV:
        _blend_mesh_vertices.Read(buf);
        break;
      case ChunkIdentifiers::ADTRootChunks::MBMI:
        _blend_mesh_indices.Read(buf);
        break;

      default:
      {
        char* fourcc = reinterpret_cast<char*>(&header.fourcc);
        buf.Seek()
        LogDebug("Encountered unknown chunk %c%c%c%c (native byte order). ", fourcc[0], fourcc[1], fourcc[2], fourcc[3]);
        
        
        break;
      }
    
    }

  }

  Ensure(buf.IsEof(), "Not all chunks have been parsed in the file.");
}

void ADTRoot::Write(Common::ByteBuffer& buf)
{

}
