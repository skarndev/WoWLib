#include <Validation/Log.hpp>

void Validation::Log::InitLoggers()
{
#ifdef VALIDATION_LOG_TO_CONSOLE
  std::cout << clock() * 1000 / CLOCKS_PER_SEC << " - " << LOG_MSG_TOKEN << " Logging to standard output." << std::endl;
#else
  static std::ofstream stream;

  stream.open("log.txt", std::ios_base::out | std::ios_base::trunc);
  if (stream)
  {
    std::cout.rdbuf(stream.rdbuf());
    std::cerr.rdbuf(stream.rdbuf());
  }
#endif
};