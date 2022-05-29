#pragma once
#include <IO/Common.hpp>
#include <IO/ADT/DataStructures.hpp>

#include <cstdint>

namespace IO::ADT
{
  class ADTFile : public IO::Common::IChunkedFile
  {
  public:
    ADTFile(std::uint32_t file_data_id);

    void Read(std::fstream const& fstream) override;
    void Write(std::fstream const& fstream) const override;

  private:

    // all
    Common::DataChunk<std::uint32_t> _mver;
    
    // root
    Common::DataChunk<DataStructures::MHDR> _mhdr;
    // todo: mh20, mcnks
    Common::DataChunk<DataStructures::MFBO> _mfbo;
    Common::DataChunk<DataStructures::MFBO> _mbmh;
    Common::DataChunk<DataStructures::MBBB> _mbbb;
    Common::DataChunk<DataStructures::MBNV> _mbnv;
    Common::DataChunk<std::uint16_t> _mbmi;

    // tex
    Common::DataChunk<std::uint32_t> _mdid;
    Common::DataChunk<std::uint32_t> _mhid;
    Common::DataChunk<DataStructures::SMTextureFlags> _mtxf;
    Common::DataChunk<DataStructures::SMTextureParams> _mtxp;
    Common::DataChunk<DataStructures::MTCG> _mtcg;
    Common::DataChunk<char> _mamp;
    // todo: mcnks

    // obj0
    Common::DataChunk<DataStructures::MODF> _modf;
    Common::DataChunk<DataStructures::MDDF> _mddf;
    // todo: mcnks
    Common::DataChunk<DataStructures::MLMB> _mlmb;
    Common::DataChunk<DataStructures::MWDR> _mwdr;
    Common::DataChunk<std::uint16_t> _mwds;


    // obj1
    Common::DataChunk<DataStructures::MODF> _modf_obj1;
    Common::DataChunk<DataStructures::MDDF> _mddf_obj1;
    // todo: mcnks
    Common::DataChunk<DataStructures::MLMD> _mlmd;
    Common::DataChunk<DataStructures::MLMX> _mlmx;
    Common::DataChunk<DataStructures::MDDF> _mldd;
    Common::DataChunk<DataStructures::MLDX> _mldx;
    Common::DataChunk<std::uint32_t> _mldl;
    Common::DataChunk<DataStructures::MLFD> _mlfd;
    Common::DataChunk<DataStructures::MLMB> _mlmb_obj1;
    Common::DataChunk<char> _mldb;
    Common::DataChunk<DataStructures::MWDR> _mwdr_obj1;
    Common::DataChunk<std::uint16_t> _mwds_obj1;

  };
}