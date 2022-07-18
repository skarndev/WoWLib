#pragma once
#include <string>

namespace Utils::PathUtils
{
  /**
     * Normalize filepath to match game client rules (all uppercase, using \\ as separator).
     * @param filepath Filepath.
     * @return Normalized filepath.
     */
  std::string NormalizeFilepathGame(std::string_view filepath);

  /**
   * Normalize filepath to match Unix filesystem requirements (/ as separator).
   * @param filepath Filepath.
   * @return Normalized filepath.
   */
  std::string NormalizeFilepathUnix(std::string_view filepath);

  /**
   * Normalize filepath to match Unix filesystem requirements (/ as separator) and lowercase it.
   * @param filepath Filepath.
   * @return Normalized filepath.
   */
  std::string NormalizeFilepathUnixLower(std::string_view filepath);
}