#pragma once
#include <IO/Storage/ClientLoaders/MPQLoader.hpp>

template<typename T>
void IO::Storage::ClientLoaders::MPQLoader::LoadClassicTBCWotLK(T begin, T end)
requires (std::contiguous_iterator<T> && std::same_as<std::iter_value_t<T>, std::string_view>)
{
  RequireF(CCodeZones::STORAGE, _storage->ClientVersion() <= Common::ClientVersion::WOTLK
          , "This method only supports clients <= wotlk");
  std::filesystem::path data_path = _storage->Path() / "Data";

  std::string_view locale;
  if (_storage->ClientVersion() != Common::ClientVersion::CLASSIC)
  {
    locale = DetermineLocale(data_path);
  }

  for (auto it = begin; it != end; ++it)
  {
    std::string_view const& filename = *it;
    std::string mpq_path = (data_path / filename).string();

    if (_storage->ClientVersion() != Common::ClientVersion::CLASSIC)
    {
      ReplaceLocale(mpq_path, locale);
    }

    if (!LoadNumberedPatches(mpq_path) && !LoadCharacterNumberedPatches(mpq_path))
    {
      // handle non-templated case
      LoadArchive(mpq_path);
    }
  }
}
