#include <IO/Storage/ClientLoaders/CASCLoader.hpp>
#include <IO/Storage/Archives/CASCArchive.hpp>
#include <IO/Storage/ClientStorage.hpp>

using namespace IO::Storage::ClientLoaders;

CASCLoader::CASCLoader(IO::Storage::ClientStorage& storage, std::string const& product)
: BaseLoader(storage)
{
  _archives.emplace_back(new Archives::CASCArchive(storage.Path().string(), product));
}

CASCLoader::CASCLoader(IO::Storage::ClientStorage& storage, std::optional<std::string> const& cdn_url
                       , std::string const& product, std::string const& region)
: BaseLoader(storage)
{
  _archives.emplace_back(new Archives::CASCArchive(storage.ProjectPath().string(), cdn_url, product, region));
}
