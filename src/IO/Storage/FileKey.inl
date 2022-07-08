#pragma once
#include <IO/Storage/FileKey.hpp>
#include <IO/Storage/ClientStorage.hpp>

#include <type_traits>
#include <regex>

namespace IO::Storage
{
  template<Common::ClientVersion client_version
          , FileExistPolicy exist_policy
          , FilePathCorrectionPolicy filepath_correction
          >
  inline FileKey<client_version, exist_policy, filepath_correction>::FileKey(ClientStorage<client_version>& storage
                                                                             , std::uint32_t file_data_id)
  requires (exist_policy != FileExistPolicy::CREATE)
  : _file_data_id(file_data_id)
  , _storage(&storage)
  {
    if constexpr (exist_policy == FileExistPolicy::STRICT)
    {
      if(_storage->FileExists(_file_data_id))
      {
        throw Exceptions::FileNotFoundError();
      }
    }
  }

  template<Common::ClientVersion client_version
          , FileExistPolicy exist_policy
          , FilePathCorrectionPolicy filepath_correction
          >
  inline FileKey<client_version, exist_policy, filepath_correction>::FileKey(ClientStorage<client_version>& storage
                                                                             , std::string const& filepath)
  : _storage(&storage)
  {
    std::conditional_t<filepath_correction == FilePathCorrectionPolicy::CORRECT
                      , std::string
                      , std::string const*
                      > filepath_;

    if constexpr (filepath_correction == FilePathCorrectionPolicy::CORRECT)
    {
      filepath_ = NormalizeFilepathGame(filepath);
    }
    else
    {
      // debug check to see if filename actually matches the required format
      RequireF(CCodeZones::STORAGE, [filepath]() -> bool
        {
          return std::none_of(filepath.begin(), filepath.end(), [](char c)
          {
            return c == '/';
          });
        }
        , "Filepath was not normalized.");

      filepath_ = &filepath;
    }

    std::uint32_t file_data_id = _storage->FileDataIDForFilepath(*filepath_);

    if constexpr (exist_policy == FileExistPolicy::STRICT)
    {
      if (!file_data_id)
      {
        throw Exceptions::FileNotFoundError();
      }
    }
    else if constexpr (exist_policy == FileExistPolicy::CREATE)
    {
      if (!file_data_id)
      {
        file_data_id = _storage->CreateFileDataIDForPath(*filepath_);
      }
    }

    _file_data_id = file_data_id;
  }

  template<Common::ClientVersion client_version
          , FileExistPolicy exist_policy
          , FilePathCorrectionPolicy filepath_correction
          >
  std::string const& FileKey<client_version, exist_policy, filepath_correction>::FilePath() const
  {
    RequireF(CCodeZones::STORAGE, _file_data_id, "Invalid FileDataID.");
    return _storage->FilepathForFileDataID(_file_data_id);
  }

  template<Common::ClientVersion client_version
          , FileExistPolicy exist_policy
          , FilePathCorrectionPolicy filepath_correction
          >
  std::string FileKey<client_version, exist_policy, filepath_correction>
    ::NormalizeFilepathGame(std::string const& filepath)
  {
    std::string normalized_string{filepath};

    std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), ::toupper);
    std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), [](char c) -> char
      {
        return c == '/' ? '\\' : c;
      });

    if (normalized_string.ends_with(".mdx"))
    {
      normalized_string = std::regex_replace(normalized_string, std::regex(".mdx"), ".m2");
    }
    else if(normalized_string.ends_with(".mdl"))
    {
      normalized_string = std::regex_replace(normalized_string, std::regex(".mdl"), ".m2");
    }

    return std::move(normalized_string);
  }

  template<Common::ClientVersion client_version
          , FileExistPolicy exist_policy
          , FilePathCorrectionPolicy filepath_correction
          >
  std::string FileKey<client_version, exist_policy, filepath_correction>
      ::NormalizeFilepathUnix(std::string const& filepath)
  {
    std::string normalized_string{filepath};

    std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), ::tolower);
    std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), [](char c) -> char
    {
      return c == '\\' ? '/' : c;
    });

    if (normalized_string.ends_with(".mdx"))
    {
      normalized_string = std::regex_replace(normalized_string, std::regex(".mdx"), ".m2");
    }
    else if(normalized_string.ends_with(".mdl"))
    {
      normalized_string = std::regex_replace(normalized_string, std::regex(".mdl"), ".m2");
    }

    return std::move(normalized_string);
  }
}

