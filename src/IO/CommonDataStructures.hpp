#pragma once

namespace IO::Common::DataStructures
{
  struct C3Vector
  {
    float x;
    float y;
    float z;
  };

  struct CAaBox
  {
    C3Vector min;
    C3Vector max;
  };
}