#include <backward.hpp>
#include <Validation/Log.hpp>
#include <IO/Storage/ClientStorage.hpp>
#include <IO/Storage/FileKey.hpp>
#include <IO/ADT/Root/ADTRoot.hpp>

#include <IO/ADT/ChunkIdentifiers.hpp>
#include <boost/hana/map.hpp>
#include <array>
#include <functional>
#include <IO/CommonTraits.hpp>

using namespace IO;
using namespace IO::Storage;


int main()
{
  backward::SignalHandling sh;
  Validation::Log::InitLoggers();

  // wotlk
  ClientStorage storage{"/home/skarn/Documents/WoWModding/Clients/3.3.5a/"
                , "/home/skarn/Documents/WoWModding/Test"
                , Common::ClientVersion::WOTLK};

  Common::ByteBuffer buf{};
  FileKey key {storage, "world/arttest/boxtest/xyz.m2", FileKey::FilePathCorrectionPolicy::CORRECT};
  FileKey::FileReadStatus status = key.Read(buf);

  Ensure(status == FileKey::FileReadStatus::SUCCESS, "Failed to read file.");
  Ensure((buf.Read<std::uint32_t>() == Common::FourCC<"MD20", Common::FourCCEndian::Big>), "Incorrect file contents.");

  IO::ADT::ADTRoot<Common::ClientVersion::MOP> root{0};

  return 0;
}