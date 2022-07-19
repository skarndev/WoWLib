#include <backward.hpp>
#include <Validation/Log.hpp>
#include <IO/Storage/ClientStorage.hpp>
#include <IO/Storage/FileKey.hpp>

using namespace IO;
using namespace IO::Storage;

int main()
{
  backward::SignalHandling sh;
  Validation::Log::InitLoggers();

  // wotlk
  ClientStorage storage{"C:\\Users\\Skarn\\Documents\\WoWModding\\WoWClients\\3.3.5a"
                , "C:\\Users\\Skarn\\Documents\\WoWModding\\Test"
                , Common::ClientVersion::WOTLK};

  Common::ByteBuffer buf{};
  FileKey key {storage, "world/arttest/boxtest/xyz.m2"};
  FileKey::FileReadStatus status = key.Read(buf);

  Ensure(status == FileKey::FileReadStatus::SUCCESS, "Failed to read file.");

  return 0;
}