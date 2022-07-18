#include <IO/Storage/ClientLoaders/ClassicLoader.hpp>
#include <IO/Storage/Archives/MPQArchive.hpp>
#include <IO/Storage/FileKey.hpp>
#include <IO/Storage/ClientStorage.hpp>
#include <IO/Common.hpp>

using namespace IO::Storage::ClientLoaders;

ClassicLoader::ClassicLoader(ClientStorage& storage)
: MPQLoader(storage)
{
  LoadClassicTBCWotLK(ClassicLoader::ArchiveNameTemplates.begin(), ClassicLoader::ArchiveNameTemplates.end());
}
