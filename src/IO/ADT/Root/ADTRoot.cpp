#include <IO/ADT/Root/ADTRoot.hpp>

using namespace IO::ADT;

bool BlendMeshes::Read(Common::ByteBuffer const& buf, Common::ChunkHeader const& chunk_header)
{
  switch (chunk_header.fourcc)
  {
    case ChunkIdentifiers::ADTRootChunks::MBMH:
      _blend_mesh_headers.Read(buf, chunk_header.size);
      return true;
    case ChunkIdentifiers::ADTRootChunks::MBBB:
      _blend_mesh_bounding_boxes.Read(buf, chunk_header.size);
      return true;
    case ChunkIdentifiers::ADTRootChunks::MBNV:
      _blend_mesh_vertices.Read(buf, chunk_header.size);
      return true;
    case ChunkIdentifiers::ADTRootChunks::MBMI:
      _blend_mesh_indices.Read(buf, chunk_header.size);
      return true;
  }

  return false;
}

void BlendMeshes::Write(Common::ByteBuffer& buf) const
{
  _blend_mesh_headers.Write(buf);
  _blend_mesh_bounding_boxes.Write(buf);
  _blend_mesh_vertices.Write(buf);
  _blend_mesh_indices.Write(buf);
}

