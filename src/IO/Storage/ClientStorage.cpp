#include <IO/Storage/ClientStorage.hpp>
#include <IO/Storage/ClientLoaders/ClassicLoader.hpp>
#include <IO/Storage/ClientLoaders/WotLKLoader.hpp>
#include <IO/Storage/ClientLoaders/CASCLoader.hpp>
#include <Utils/PathUtils.hpp>

#include <system_error>
#include <fstream>

using namespace IO::Storage;
namespace fs = std::filesystem;

ClientStorage::ClientStorage(std::string const& path
                             , std::string const& project_path
                             , Common::ClientVersion client_version
                             , Common::ClientLocale locale)
: _project_path(project_path)
, _path(path)
, _client_version(client_version)
, _locale(locale)
{
  RequireF(CCodeZones::STORAGE, client_version < Common::ClientVersion::WOD
           , "This contructor can be used for MPQ-based clients only.");

  // make sure project path exists
  if (!fs::exists(project_path))
  {
    fs::create_directories(project_path);
  }

  switch (client_version)
  {
    case Common::ClientVersion::WOTLK:
    {
      _loader = std::make_unique<ClientLoaders::WotLKLoader>(*this);
      break;
    }
    case Common::ClientVersion::CLASSIC:
    {
      _loader = std::make_unique<ClientLoaders::ClassicLoader>(*this);
      break;
    }
    default:
    {
      assert(false && "Not implemented yet.");
    }
  }

  _listfile = Storage::Listfile(_loader->GetListfile());
}

ClientStorage::ClientStorage(std::string const& path
                             , std::string const& project_path
                             , IO::Common::ClientVersion client_version
                             , std::string const& product
                             , IO::Common::ClientLocale locale)
: _project_path(Utils::PathUtils::NormalizeFilepathUnix(project_path))
, _path(Utils::PathUtils::NormalizeFilepathUnix(path))
, _client_version(client_version)
, _locale(locale)

{
  RequireF(CCodeZones::STORAGE, client_version >= Common::ClientVersion::WOD
           , "This contructor can be used for CASC-based clients only.");

  _loader = std::make_unique<ClientLoaders::CASCLoader>(*this, product);
  _listfile = Storage::Listfile((fs::path(project_path) / "listfile.csv").string());
}

ClientStorage::ClientStorage(std::string const& cdn_url, std::string const& project_path
                             , IO::Common::ClientVersion client_version, std::string const& product
                             , std::string const& region, IO::Common::ClientLocale locale)
: _project_path(Utils::PathUtils::NormalizeFilepathUnix(project_path))
, _path(cdn_url)
, _client_version(client_version)
, _locale(locale)
{
  RequireF(CCodeZones::STORAGE, client_version >= Common::ClientVersion::WOD
           , "This contructor can be used for CASC-based clients only.");

  _loader = std::make_unique<ClientLoaders::CASCLoader>(*this, std::optional{_path.string()}
                                                        , product, region);
  _listfile = Storage::Listfile((fs::path(project_path) / "listfile.csv").string());
}


FileKey::FileReadStatus ClientStorage::ReadFile(FileKey const& file_key, Common::ByteBuffer& buf) const
{
  fs::path filepath = _project_path / Utils::PathUtils::NormalizeFilepathUnixLower(file_key.FilePath());

  // first try to read from project directory
  if (fs::exists(filepath))
  {
    std::uintmax_t size = fs::file_size(filepath);

    EnsureF(CCodeZones::STORAGE, size <= std::numeric_limits<std::uint32_t>::max(), "Invalid filesize.");

    buf.Reserve(size);

    std::ifstream istrm(filepath, std::ios::binary);

    if (istrm.is_open())
    {
      istrm.read(buf.Data(), size);
      return FileKey::FileReadStatus::SUCCESS;
    }
    else
    {
      return FileKey::FileReadStatus::FILE_OPEN_FAILED_OS;
    }
  }

  // attempt to read from client
  return _loader->ReadFile(file_key, buf);
}

FileKey::FileWriteStatus ClientStorage::WriteFile(FileKey const& file_key, Common::ByteBuffer const& buf) const
{
  fs::path filepath = _project_path / Utils::PathUtils::NormalizeFilepathUnixLower(file_key.FilePath());
  fs::path dir_path = filepath.remove_filename();

  std::error_code error;
  fs::create_directories(dir_path, error);

  if (error)
  {
    LogError("Creating directory \"%s\" failed. OS error code: %d. msg: %s.", dir_path.c_str(), error.value(), error
        .message().c_str());
    return FileKey::FileWriteStatus::FILE_WRITE_FAILED;
  }

  std::fstream strm{filepath, std::fstream::binary | std::fstream::out | std::fstream::trunc};

  if (!strm.is_open())
  {
    LogError("Writing file \"%s\" failed. Unknown OS error.", filepath.c_str());
    return FileKey::FileWriteStatus::FILE_WRITE_FAILED;
  }

  buf.Flush(strm);

  return FileKey::FileWriteStatus::SUCCESS;
}

bool ClientStorage::Exists(FileKey const& file_key) const
{
  EnsureF(CCodeZones::STORAGE, file_key.FileDataID(), "Invalid FileDataID.");

  if (!_listfile.Exists(file_key.FileDataID())) [[unlikely]]
  {
    return false;
  }
  else if (fs::exists(file_key.FilePath()) || _loader->Exists(file_key))
  {
    return true;
  }

  return false;
}


