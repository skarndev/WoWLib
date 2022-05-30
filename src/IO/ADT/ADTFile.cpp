#include <IO/ADT/ADTFile.hpp>

using namespace IO::ADT;

ADTFile::ADTFile(std::uint32_t file_data_id)
: IChunkedFile(file_data_id)
{
  // register readers
  // common
  _common_chunks[ChunkIdentifiers::ADTCommonChunks::MVER] = &_version;

  //root
  _root_chunks[ChunkIdentifiers::ADTRootChunks::MHDR] = &_header;
  _root_chunks[ChunkIdentifiers::ADTRootChunks::MFBO] = &_flight_bounds;
  _root_chunks[ChunkIdentifiers::ADTRootChunks::MBMH] = &_blend_mesh_headers;
  _root_chunks[ChunkIdentifiers::ADTRootChunks::MBBB] = &_blend_mesh_bounding_boxes;
  _root_chunks[ChunkIdentifiers::ADTRootChunks::MBNV] = &_blend_mesh_vertices;
  _root_chunks[ChunkIdentifiers::ADTRootChunks::MBMI] = &_blend_mesh_indices;

  // tex
  _tex_chunks[ChunkIdentifiers::ADTTexChunks::MDID] = &_diffuse_texture_ids;
  _tex_chunks[ChunkIdentifiers::ADTTexChunks::MHID] = &_height_texture_ids;
  _tex_chunks[ChunkIdentifiers::ADTTexChunks::MTXF] = &_texture_flags;
  _tex_chunks[ChunkIdentifiers::ADTTexChunks::MTXP] = &_texture_params;
  _tex_chunks[ChunkIdentifiers::ADTTexChunks::MTCG] = &_texture_color_grading;
  _tex_chunks[ChunkIdentifiers::ADTTexChunks::MAMP] = &_texture_amplifier;

  // obj0
  _obj0_chunks[ChunkIdentifiers::ADTObj0Chunks::MODF] = &_wmo_instances;
  _obj0_chunks[ChunkIdentifiers::ADTObj0Chunks::MDDF] = &_model_instances;
  _obj0_chunks[ChunkIdentifiers::ADTObj0Chunks::MLMB] = &_mlmb;
  _obj0_chunks[ChunkIdentifiers::ADTObj0Chunks::MWDR] = &_wmo_doodadset_ranges;
  _obj0_chunks[ChunkIdentifiers::ADTObj0Chunks::MWDS] = &_wmo_dooodad_set_indices;

  // obj1
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLMD] = &_mlmd;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLMX] = &_mlmx;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDD] = &_mldd;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDX] = &_mldx;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDL] = &_mldl;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLFD] = &_mlfd;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLMB] = &_mlmb_obj1;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDB] = &_mldb;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MWDR] = &_mwdr_obj1;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MWDS] = &_mwds_obj1;



}

void ADTFile::Read(std::fstream const& fstream)
{

}

void ADTFile::Write(std::fstream const& fstream) const
{

}