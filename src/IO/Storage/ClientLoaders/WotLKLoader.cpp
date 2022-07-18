#include <IO/Storage/ClientLoaders/WotLKLoader.hpp>
#include <IO/Storage/Archives/MPQArchive.hpp>
#include <IO/Storage/FileKey.hpp>
#include <IO/Storage/ClientStorage.hpp>
#include <IO/Common.hpp>

using namespace IO::Storage::ClientLoaders;

WotLKLoader::WotLKLoader(ClientStorage& storage)
: MPQLoader(storage)
{
  LoadClassicTBCWotLK(WotLKLoader::ArchiveNameTemplates.begin(), WotLKLoader::ArchiveNameTemplates.end());
}

