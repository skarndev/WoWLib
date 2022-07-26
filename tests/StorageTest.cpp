#include <backward.hpp>
#include <Validation/Log.hpp>
#include <IO/Storage/ClientStorage.hpp>
#include <IO/Storage/FileKey.hpp>
#include <IO/ADT/Root/ADTRoot.hpp>

#include <boost/pfr.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>
#include <boost/hana/map.hpp>
#include <array>
#include <functional>
#include <IO/CommonTraits.hpp>

using namespace IO;
using namespace IO::Storage;


struct Trait : public Common::Traits::AutoIOTrait<Trait>
{
protected:
  Common::DataArrayChunk<char, IO::ADT::ChunkIdentifiers::ADTObj1Chunks::MLDB> _foo;
  Common::DataArrayChunk<IO::ADT::DataStructures::MDDF, IO::ADT::ChunkIdentifiers::ADTObj0Chunks::MDDF> _bar;

public:
  bool Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
  {
    return ReadChunk<&Trait::_foo, &Trait::_bar>(chunk_header.fourcc, buf, chunk_header.size);
  }
};

struct TraitAlt
{
public:
  Common::DataArrayChunk<char, IO::ADT::ChunkIdentifiers::ADTObj1Chunks::MLDB> _foo;
  Common::DataArrayChunk<IO::ADT::DataStructures::MDDF, IO::ADT::ChunkIdentifiers::ADTObj0Chunks::MDDF> _bar;
};

int main()
{
  backward::SignalHandling sh;
  Validation::Log::InitLoggers();

  // wotlk
  ClientStorage storage{"C:\\Users\\Skarn\\Documents\\WoWModding\\WoWClients\\3.3.5a"
                , "C:\\Users\\Skarn\\Documents\\WoWModding\\Test"
                , Common::ClientVersion::WOTLK};

  Common::ByteBuffer buf{};
  FileKey key {storage, "world/arttest/boxtest/xyz.m2", FileKey::FilePathCorrectionPolicy::CORRECT};
  FileKey::FileReadStatus status = key.Read(buf);

  Ensure(status == FileKey::FileReadStatus::SUCCESS, "Failed to read file.");
  Ensure((buf.Read<std::uint32_t>() == Common::FourCC<"MD20", Common::FourCCEndian::BIG>), "Incorrect file contents.");

  IO::ADT::ADTRoot<Common::ClientVersion::MOP> root{0};

  Trait t {};
  Common::ChunkHeader h{static_cast<std::uint32_t>(IO::ADT::ChunkIdentifiers::ADTObj1Chunks::MLDB), 10};
  t.Read(buf, h);
  Common::ChunkHeader h1{static_cast<std::uint32_t>(IO::ADT::ChunkIdentifiers::ADTObj1Chunks::MLDL), 10};
  assert(!t.Read(buf, h1));
  //handle_cases<&Trait::_foo, &Trait::_bar>(&t, 1);

  TraitAlt t1 {};
  boost::pfr::for_each_field(t1, [=](auto& val){});
  return 0;
}