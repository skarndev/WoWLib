#include <IO/Storage/Archives/MPQArchive.hpp>
#include <IO/Storage/FileKey.hpp>
#include <IO/ByteBuffer.hpp>
#include <Utils/PathUtils.hpp>
#include <StormLib.h>

#include <filesystem>
#include <fstream>

using namespace IO::Storage;
using namespace IO::Storage::Archives;
namespace fs = std::filesystem;

MPQArchive::MPQArchive(std::string const& path)
: _path(path)
{
  // MPQ, else MPQ-like named dir.
  if (!fs::is_directory(path))
  {
    Log("Loading MPQ archive: %s", path.c_str());
    if (!SFileOpenArchive(path.c_str(), 0, MPQ_OPEN_NO_LISTFILE | STREAM_FLAG_READ_ONLY, &_handle))
    {
      throw Exceptions::MPQOpenFailedError(path);
    }
  }
  else
  {
    Log("Loading directory archive: %s", path.c_str());
  }
}

MPQArchive::~MPQArchive()
{
  if (_handle)
  {
    SFileCloseArchive(_handle);
  }
}

FileKey::FileReadStatus MPQArchive::ReadFile(FileKey const& file_key, IO::Common::ByteBuffer& buf) const
{
  RequireF(CCodeZones::STORAGE, buf.IsDataOnwed(), "Buffer is a borrowed buffer.");

  // MPQ archive
  if (_handle) [[likely]]
  {
    HANDLE handle;
    if (SFileOpenFileEx(_handle, file_key.FilePath().c_str(), 0, &handle))
    {
      std::size_t size = SFileGetFileSize(handle, nullptr);
      if (size == SFILE_INVALID_SIZE)
      {
        return FileKey::FileReadStatus::FILE_OPEN_FAILED_CLIENT;
      }

      buf.Reserve(size);

      std::size_t bytes_read = 0;
      if (!SFileReadFile(handle, buf.Data(), static_cast<DWORD>(size), reinterpret_cast<LPDWORD>(&bytes_read), nullptr))
      {
        DWORD error = GetLastError();

        if (error == ERROR_HANDLE_EOF)
        {
          SFileCloseFile(handle);
          return FileKey::FileReadStatus::FILE_READ_FAILED;
        }
      }

      EnsureF(CCodeZones::STORAGE, bytes_read == size, "Size mismatch on read.");
      SFileCloseFile(handle);
      return FileKey::FileReadStatus::SUCCESS;
    }
    else
    {
      DWORD error = GetLastError();

      switch (error)
      {
        case ERROR_FILE_NOT_FOUND:
          return FileKey::FileReadStatus::FILE_NOT_FOUND;
        case ERROR_FILE_CORRUPT:
          return FileKey::FileReadStatus::FILE_OPEN_FAILED_CLIENT;
        case ERROR_NOT_ENOUGH_MEMORY:
          return FileKey::FileReadStatus::NOT_ENOUGH_MEMORY;
        default:
          EnsureF(CCodeZones::STORAGE, false, "Unexpected MPQ file read error. Error code: %d", error);
          return FileKey::FileReadStatus::FILE_OPEN_FAILED_CLIENT;
      }
    }
  }
  // MPQ-like directory
  else
  {
    std::string normalized_path = Utils::PathUtils::NormalizeFilepathUnixLower(file_key.FilePath());

    fs::path local_filepath = fs::path(_path) / fs::path(normalized_path);

    if (fs::exists(local_filepath))
    {
      std::uintmax_t size = fs::file_size(local_filepath);

      EnsureF(CCodeZones::STORAGE, size <= std::numeric_limits<std::uint32_t>::max(), "Invalid filesize.");

      buf.Reserve(size);

      std::ifstream istrm(local_filepath, std::ios::binary);

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
    else
    {
      return FileKey::FileReadStatus::FILE_NOT_FOUND;
    }
  }
}

bool MPQArchive::Exists(FileKey const& file_key) const
{
  // MPQ archive
  if (_handle) [[likely]]
  {
    return SFileHasFile(_handle, file_key.FilePath().c_str());
  }
  // MPQ-like dir
  else
  {
    std::string normalized_path = Utils::PathUtils::NormalizeFilepathUnixLower(file_key.FilePath());

    fs::path local_filepath = fs::path(_path) / fs::path(normalized_path);

    return fs::exists(local_filepath);
  }
}
