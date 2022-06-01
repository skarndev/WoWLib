#pragma once
#include <Utils/Misc/ForceInline.hpp>
#include <Utils/Misc/CurrentFunction.hpp>
#include <Validation/Log.hpp>
#include <backward.hpp>

#include <iostream>
#include <filesystem>
#include <cassert>
#include <cstdlib>

namespace Validation::Contracts
{

  template<typename... Args>
  FORCEINLINE bool ResolveContract(bool expr_result
    , const char* expr
    , const char* file
    , int line
    , const char* func
    , const char* type
    , const char* msg
    , const Args&... args)
  {
    if (expr_result)
      return true;
    
    std::string error_text{ "Contract check failed: %s(%s): " };
    error_text.append(msg);
    Validation::Log::_LogErrorV(file, line, func, error_text.c_str(), type, expr, args...);

    return false;

  }

  template<unsigned flags, typename... Args>
  FORCEINLINE bool ResolveContract(bool expr_result
    , const char* expr
    , const char* file
    , int line
    , const char* func
    , const char* type
    , const char* msg
    , const Args&... args)
  {
    if constexpr (CONTRACT_FLAGS & flags)
    {
      if (expr_result)
        return true;

      std::string error_text{ "Contract check failed: %s(%s): " };
      error_text.append(msg);
      Validation::Log::_LogErrorV(file, line, func, error_text.c_str(), type, expr, args...);

      return false;
    }
    else
    {
      return true;
    }
  }
}


#if defined(NDEBUG) && !defined(ENABLE_CONTRACTS_IN_RELEASE)
  #define Require(EXPR, ...) static_cast<void>(0)
  #define Ensure(EXPR, ...) static_cast<void>(0)
  #define Invariant(...) static_cast<void>(0)
  #define RequireF(FLAGS, EXPR, ...) static_cast<void>(0)
  #define EnsureF(FLAGS, EXPR, ...) static_cast<void>(0)
  #define InvariantF(FLAGS, ...) static_cast<void>(0)

#else
  #define Require(EXPR, ...) if(!Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Require", __VA_ARGS__) ) std::abort()
  #define Ensure(EXPR, ...) if(!Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Ensure", __VA_ARGS__) ) std::abort()
  #define Invariant(EXPR, ...) if(!Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__) ) std::abort()
  #define RequireF(FLAGS, EXPR, ...) if(!Validation::Contracts::ResolveContract<FLAGS>((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Require", __VA_ARGS__) ) std::abort()
  #define EnsureF(FLAGS, EXPR, ...) if(!Validation::Contracts::ResolveContract<FLAGS>((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Ensure", __VA_ARGS__) ) std::abort()
  #define InvariantF(FLAGS, EXPR, ...) if(!Validation::Contracts::ResolveContract<FLAGS>((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__) ) std::abort()

#endif

