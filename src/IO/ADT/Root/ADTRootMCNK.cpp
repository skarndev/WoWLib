#include <IO/ADT/Root/ADTRootMCNK.hpp>

using namespace IO::ADT;

bool MCNKRootBlendBatches::Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
{
  if (chunk_header.fourcc == ChunkIdentifiers::ADTRootMCNKSubchunks::MCBB)
  {
    _blend_batches.Read(buf, chunk_header.size);
    return true;
  }
  return false;
}

void MCNKRootBlendBatches::Write(Common::ByteBuffer& buf) const
{
  _blend_batches.Write(buf);
}
