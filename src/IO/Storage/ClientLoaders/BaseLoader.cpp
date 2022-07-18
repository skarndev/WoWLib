#include <IO/Storage/ClientLoaders/BaseLoader.hpp>

using namespace IO::Storage::ClientLoaders;



IO::Storage::FileKey::FileReadStatus BaseLoader::ReadFile(IO::Storage::FileKey const& file_key, IO::Common::ByteBuffer& buf)
{
  for (auto it = _archives.rbegin(); it != _archives.rend(); ++it)
  {
    FileKey::FileReadStatus status = (*it)->ReadFile(file_key, buf);

    if (status == FileKey::FileReadStatus::FILE_NOT_FOUND)
    {
      continue;
    }
    else
    {
      return status;
    }
  }

  return FileKey::FileReadStatus::FILE_NOT_FOUND;
}

bool BaseLoader::Exists(IO::Storage::FileKey const& file_key) const
{
  for (auto it = _archives.rbegin(); it != _archives.rend(); ++it)
  {
    if ((*it)->Exists(file_key))
    {
      return true;
    }
  }

  return false;
}
