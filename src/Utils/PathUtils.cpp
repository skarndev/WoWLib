#include <Utils/PathUtils.hpp>

#include <algorithm>
#include <regex>

std::string RemoveInconsistentNaming(std::string const& str)
{
  std::string ret_str;

  if (str.ends_with(".mdx"))
  {
    ret_str = std::regex_replace(str, std::regex(".mdx"), ".m2");
  }
  else if(str.ends_with(".mdl"))
  {
    ret_str = std::regex_replace(str, std::regex(".mdl"), ".m2");
  }
  else
  {
    return str;
  }

  return std::move(ret_str);
}

std::string Utils::PathUtils::NormalizeFilepathGame(std::string_view filepath)
{
  std::string normalized_string{filepath};

  std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), ::toupper);
  std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), [](char c) -> char
  {
    return c == '/' ? '\\' : c;
  });

  return std::move(RemoveInconsistentNaming(normalized_string));
}

std::string Utils::PathUtils::NormalizeFilepathUnix(std::string_view filepath)
{
  std::string normalized_string{filepath};

  std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), [](char c) -> char
  {
    return c == '\\' ? '/' : c;
  });

  return std::move(RemoveInconsistentNaming(normalized_string));
}

std::string Utils::PathUtils::NormalizeFilepathUnixLower(std::string_view filepath)
{
  std::string normalized_string{filepath};

  std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), ::tolower);
  std::transform(normalized_string.begin(), normalized_string.end(), normalized_string.begin(), [](char c) -> char
  {
    return c == '\\' ? '/' : c;
  });

  return std::move(RemoveInconsistentNaming(normalized_string));
}
