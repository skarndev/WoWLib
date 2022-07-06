#pragma once
#include <IO/ADT/Root/ADTRootMCNK.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

namespace IO::ADT
{
  template<Common::ClientVersion client_version>
  MCNKRoot<client_version>::MCNKRoot()
  {
    _heightmap.Initialize();
    _normals.Initialize();
  }

  template<Common::ClientVersion client_version>
  void MCNKRoot<client_version>::Read(Common::ByteBuffer const& buf, std::size_t size)
  {
    std::size_t end_pos = buf.Tell() + size;

    buf.Read(_header);

    while (buf.Tell() != end_pos)
    {
      auto const& chunk_header = buf.ReadView<Common::ChunkHeader>();

      switch (chunk_header.fourcc)
      {
        case ChunkIdentifiers::ADTRootMCNKSubchunks::MCVT:
          _heightmap.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootMCNKSubchunks::MCLV:
          _vertex_lighting.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootMCNKSubchunks::MCCV:
          _vertex_color.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootMCNKSubchunks::MCNR:
          _normals.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootMCNKSubchunks::MCLQ:
          _tbc_water.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootMCNKSubchunks::MCSE:
          _sound_emitters.Read(buf, chunk_header.size);
          continue;
        case ChunkIdentifiers::ADTRootMCNKSubchunks::MCDD:
          _groundeffect_disable.Read(buf, chunk_header.size);
          continue;
      }

      if constexpr (client_version >= Common::ClientVersion::MOP)
      {
        if (chunk_header.fourcc == ChunkIdentifiers::ADTRootMCNKSubchunks::MCBB)
        {
          this->_blend_batches.Read(buf, chunk_header.size);
          continue;
        }
      }

      buf.Seek<Common::ByteBuffer::SeekDir::Forward, Common::ByteBuffer::SeekType::Relative>(chunk_header.size);
      LogError("Encountered unknown ADT root MCNK sub-chunk %s.", Common::FourCCToStr(chunk_header.fourcc).c_str());
    }
  }

  template<Common::ClientVersion client_version>
  void MCNKRoot<client_version>::Write(Common::ByteBuffer& buf) const
  {
    InvariantF(CCodeZones::FILE_IO, _heightmap.IsInitialized() && _normals.IsInitialized(),
        "MCVT and MCNR must be initialized to write root MCNK.");

    std::size_t pos = buf.Tell();

    // writing chunk header to just initialize buffer bytes
    Common::ChunkHeader chunk_header{};
    buf.Write(chunk_header);

    buf.Write(_header);
    _heightmap.Write(buf);
    _normals.Write(buf);

    // optional sub-chunks
    _vertex_lighting.Write(buf);
    _vertex_color.Write(buf);
    _tbc_water.Write(buf);
    _sound_emitters.Write(buf);

    if constexpr (client_version >= Common::ClientVersion::MOP)
    {
      this->_blend_batches.Write(buf);
    }
    _groundeffect_disable.Write(buf);

    // writing actual chunk header data
    std::size_t end_pos = buf.Tell();
    buf.Seek(pos);
    chunk_header.fourcc = ChunkIdentifiers::ADTRootChunks::MCNK;

    EnsureF(CCodeZones::FILE_IO, (end_pos - pos - sizeof(Common::ChunkHeader)) <= std::numeric_limits<std::uint32_t>::max()
            , "Root MCNK chunk size overflow.");
    chunk_header.size = static_cast<std::uint32_t>(end_pos - pos - sizeof(Common::ChunkHeader));
    buf.Write(chunk_header);
    buf.Seek(end_pos);
  }
}
