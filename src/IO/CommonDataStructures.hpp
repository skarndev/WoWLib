#pragma once

namespace IO::Common::DataStructures
{
  struct C3Vector
  {
    float x;
    float y;
    float z;
  };

  struct C2Vector
  {
    float x;
    float y;
  };

  struct CAaBox
  {
    C3Vector min;
    C3Vector max;
  };

  struct CArgb // todo: verify, add CRgba, ..?
  {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
  };

  struct CRange
  {
    float min;
    float max;
  };

  struct CAaSphere
  {
    C3Vector position;
    float radius;
  };
}