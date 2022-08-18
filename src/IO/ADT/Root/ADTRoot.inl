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
    this->Read(buf);
  }

  template<Common::ClientVersion client_version>
  void ADTRoot<client_version>::WriteExtraPost(details::ADTRootWriteContext& ctx, Common::ByteBuffer& buf) const
  {
    Common::DataChunk<DataStructures::MHDR, ChunkIdentifiers::ADTRootChunks::MHDR> header{};
    header.Initialize();
    header.data.flags |= ctx.header_flags;
    header.data.mh2o = ctx.liquid_pos;
    header.data.mfbo = ctx.mfbo_pos;

    std::size_t end_pos = buf.Tell();

    // go back and write relevant header data
    buf.Seek(ctx.header_pos);
    header.Write(buf);
    buf.Seek(end_pos);
  }
}
