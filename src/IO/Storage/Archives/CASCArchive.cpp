#include <IO/Storage/Archives/CASCArchive.hpp>
#include <CascLib.h>

#include <filesystem>

namespace fs = std::filesystem;
using namespace IO::Storage::Archives;

bool WINAPI LogInitProgress(void* user_param, LPCSTR work, LPCSTR object, DWORD cur_value, DWORD tot_value)
{
  Log("(%d/%d) %s", cur_value, tot_value, work);
  return false;
}

CASCArchive::CASCArchive(std::string const& path, std::string const& product)
: _handle(nullptr)
{
  CASC_OPEN_STORAGE_ARGS args;
  args.Size = sizeof(CASC_OPEN_STORAGE_ARGS);
  args.szLocalPath = path.c_str();
  args.szCodeName = product.c_str();
  args.szRegion = nullptr;
  args.PfnProgressCallback = &LogInitProgress;
  args.PtrProgressParam = nullptr;
  args.PfnProductCallback = nullptr;
  args.PtrProductParam = nullptr;
  args.dwLocaleMask = 0;
  args.dwFlags = 0;
  args.szBuildKey = nullptr;
  args.szCdnHostUrl = nullptr;

  Log("Opening local CASC storage at: \"%s\"", path.c_str());
  if (!CascOpenStorageEx(nullptr, &args, false, &_handle))
  {
    throw Exceptions::CASCStorageOpenFailedError(path);
  }
}

CASCArchive::CASCArchive(std::string const& project_path
                         , std::optional<std::string> const& cdn_url
                         , std::string const& product
                         , std::string const& region)
: _handle(nullptr)
{
  auto cache_path = fs::path(project_path) / ".TACTCache";

  // ensure cache dir existence
  if (!fs::exists(cache_path))
  {
    std::error_code error;
    fs::create_directories(cache_path, error);

    if (error)
    {
      throw Exceptions::CASCStorageOpenFailedError("Failed to create cache dir.");
    }
  }

  CASC_OPEN_STORAGE_ARGS args;
  args.Size = sizeof(CASC_OPEN_STORAGE_ARGS);
  args.szLocalPath = cache_path.string().c_str();
  args.szCodeName = product.c_str();
  args.szRegion = region.c_str();
  args.PfnProgressCallback = &LogInitProgress;
  args.PtrProgressParam = nullptr;
  args.PfnProductCallback = nullptr;
  args.PtrProductParam = nullptr;
  args.dwLocaleMask = 0;
  args.dwFlags = 0;
  args.szBuildKey = nullptr;
  args.szCdnHostUrl = cdn_url.has_value() ? cdn_url.value().c_str() : nullptr;

  Log("Opening online CASC storage at: \"%s\"", cdn_url.has_value() ? cdn_url.value().c_str() : "Blizzard CDN");
  if (!CascOpenStorageEx(nullptr, &args, true, &_handle))
  {
    throw Exceptions::CASCStorageOpenFailedError("(online)");
  }
}

CASCArchive::~CASCArchive()
{
  CascCloseStorage(_handle);
}

IO::Storage::FileKey::FileReadStatus CASCArchive::ReadFile(IO::Storage::FileKey const& file_key
                                                           , IO::Common::ByteBuffer& buf) const
{
  RequireF(CCodeZones::STORAGE, file_key.FileDataID(), "Invalid FileDataID.");
  RequireF(CCodeZones::STORAGE, buf.IsDataOnwed(), "Buffer is a borrowed buffer.");

  HANDLE file;
  if (CascOpenFile(_handle, CASC_FILE_DATA_ID(file_key.FileDataID()), 0, CASC_OPEN_BY_FILEID, &file))
  {
    std::uint64_t file_size;
    if (CascGetFileSize64(file, &file_size))
    {
      buf.Reserve(file_size);
      CascReadFile(file, buf.Data(), static_cast<DWORD>(file_size), nullptr);
      CascCloseFile(file);
      return IO::Storage::FileKey::FileReadStatus::SUCCESS;
    }
    else
    {
      CascCloseFile(file);
      return IO::Storage::FileKey::FileReadStatus::FILE_OPEN_FAILED_CLIENT;
    }
  }
  else
  {
    DWORD error = GetCascError();

    switch (error)
    {
      case ERROR_FILE_NOT_FOUND:
        return FileKey::FileReadStatus::FILE_NOT_FOUND;
      case ERROR_FILE_CORRUPT:
        return FileKey::FileReadStatus::FILE_OPEN_FAILED_CLIENT;
      case ERROR_NOT_ENOUGH_MEMORY:
        return FileKey::FileReadStatus::NOT_ENOUGH_MEMORY;
      default:
        EnsureF(CCodeZones::STORAGE, false, "Unexpected CASC file read error. Error code: %d", error);
        return FileKey::FileReadStatus::FILE_OPEN_FAILED_CLIENT;
    }
  }
}

bool CASCArchive::Exists(IO::Storage::FileKey const& file_key) const
{
  RequireF(CCodeZones::STORAGE, file_key.FileDataID(), "Invalid FileDataID.");

  // If a file made it through the listfile check, we trust for it to exist within storage.
  return true;
}
