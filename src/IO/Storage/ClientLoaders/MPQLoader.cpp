#include <IO/Storage/ClientLoaders/MPQLoader.hpp>
#include <IO/Storage/Archives/MPQArchive.hpp>
#include <IO/Storage/FileKey.hpp>
#include <IO/Common.hpp>

#include <filesystem>

using namespace IO::Storage::ClientLoaders;
namespace fs = std::filesystem;

MPQLoader::MPQLoader(ClientStorage& storage)
    : BaseLoader(storage)
{
}

void MPQLoader::LoadArchive(std::string const& path)
{
  if (fs::exists(path) && !fs::equivalent(path, _storage->ProjectPath()))
  {
    try
    {
      _archives.emplace_back(std::make_unique<Archives::MPQArchive>(path));
    }
    catch(Archives::Exceptions::MPQOpenFailedError&)
    {
      throw Exceptions::ArchiveLoadingFailureError(path);
    }
  }

}

IO::Common::ByteBuffer MPQLoader::GetListfile()
{
  Common::ByteBuffer buf{};

  for(auto it = _archives.rbegin();  it != _archives.rend(); ++it)
  {
    if (static_cast<Archives::MPQArchive*>(it->get())->ReadFile({*_storage, "(LISTFILE)"}, buf)
        != FileKey::FileReadStatus::SUCCESS)
    {
      LogDebugF(CCodeZones::STORAGE, "(listfile) not found in archive: %s"
                , static_cast<Archives::MPQArchive*>(it->get())->Path().c_str());
    }
  }

  return buf;
}

void MPQLoader::ReplaceLocale(std::string& mpq_path, std::string_view const& locale) const
{
  std::string::size_type location(std::string::npos);

  do
  {
    location = mpq_path.find("{locale}");
    if (location != std::string::npos)
    {
      mpq_path.replace(location, 8, locale);
    }
  } while (location != std::string::npos);
}

bool MPQLoader::LoadNumberedPatches(std::string& mpq_path)
{
  if (std::string::size_type location = mpq_path.find("{number}"); location != std::string::npos)
  {
    location = mpq_path.find("{number}");
    mpq_path.replace(location, 8, " ");
    for (char j = '2'; j <= '9'; j++)
    {
      mpq_path.replace(location, 1, std::string(&j, 1));
      LoadArchive(mpq_path);
    }

    return true;
  }

  return false;
}

bool MPQLoader::LoadCharacterNumberedPatches(std::string& mpq_path)
{
  if (std::string::size_type location = mpq_path.find("{character}"); location != std::string::npos)
  {
    location = mpq_path.find("{character}");
    mpq_path.replace(location, 11, " ");
    for (char c = 'a'; c <= 'z'; c++)
    {
      mpq_path.replace(location, 1, std::string(&c, 1));
      LoadArchive(mpq_path);
    }

    return true;
  }

  return false;
}

std::string_view MPQLoader::DetermineLocale(std::filesystem::path const& data_path) const
{
  if (_storage->Locale() == Common::ClientLocale::AUTO)
  {
    for (auto const& loc : Common::ClientLocaleStr)
    {
      if (fs::exists(data_path / loc))
      {
        return loc;
      }
    }
  }
  else
  {
    std::string_view locale = Common::ClientLocaleStr[static_cast<std::uint32_t>(_storage->Locale())];

    if (fs::exists(data_path / locale))
    {
      return locale;
    }
    else
    {
      throw Exceptions::LocaleDirNotFoundError("Locale \"" + std::string(locale)
                                               + "\" was not found in client directory.");
    }
  }

  throw Exceptions::LocaleDirNotFoundError("Automatic locale detection failed. "
                                           "No locale found in client directory.");

}


