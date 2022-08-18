#pragma once
#include <IO/ADT/Root/ADTRootMCNK.hpp>
#include <IO/ADT/ChunkIdentifiers.hpp>

namespace IO::ADT
{
  template
  <
    Common::ClientVersion client_version
    , std::default_initializable ReadContext
    , std::default_initializable WriteContext
  >
  MCNKRoot<client_version, ReadContext, WriteContext>::MCNKRoot()
  {
    _heightmap.Initialize();
    _normals.Initialize();
  }
}

