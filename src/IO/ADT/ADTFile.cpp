#include <IO/ADT/ADTFile.hpp>
#include <IO/ADT/Obj/ADTObj.hpp>
#include <IO/ADT/Tex/ADTTex.hpp>

using namespace IO::ADT;

ADTFile::ADTFile(std::uint32_t file_data_id)
{

  Common::ByteBuffer buf{};
  ADTObj<Common::ClientVersion::SL, ADTObjLodLevel::NORMAL> normal_sl{1};
  ADTObj<Common::ClientVersion::SL, ADTObjLodLevel::LOD> lod_sl{1};
  normal_sl.Read(buf);
  normal_sl.Write(buf);
  lod_sl.Read(buf);
  lod_sl.Write(buf);


  ADTObj<Common::ClientVersion::BFA, ADTObjLodLevel::NORMAL> normal_bfa{1};
  ADTObj<Common::ClientVersion::BFA, ADTObjLodLevel::LOD> lod_bfa{1};

  normal_bfa.Read(buf);
  normal_bfa.Write(buf);
  lod_bfa.Read(buf);
  lod_bfa.Write(buf);

  ADTObj<Common::ClientVersion::LEGION, ADTObjLodLevel::NORMAL> normal_legion{1};
  ADTObj<Common::ClientVersion::LEGION, ADTObjLodLevel::LOD> lod_legion{1};

  normal_legion.Read(buf);
  normal_legion.Write(buf);
  lod_legion.Read(buf);
  lod_legion.Write(buf);

  ADTObj<Common::ClientVersion::WOD, ADTObjLodLevel::NORMAL> normal_wod{1};

  normal_wod.Read(buf);
  normal_wod.Write(buf);

  ADTObj<Common::ClientVersion::MOP, ADTObjLodLevel::NORMAL> normal_mop{1};

  normal_mop.Read(buf);
  normal_mop.Write(buf);

  ADTObj<Common::ClientVersion::CATA, ADTObjLodLevel::NORMAL> normal_cata{1};

  normal_cata.Read(buf);
  normal_cata.Write(buf);

  // tex

  ADTTex<Common::ClientVersion::SL> tex_sl{1};
  tex_sl.Read(buf, MCAL::AlphaFormat::HIGHRES, true);
  tex_sl.Write(buf, MCAL::AlphaFormat::HIGHRES);

  ADTTex<Common::ClientVersion::LEGION> tex_legion{1};
  tex_legion.Read(buf, MCAL::AlphaFormat::HIGHRES, true);
  tex_legion.Write(buf, MCAL::AlphaFormat::HIGHRES);

  ADTTex<Common::ClientVersion::BFA> tex_bfa{1};
  tex_bfa.Read(buf, MCAL::AlphaFormat::HIGHRES, true);
  tex_bfa.Write(buf, MCAL::AlphaFormat::HIGHRES);

  ADTTex<Common::ClientVersion::WOD> tex_wod{1};
  tex_wod.Read(buf, MCAL::AlphaFormat::HIGHRES, true);
  tex_wod.Write(buf, MCAL::AlphaFormat::HIGHRES);

  ADTTex<Common::ClientVersion::MOP> tex_mop{1};
  tex_mop.Read(buf, MCAL::AlphaFormat::HIGHRES, true);
  tex_mop.Write(buf, MCAL::AlphaFormat::HIGHRES);

  ADTTex<Common::ClientVersion::CATA> tex_cata{1};
  tex_cata.Read(buf, MCAL::AlphaFormat::HIGHRES, true);
  tex_cata.Write(buf, MCAL::AlphaFormat::HIGHRES);

  // register readers
  // common
 
  /*
  
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
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLMD] = &_wmo_instanced_lod;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLMX] = &_wmo_instances_lod_extents;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDD] = &_model_instances_lod;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDX] = &_model_instances_lod_extents;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDL] = &_mldl;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLFD] = &_object_lod_levels;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLMB] = &_mlmb_obj1;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MLDB] = &_mldb;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MWDR] = &_wmo_doodadset_ranges_lod;
  _obj1_chunks[ChunkIdentifiers::ADTObj1Chunks::MWDS] = &_wmo_doodad_set_indices_lod;

  */

}

