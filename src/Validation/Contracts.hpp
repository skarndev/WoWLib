#pragma once
#include <Utils/Misc/ForceInline.hpp>
#include <Utils/Misc/CurrentFunction.hpp>
#include <Utils/Meta/Templates.hpp>
#include <Validation/Log.hpp>

#include <iostream>
#include <filesystem>
#include <cassert>
#include <functional>
#include <cstdlib>
#include <algorithm>

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

  template<typename... Args, std::size_t Sz>
  FORCEINLINE bool ResolveContract(std::array<bool, Sz> const&& exprs
    , const char* expr
    , const char* file
    , int line
    , const char* func
    , const char* type
    , const char* msg
    , const Args&... args)
  {
    if (std::all_of(exprs.cbegin(), exprs.cend(), [](bool x) { return x; }))
      return true;

    std::string error_text{ "Contract check failed: %s(%s): " };
    error_text.append(msg);
    Validation::Log::_LogErrorV(file, line, func, error_text.c_str(), type, expr, args...);

    return false;
  }

  template<typename... Args>
  FORCEINLINE bool ResolveContract(const std::function<bool()>& callback
    , const char* expr
    , const char* file
    , int line
    , const char* func
    , const char* type
    , const char* msg
    , const Args&... args)
  {
    bool expr_result = callback();
    if (expr_result)
      return true;

    std::string error_text{ "Contract check failed: %s(%s): " };
    error_text.append(msg);
    Validation::Log::_LogErrorV(file, line, func, error_text.c_str(), type, expr, args...);

    return false;

  }

  FORCEINLINE void RaiseAbort(bool is_valid)
  {
    if (!is_valid)
    {
      std::abort();
    }
  }
}


#if defined(NDEBUG) && !defined(ENABLE_CONTRACTS_IN_RELEASE)
  // Pre-condition
  #define Require(EXPR, ...) static_cast<void>(0)

  // Pre-condition (multiple conditions)
  #define RequireM(EXPR, ...) static_cast<void>(0)

  // Post-condition 
  #define Ensure(EXPR, ...) static_cast<void>(0)

  // Post-condition (multiple conditions)
  #define EnsureM(EXPR, ...) static_cast<void>(0)

  // Object invariant check
  #define Invariant(...) static_cast<void>(0)

  // Object invariant check (multiple conditions)
  #define InvariantM(...) static_cast<void>(0)

  // Pre-condition (flagged)
  #define RequireF(FLAGS, EXPR, ...) static_cast<void>(0)

  // Pre-condition (multiple conditions, flagged)
  #define RequireMF(FLAGS, EXPR, ...) static_cast<void>(0)

  // Pre-condition (flagged, expression)
  #define RequireFE(FLAGS, EXPR, ...) static_cast<void>(0)

  // Pre-condition (multiple conditions, flagged, expression)
  #define RequireMFE(FLAGS, EXPR, ...) static_cast<void>(0)

  // Post-condition (flagged)
  #define EnsureF(FLAGS, EXPR, ...) static_cast<void>(0)

  // Post-condition (multiple conditions, flagged)
  #define EnsureMF(FLAGS, EXPR, ...) static_cast<void>(0)

  // Post-condition (flagged, expression)
  #define EnsureFE(FLAGS, EXPR, ...) static_cast<void>(0)

  // Post-condition (multiple conditions, flagged, expression)
  #define EnsureMFE(FLAGS, EXPR, ...) static_cast<void>(0)

  // Object invariant check (flagged)
  #define InvariantF(FLAGS, ...) static_cast<void>(0)

  // Object invariant check (multiple conditions, flagged)
  #define InvariantMF(FLAGS, ...) static_cast<void>(0)

  // Object invariant check (flagged, expression)
  #define InvariantFE(FLAGS, ...) static_cast<void>(0)

  // Object invariant check (multiple conditions, flagged, expression)
  #define InvariantMFE(FLAGS, ...) static_cast<void>(0)

#else
  // Pre-condition
  #define Require(EXPR, ...) Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Require", __VA_ARGS__) ) 
  
  // Pre-condition (multiple conditions)
  #define RequireM(EXPR, ...) Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Require", __VA_ARGS__) ) 
  
  // Post-condition 
  #define Ensure(EXPR, ...) Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Ensure", __VA_ARGS__) ) 
  
  // Post-condition (multiple conditions)
  #define EnsureM(EXPR, ...) Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION,  "Ensure", __VA_ARGS__) ) 
  
  // Object invariant check
  #define Invariant(EXPR, ...) Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__) ) 
  
  // Object invariant check (multiple conditions)
  #define InvariantM(EXPR, ...) Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__) ) 
  
  // Pre-condition (flagged)
  #define RequireF(FLAGS, EXPR, ...) \
    if constexpr (CONTRACT_FLAGS & FLAGS)                                                                                                                    \
    {                                                                                                                                                        \
      Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Require", __VA_ARGS__));\
    }

  // Pre-condition (multiple conditions, flagged)
  #define RequireMF(FLAGS, EXPR, ...) \
    if constexpr (CONTRACT_FLAGS & FLAGS)                                                                                                                    \
    {                                                                                                                                                        \
      Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Require", __VA_ARGS__));\
    }

  // Post-condition (flagged)
  #define EnsureF(FLAGS, EXPR, ...) \
    if constexpr (CONTRACT_FLAGS & FLAGS)                                                                                                                    \
    {                                                                                                                                                        \
      Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Ensure", __VA_ARGS__)); \
    }

  // Post-condition (multiple conditions, flagged)
  #define EnsureMF(FLAGS, EXPR, ...) \
    if constexpr (CONTRACT_FLAGS & FLAGS)                                                                                                                    \
    {                                                                                                                                                        \
      Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Ensure", __VA_ARGS__)); \
    }

  // Object invariant check (flagged)
  #define InvariantF(FLAGS, EXPR, ...) \
    if constexpr (CONTRACT_FLAGS & FLAGS)                                                                                                                      \
    {                                                                                                                                                          \
      Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__));\
    }
  
  // Object invariant check (multiple conditions, flagged)
  #define InvariantMF(FLAGS, EXPR, ...) \
    if constexpr (CONTRACT_FLAGS & FLAGS)                                                                                                                      \
    {                                                                                                                                                          \
      Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__));\
    }

  // Pre-condition (flagged, expression)
  #define RequireFE(FLAGS, EXPR, ...) \
    CONTRACT_FLAGS & FLAGS ? Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Require", __VA_ARGS__)) : static_cast<void>(0)   
  
  // Pre-condition (multiple conditions, flagged, expression)
  #define RequireMFE(FLAGS, EXPR, ...) \
    CONTRACT_FLAGS & FLAGS ? Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Require", __VA_ARGS__)) : static_cast<void>(0)      
  
  // Object invariant check (flagged, expression)
  #define InvariantFE(FLAGS, EXPR, ...) \
    CONTRACT_FLAGS & FLAGS ? Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract((EXPR), #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__)) :  static_cast<void>(0) 
  
  // Object invariant check (multiple conditions, flagged, expression)
  #define InvariantMFE(FLAGS, EXPR, ...) \
    CONTRACT_FLAGS & FLAGS ? Validation::Contracts::RaiseAbort(Validation::Contracts::ResolveContract(Utils::Meta::Templates::MakeArray<bool> EXPR, #EXPR, __FILE__, __LINE__, CURRENT_FUNCTION, "Invariant", __VA_ARGS__)) :  static_cast<void>(0) 

#endif