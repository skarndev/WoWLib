#pragma once
#include <IO/ADT/Root/ADTRoot.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <Validation/Log.hpp>
#include <Validation/Contracts.hpp>
#include <Config/CodeZones.hpp>

namespace IO::ADT
{
  template<Common::ClientVersion client_version>
  ADTRoot<client_version>::ADTRoot(std::uint32_t file_data_id)
      : _file_data_id(file_data_id)
  {
  }

  template<Common::ClientVersion client_version>
  ADTRoot<client_version>::ADTRoot(std::uint32_t file_data_id, Common::ByteBuffer const& buf)
      : _file_data_id(file_data_id)
  {
    Read(buf);
  }

  template<Common::ClientVersion client_version>
  void ADTRoot<client_version>::Read(Common::ByteBuffer const& buf)
  {
    LogDebugF(LCodeZones::FILE_IO, "Reading ADT Root. Filedata ID: %d.", _file_data_id);
    LogIndentScoped;

    RequireF(CCodeZones::FILE_IO, !buf.Tell(), "Attempted to read ByteBuffer from non-zero adress.");
    RequireF(CCodeZones::FILE_IO, !buf.IsEof(), "Attempted to read ByteBuffer past EOF.");

    std::size_t chunk_counter = 0;
    Common::DataChunk<DataStructures::MHDR, ChunkIdentifiers::ADTRootChunks::MHDR> header{};

    while (!buf.IsEof())
    {
      auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

      switch (chunk_header.fourcc)
      {
        case ChunkIdentifiers::ADTCommonChunks::MVER:
        {
          Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{};
          version.Read(buf, chunk_header.size);
          EnsureF(CCodeZones::FILE_IO, version.data == 18, "Version must be 18.");
          continue;
        }
        case ChunkIdentifiers::ADTRootChunks::MHDR:
          header.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootChunks::MFBO:
          _flight_bounds.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootChunks::MCNK:
          LogDebugF(LCodeZones::FILE_IO, "Reading chunk: MCNK (root) (%d / 255), size: %d."
                  , chunk_counter, chunk_header.size);
          _chunks[chunk_counter++].Read(buf, chunk_header.size);
          continue;
      }

      // mop+ blend mesh related features
      if constexpr (client_version >= Common::ClientVersion::MOP)
      {
        switch (chunk_header.fourcc)
        {
          case ChunkIdentifiers::ADTRootChunks::MBMH:
            this->_blend_mesh_headers.Read(buf, chunk_header.size);
            continue;
          case ChunkIdentifiers::ADTRootChunks::MBBB:
            this->_blend_mesh_bounding_boxes.Read(buf, chunk_header.size);
            continue;
          case ChunkIdentifiers::ADTRootChunks::MBNV:
            this->_blend_mesh_vertices.Read(buf, chunk_header.size);
            continue;
          case ChunkIdentifiers::ADTRootChunks::MBMI:
            this->_blend_mesh_indices.Read(buf, chunk_header.size);
            continue;
          case ChunkIdentifiers::ADTRootChunks::MH2O:
            _liquids.Read(buf, chunk_header.size);
            continue;
        }
      }

      buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
      LogError("Encountered unknown ADT root chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
      break;

    }

    EnsureF(CCodeZones::FILE_IO, header.IsInitialized(), "Header was not parsed.");
    EnsureF(CCodeZones::FILE_IO, chunk_counter == Common::WorldConstants::CHUNKS_PER_TILE, "Expected exactly 256 MCNKs to be read, got %d instead.", chunk_counter);

    LogDebugF(LCodeZones::FILE_IO, "Done reading ADT Root. Filedata ID: %d.", _file_data_id);
    EnsureF(CCodeZones::FILE_IO, buf.IsEof(), "Not all chunks have been parsed in the file. Bad logic or corrupt file.");
  }

  template<Common::ClientVersion client_version>
  void ADTRoot<client_version>::Write(Common::ByteBuffer& buf) const
  {
    LogDebugF(LCodeZones::FILE_IO, "Writing ADT Root. Filedata ID: %d.", _file_data_id);
    LogIndentScoped;

    Common::DataChunk<std::uint32_t, ChunkIdentifiers::ADTCommonChunks::MVER> version{18};
    version.Write(buf);

    Common::DataChunk<DataStructures::MHDR, ChunkIdentifiers::ADTRootChunks::MHDR> header{};
    header.Initialize();
    std::size_t header_pos = buf.Tell();
    std::size_t header_data_pos = header_pos + 8;
    header.Write(buf);

    if (_liquids.IsInitialized())
    {
      header.data.mh2o = static_cast<std::uint32_t>(buf.Tell() - header_data_pos);
      _liquids.Write(buf);
    }

    for (std::size_t i = 0; i < Common::WorldConstants::CHUNKS_PER_TILE; ++i)
    {
      LogDebugF(LCodeZones::FILE_IO, "Writing chunk: MCNK (root) (%d / 255).", i);
      _chunks[i].Write(buf);
    }

    if (_flight_bounds.IsInitialized())
    {
      header.data.mfbo = static_cast<std::uint32_t>(buf.Tell() - header_data_pos);
      header.data.flags |= DataStructures::MHDRFlags::mhdr_MFBO;
      _flight_bounds.Write(buf);
    }

    // mop+, blend mesh related functionality
    if constexpr (client_version >= Common::ClientVersion::MOP)
    {
      this->_blend_mesh_headers.Write(buf);
      this->_blend_mesh_bounding_boxes.Write(buf);
      this->_blend_mesh_vertices.Write(buf);
      this->_blend_mesh_indices.Write(buf);
    }

    std::size_t end_pos = buf.Tell();

    // go back and write relevant header data
    buf.Seek(header_pos);
    header.Write(buf);
    buf.Seek(end_pos);
  }

}
