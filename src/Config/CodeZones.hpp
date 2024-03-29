#pragma once  
// This file defines enums used to mark logging, contract validation and profiling zones of code.
// Must match with the corresponding CMake parameters.
// Enums are prefixed in the following way - L for logging, C for contracts, P for profiling.


namespace LCodeZones
{
  enum  eLCodeZones : unsigned
  {
    GRAPHICS = 0x1,
    CLIENT_HOOKS = 0x2,
    FILE_IO = 0x4,
    FILE_IO_DETAILS = 0x8
  };
}

namespace CCodeZones
{
  enum  eCCodeZones : unsigned
  {
    FILE_IO = 0x1,
    STORAGE = 0x2
  };
}

