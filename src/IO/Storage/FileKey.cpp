#include <IO/Storage/FileKey.hpp>
#include <IO/Storage/ClientStorage.hpp>

using namespace IO::Storage;

FileKey::FileKey(ClientStorage& storage, std::uint32_t file_data_id, FileExistPolicy file_exist_policy)
: _storage(&storage)
{
  RequireF(CCodeZones::STORAGE, file_exist_policy != FileExistPolicy::CREATE, "Adding by FDID is not supported.");
  if (file_exist_policy == FileExistPolicy::STRICT)
  {
    if (!_storage->Exists({storage, file_data_id, FileExistPolicy::WEAK}))
    {
      throw Exceptions::FileNotFoundError("File not found (FileDataID): " + std::to_string(file_data_id));
    }
  }
}

FileKey::FileKey(ClientStorage& storage, std::string const& filepath, FileExistPolicy file_exist_policy)
: _storage(&storage)
{
  switch(file_exist_policy)
  {
    case FileExistPolicy::STRICT:
    {
      _file_data_id = storage.Listfile().GetFileDatIDForFilepath(filepath);
      if (!_file_data_id || !_storage->Exists({storage, _file_data_id, FileExistPolicy::WEAK}))
      {
        throw Exceptions::FileNotFoundError("File not found (filepath): " + filepath);
      }
      break;
    }
    case FileExistPolicy::CREATE:
    {
      _file_data_id = storage.Listfile().GetOrAddFileDataID(filepath);
      break;
    }
    case FileExistPolicy::WEAK:
    {
      _file_data_id = storage.Listfile().GetFileDatIDForFilepath(filepath);
      break;
    }
  }
}

std::string const& FileKey::FilePath() const
{
  EnsureF(CCodeZones::STORAGE, _file_data_id, "Invalid FileDataID to load.");

  return _storage->Listfile().GetOrGenerateFilepath(_file_data_id);
}

FileKey::FileReadStatus FileKey::Read(IO::Common::ByteBuffer& buf) const
{
  if (!_file_data_id) [[unlikely]]
  {
    return FileKey::FileReadStatus::INVALID_FILEDATAID;
  }

  return _storage->ReadFile(*this, buf);
}

FileKey::FileWriteStatus FileKey::Write(IO::Common::ByteBuffer const& buf) const
{
  return _storage->WriteFile(*this, buf);
}

bool FileKey::Exists() const
{
  return _storage->Exists(*this);
}



