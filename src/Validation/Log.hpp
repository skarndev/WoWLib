#pragma once

#include <Utils/Meta/Concepts.hpp>
#include <Utils/Misc/CurrentFunction.hpp>
#include <Utils/Misc/ForceInline.hpp>

#include <iostream>
#include <filesystem>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <string>

#ifdef VALIDATION_LOG_TO_CONSOLE
  #define LOG_MSG_TOKEN "\u001b[32m[Log]  \u001b[0m"
  #define DEBUG_LOG_MSG_TOKEN "\u001b[33m[Debug]\u001b[0m"
  #define ERROR_LOG_MSG_TOKEN "\u001b[31m[Error]\u001b[0m"
#else
  #define LOG_MSG_TOKEN "[Log]  "
  #define DEBUG_LOG_MSG_TOKEN "[Debug]"
  #define ERROR_LOG_MSG_TOKEN "[Error]"
#endif

namespace Validation::Log
{
  static inline unsigned gLogIndentLevel = 0;

  FORCEINLINE void PrintFormattedLine(const char* name, const char* file, const char* func, int line)
  {
    auto time = clock() * 1000 / CLOCKS_PER_SEC;
    auto time_str = std::to_string(time);

    unsigned new_line_indent = static_cast<unsigned>(time_str.length()) + 11 + gLogIndentLevel * 4;
    if (gLogIndentLevel)
      new_line_indent--;

    std::printf("%s - %s%*cSource: \"%s\", line %d, in %s:\n%*c", time_str.c_str(), name, gLogIndentLevel * 4, ' ',
      std::filesystem::relative(file, SOURCE_DIR).generic_string().c_str(),
      line, func, new_line_indent, ' ');
  }

  FORCEINLINE void PrintFormattedLine(const char* name)
  {
    std::printf("%d - %s%*c", static_cast<int>(clock() * 1000 / CLOCKS_PER_SEC), name, gLogIndentLevel * 4, ' ');
  }

  template<typename ... Args>
  FORCEINLINE void impl_Log(const char* format, const Args&... args)
  {
    PrintFormattedLine(LOG_MSG_TOKEN);
    std::printf(format, args...);
    std::cout << std::endl;
  }

  template<typename ... Args>
  FORCEINLINE void impl_LogDebugV(const char* file, int line, const char* func, const char* format, const Args&... args)
  {
    PrintFormattedLine(DEBUG_LOG_MSG_TOKEN, file, func, line);
    std::printf(format, args...);
    std::cout << std::endl;
  }

  template<typename ... Args>
  FORCEINLINE void impl_LogDebug(const char* format, const Args&... args)
  {
    PrintFormattedLine(DEBUG_LOG_MSG_TOKEN);
    std::printf(format, args...);
    std::cout << std::endl;
  }

  template<typename ... Args>
  FORCEINLINE void impl_LogError(const char* format, const Args&... args)
  {
    PrintFormattedLine(ERROR_LOG_MSG_TOKEN);
    std::printf(format, args...);
    std::cout << std::endl;
  }

  template<typename ... Args>
  FORCEINLINE void impl_LogErrorV(const char* file, int line, const char* func, const char* format, const Args&... args)
  {
    PrintFormattedLine(ERROR_LOG_MSG_TOKEN, file, func, line);
    std::printf(format, args...);
    std::cout << std::endl;
  }

  struct impl_LogLevelScopedSetter
  {
    impl_LogLevelScopedSetter()
    {
      gLogIndentLevel++;
    }

    ~impl_LogLevelScopedSetter()
    {
      gLogIndentLevel--;
    }
  };


  void InitLoggers();
}


#if defined(NDEBUG) && !defined(DEBUG_LOG_IN_RELEASE)
  // Basic debug logger
  #define LogDebug(...) static_cast<void>(0)
  
  // Basic verbose debug logger 
  #define LogDebugV(...) static_cast<void>(0)
  
  // Flagged verbose debug logger
  #define LogDebugVF(...) static_cast<void>(0)

  // Flagged debug logger
  #define LogDebugF(...) static_cast<void>(0)
#else
  // Basic debug logger
  #define LogDebug(...) Validation::Log::impl_LogDebug(__VA_ARGS__)

  // Basic verbose debug logger 
  #define LogDebugF(FLAGS, ...)  \
    if constexpr ((LOGGING_FLAGS & static_cast<unsigned>(FLAGS)) != 0) \
    {\
      Validation::Log::impl_LogDebug(__VA_ARGS__);\
    }\
    static_assert(true)

  // Flagged verbose debug logger
  #define LogDebugV(...) Validation::Log::impl_LogDebugV(__FILE__, __LINE__, CURRENT_FUNCTION, __VA_ARGS__)
  
  // Flagged debug logger
  #define LogDebugVF(FLAGS, ...) \
  if constexpr ((LOGGING_FLAGS & static_cast<unsigned>(FLAGS)) != 0)\
  {\
    Validation::Log::impl_LogDebugV(__FILE__, __LINE__, CURRENT_FUNCTION, __VA_ARGS__);\
  }\
  static_assert(true)

#endif

// Basic logger
#define Log(...) Validation::Log::impl_Log(__VA_ARGS__)

// Basic error logger
#define LogError(...) Validation::Log::impl_LogError(__VA_ARGS__)

// Basic verbose error logger
#define LogErrorV(...) Validation::Log::impl_LogErrorV(__FILE__, __LINE__, CURRENT_FUNCTION, __VA_ARGS__)

// Log indent level scoped setter
#define LogIndentScoped Validation::Log::impl_LogLevelScopedSetter _log_level_indent_setter{}
