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