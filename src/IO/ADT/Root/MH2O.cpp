#include <IO/ADT/Root/MH2O.hpp>

#include <cstdint>

using namespace IO::ADT;

void MH2O::Read(Common::ByteBuffer const& buf, std::size_t size)
{
  LogDebugF(LCodeZones::FILE_IO, "Loading ADT root chunk MH2O.");

  std::size_t data_pos = buf.Tell();

  std::array<DataStructures::SMLiquidChunk, 16 * 16> chunks{};
  buf.Read(&(*chunks.begin()), &(*chunks.end()));


  



}

void MH2O::Write(Common::ByteBuffer& buf)
{
   
}
