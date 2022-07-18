#include <IO/Storage/Listfile.hpp>
#include <IO/ByteBuffer.hpp>
#include <IO/Storage/FileKey.hpp>
#include <Utils/PathUtils.hpp>

#include <charconv>
#include <fstream>

using namespace IO::Storage;

Listfile::Listfile(std::string const& path, std::uint32_t max_file_data_id)
: _max_file_data_id(max_file_data_id)
, _file_data_id_policy(FileDataIDPolicy::REAL)
{
  std::ifstream fstream;
  fstream.open(path);

  if (!fstream.is_open())
  {
    throw Exceptions::ListFileNotFoundError();
  }
  else
  {
    std::string line;
    while (std::getline(fstream, line))
    {
      std::string_view line_ {line};

      auto sep_pos = line_.find_first_of(';');
      EnsureF(CCodeZones::STORAGE, sep_pos != std::string_view::npos, "Malformed listfile.");

      std::string_view uid_str {line_.substr(0, sep_pos)};
      std::string_view filename {line_.substr(sep_pos + 1)};

      std::uint32_t uid;
      std::from_chars(uid_str.data(), uid_str.data() + uid_str.size(), uid);

      _max_file_data_id = std::max(_max_file_data_id, uid);
      _fdid_path_map.insert(uid, Utils::PathUtils::NormalizeFilepathGame(filename));
    }
  }
}

Listfile::Listfile(IO::Common::ByteBuffer const& listfile_buf)
: _max_file_data_id(0)
, _file_data_id_policy(FileDataIDPolicy::INTERNAL)
{
  std::string current;

  for (std::size_t i = 0; i < listfile_buf.Size(); ++i)
  {
    char c = listfile_buf.Data()[i];

    if (c == '\r')
    {
      continue;
    }
    if (c == '\n')
    {
      _fdid_path_map.insert(++_max_file_data_id, Utils::PathUtils::NormalizeFilepathGame(current));
      current.resize(0);
    }
    else
    {
      current += c;
    }
  }

  if (!current.empty())
  {
    _fdid_path_map.insert(++_max_file_data_id, Utils::PathUtils::NormalizeFilepathGame(current));
  }

}

std::uint32_t Listfile::GetOrAddFileDataID(std::string const& filepath)
{
  auto it = _fdid_path_map.find_value(filepath);

  if (it != _fdid_path_map.end())
  {
    return it->first.first;
  }

  _fdid_path_map.insert(++_max_file_data_id, filepath);
  return _max_file_data_id;
}

std::string const& Listfile::GetOrGenerateFilepath(std::uint32_t file_data_id)
{
  auto it = _fdid_path_map.find_key(file_data_id);

  if (it != _fdid_path_map.end())
  {
    return it->first.second;
  }

  auto new_it = _fdid_path_map.insert(file_data_id, "UNKNOWN\\" + std::to_string(file_data_id));
  return new_it.first->first.second;
}

void Listfile::Save()
{
  EnsureF(CCodeZones::STORAGE, _file_data_id_policy == FileDataIDPolicy::REAL, "Can't be used with fake listfiles.");
  std::fstream stream{_path, std::fstream::binary | std::fstream::trunc | std::fstream::out};

  if (!stream.is_open())
  {
    throw Exceptions::ListFileNotFoundError();
  }

  for (auto& [pair, _] : _fdid_path_map)
  {
    stream.write(reinterpret_cast<const char*>(&pair.first), sizeof(std::uint32_t));
    stream << ';';
    stream.write(reinterpret_cast<const char*>(&pair.second), pair.second.size());
    stream << '\n';
  }
}

std::uint32_t Listfile::GetFileDatIDForFilepath(std::string const& filepath) const
{
  auto it = _fdid_path_map.find_value(filepath);

  return it != _fdid_path_map.end() ? it->first.first : 0;
}

bool Listfile::Exists(std::uint32_t file_data_id) const
{
  return _fdid_path_map.contains_key(file_data_id);
}
