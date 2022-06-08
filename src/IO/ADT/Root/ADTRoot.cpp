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
  _version.Initialize(18);
  _header.Initialize();
}

ADTRoot::ADTRoot(std::uint32_t file_data_id, ByteBuffer const& buf)
  : _file_data_id(file_data_id)
{

  Read(buf);
}

void ADTRoot::Read(ByteBuffer const& buf)
{
  LogDebugF(LCodeZones::ADT_IO, "Reading ADT Root. Filedata ID: %d.", _file_data_id);

  {
    LogIndentScoped;

    RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
    RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

    unsigned chunk_counter = 0;

    while (!buf.IsEof())
    {
      ChunkHeader const& chunk_header = buf.ReadView<ChunkHeader>();

      LogDebugF(LCodeZones::ADT_IO, "Loading ADT root chunk %s.", FourCCToStr(chunk_header.fourcc).c_str());

      switch (chunk_header.fourcc)
      {
        case ADTCommonChunks::MVER:
          _version.Read(buf);
          break;
        case ADTRootChunks::MHDR:
          _header.Read(buf);
          break;
        case ADTRootChunks::MFBO:
          _flight_bounds.Read(buf);
          break;
        case ADTRootChunks::MCNK:
          _chunks[chunk_counter++].Read(buf, chunk_header.size);
          break;

          // blend mesh related
        case ADTRootChunks::MBMH:
          _blend_mesh_headers.Read(buf);
          break;
        case ADTRootChunks::MBBB:
          _blend_mesh_bounding_boxes.Read(buf);
          break;
        case ADTRootChunks::MBNV:
          _blend_mesh_vertices.Read(buf);
          break;
        case ADTRootChunks::MBMI:
          _blend_mesh_indices.Read(buf);
          break;

        default:
        {
          buf.Seek<ByteBuffer::SeekDir::Forward, ByteBuffer::SeekType::Relative>(chunk_header.size);
          LogError("Encountered unknown ADT root chunk %s.", FourCCToStr(chunk_header.fourcc));
          break;
        }
      }
    }

    EnsureF(CCodeZones::FILE_IO, chunk_counter == 256, "Expected exactly 256 MCNKs to be read, got %d instead.", chunk_counter);
  }
  
  LogDebugF(LCodeZones::ADT_IO, "Done reading ADT Root. Filedata ID: %d.", _file_data_id);  EnsureF(CCodeZones::FILE_IO, buf.IsEof(), "Not all chunks have been parsed in the file.");
}

void ADTRoot::Write(ByteBuffer& buf)
{
  LogDebugF(LCodeZones::ADT_IO, "Writing ADT Root. Filedata ID: %d.", _file_data_id);
  InvariantF(CCodeZones::FILE_IO, _version.IsInitialized() && _header.IsInitialized(),
    "Attempted writing ADT file without version or header initialized.");

  _version.Write<ADTCommonChunks::MVER>(buf);

  std::size_t header_pos = buf.Tell();
  std::size_t header_data_pos = header_pos + 8;
  _header.Write<ADTRootChunks::MHDR>(buf);
  
  // todo: MH20

  for (int i = 0; i < 256; ++i)
  {
    LogDebugF(LCodeZones::ADT_IO, "Writing ADT Root MCNK (%d / 256).", i);
    _chunks[i].Write(buf);
  }

  if (_flight_bounds.IsInitialized())
  {
    _header.data.mfbo = _header.data.mfbo - buf.Tell();
    _header.data.flags |= DataStructures::MHDRFlags::mhdr_MFBO;
    _flight_bounds.Write<ADTRootChunks::MFBO>(buf);
  }

  // blend mesh stuff
  _blend_mesh_headers.Write<ADTRootChunks::MBMH>(buf);
  _blend_mesh_bounding_boxes.Write<ADTRootChunks::MBBB>(buf);
  _blend_mesh_vertices.Write<ADTRootChunks::MBNV>(buf);
  _blend_mesh_indices.Write<ADTRootChunks::MBMI>(buf);

  std::size_t end_pos = buf.Tell();

  // go back and write relevant header data
  buf.Seek(header_pos);
  _header.Write<ADTRootChunks::MHDR>(buf);
  buf.Seek(end_pos);
}