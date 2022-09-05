#pragma once

#if !defined(FORCEINLINE)
#  if defined(_MSC_VER)
#    define FORCEINLINE __forceinline
#  elif defined(__GNUC__) && __GNUC__ > 3
// Clang also defines __GNUC__ (as 4)
#    define FORCEINLINE inline __attribute__ ((__always_inline__))
#  else
#    define FORCEINLINE inline
#  endif
#endif

#if !defined(FORCEINLINE_ATTR)
#  if defined(__clang__)
#     define FORCEINLINE_ATTR __attribute__ ((__always_inline__))
#  elif defined(_MSC_VER)
#    define FORCEINLINE_ATTR [[msvc::forceinline]]
#  elif defined(__GNUC__) || defined(__GNUG__)
#    define FORCEINLINE_ATTR [[gnu::always_inline]]
#  endif
#endif
