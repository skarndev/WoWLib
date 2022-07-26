#include <IO/Storage/FileKey.hpp>
#include <IO/Storage/ClientStorage.hpp>
#include <Utils/PathUtils.hpp>

#include <string>
#include <algorithm>

using namespace IO::Storage;

FileKey::FileKey(ClientStorage& storage, std::uint32_t file_data_id, FileExistPolicy file_exist_policy)
: _storage(&storage)
{
  RequireF(CCodeZones::STORAGE, file_exist_policy != FileExistPolicy::CREATE, "Adding by FDID is not supported.");
  if (file_exist_policy == FileExistPolicy::CHECKEXISTS)
  {
    if (!_storage->Exists({storage, file_data_id, FileExistPolicy::WEAK}))
    {
      throw Exceptions::FileNotFoundError("File not found (FileDataID): " + std::to_string(file_data_id));
    }
  }
}

FileKey::FileKey(ClientStorage& storage
                 , std::string const& filepath
                 , FilePathCorrectionPolicy filepath_correction_policy
                 , FileExistPolicy file_exist_policy)
: _storage(&storage)
{
  auto process = [&, this](std::string const& filepath_)
  {
    switch(file_exist_policy)
    {
      case FileExistPolicy::CHECKEXISTS:
      {
        _file_data_id = storage.Listfile().GetFileDatIDForFilepath(filepath_);
        if (!_file_data_id || !_storage->Exists({storage, _file_data_id, FileExistPolicy::WEAK}))
        {
          throw Exceptions::FileNotFoundError("File not found (filepath): " + filepath_);
        }
        break;
      }
      case FileExistPolicy::CREATE:
      {
        _file_data_id = storage.Listfile().GetOrAddFileDataID(filepath_);
        break;
      }
      case FileExistPolicy::WEAK:
      {
        if (storage.ClientVersion() <= Common::ClientVersion::WOD)
        {
          _file_data_id = storage.Listfile().GetOrAddFileDataID(filepath_);
        }
        else
        {
          _file_data_id = storage.Listfile().GetFileDatIDForFilepath(filepath_);
        }
        break;
      }
    }
  };

  switch (filepath_correction_policy)
  {
    case FilePathCorrectionPolicy::CORRECT:
      process(Utils::PathUtils::NormalizeFilepathGame(filepath));
      break;

    case FilePathCorrectionPolicy::TRUST:
    {
      EnsureF(CCodeZones::STORAGE
              , [filepath]() -> bool
                { return std::all_of(filepath.begin(), filepath.end()
                                     , [](char c) -> bool
                                       {
                                         return ((std::isdigit(c) || std::iscntrl(c)
                                                  || std::ispunct(c) || std::isspace(c))
                                           || std::isupper(c)) && c != '/';
                                       });
                }, "Trusted path is not in game format.");
      process(filepath);
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



