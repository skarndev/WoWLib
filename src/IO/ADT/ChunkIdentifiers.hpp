#pragma once
#include <IO/Common.hpp>

#include <cstdint>

namespace IO::ADT::ChunkIdentifiers
{
  namespace ADTCommonChunks
  {
    enum eADTCommonChunks : std::uint32_t
    {
      MVER = IO::Common::FourCC<"MVER">
    };
  }
  namespace ADTRootChunks
  {
    enum eADTRootChunks : std::uint32_t
    {
      MHDR = IO::Common::FourCC<"MHDR">,
      MH2O = IO::Common::FourCC<"MH2O">,
      MCNK = IO::Common::FourCC<"MCNK">,
      MFBO = IO::Common::FourCC<"MFBO">,
      MBMH = IO::Common::FourCC<"MBMH">,
      MBBB = IO::Common::FourCC<"MBBB">,
      MBNV = IO::Common::FourCC<"MBNV">,
      MBMI = IO::Common::FourCC<"MBMI">
    };
  }
  namespace ADTRootMCNKSubchunks
  {
    enum eADTRootMCNKSubchunks : std::uint32_t
    {
      MCVT = IO::Common::FourCC<"MCVT">,
      MCLV = IO::Common::FourCC<"MCLV">,
      MCCV = IO::Common::FourCC<"MCCV">,
      MCNR = IO::Common::FourCC<"MCNR">,
      MCLQ = IO::Common::FourCC<"MCLQ">,
      MCSE = IO::Common::FourCC<"MCSE">,
      MCBB = IO::Common::FourCC<"MCBB">,
      MCDD = IO::Common::FourCC<"MCDD">
      
    };
  }
  namespace ADTTexChunks
  {
    enum eADTTexChunks : std::uint32_t
    {
      MDID = IO::Common::FourCC<"MDID">,
      MCNK = IO::Common::FourCC<"MCNK">,
      MHID = IO::Common::FourCC<"MHID">,
      MTXF = IO::Common::FourCC<"MTXF">,
      MTXP = IO::Common::FourCC<"MTXP">,
      MTCG = IO::Common::FourCC<"MTCG">,
      MAMP = IO::Common::FourCC<"MAMP">
    };
  }

  namespace ADTTexMCNKSubchunks
  {
    enum eADTTexMCNKSubchunks : std::uint32_t
    {
      MCLY = IO::Common::FourCC<"MCLY">,
      MCSH = IO::Common::FourCC<"MCSH">,
      MCAL = IO::Common::FourCC<"MCAL">,
      MCMT = IO::Common::FourCC<"MCMT">
    };
  }

  namespace ADTObjCommonChunks
  {
    enum eADTObjCommonChunks
    {
      MLMB = IO::Common::FourCC<"MLMB">,
      MWDR = IO::Common::FourCC<"MWDR">,
      MWDS = IO::Common::FourCC<"MWDS">
    };
  }

  namespace ADTObj0Chunks
  {
    enum eADTObj0Chunks : std::uint32_t
    {
      MDDF = IO::Common::FourCC<"MDDF">,
      MODF = IO::Common::FourCC<"MODF">,
      MCNK = IO::Common::FourCC<"MCNK">,
    };
  }

  namespace ADTObj0MCNKSubchunks
  {
    enum eADTObj0Chunks : std::uint32_t
    {
      MCRD = IO::Common::FourCC<"MCRD">,
      MCRW = IO::Common::FourCC<"MCRW">
    };
  }

  namespace ADTObj1Chunks
  {
    enum eADTObj1Chunks : std::uint32_t
    {
      MLMD = IO::Common::FourCC<"MLMD">,
      MLMX = IO::Common::FourCC<"MLMX">,
      MLDD = IO::Common::FourCC<"MLDD">,
      MLDX = IO::Common::FourCC<"MLDX">,
      MLDL = IO::Common::FourCC<"MLDL">,
      MLFD = IO::Common::FourCC<"MLFD">,
      MLDB = IO::Common::FourCC<"MLDB">
    };
  }

  namespace ADTObj1MCNKSubchunks
  {
    enum eADTObj0Chunks : std::uint32_t
    {
      MCRD = IO::Common::FourCC<"MCRD">,
      MCRW = IO::Common::FourCC<"MCRW">
    };
  }

  namespace ADTLodChunks
  {
    enum eADTLodChunks : std::uint32_t
    {
      MCBB = IO::Common::FourCC<"MCBB">,
      MBMH = IO::Common::FourCC<"MBMH">,
      MBBB = IO::Common::FourCC<"MBBB">,
      MBNV = IO::Common::FourCC<"MBNV">,
      MLHD = IO::Common::FourCC<"MLHD">,
      MLVH = IO::Common::FourCC<"MLVH">,
      MLVI = IO::Common::FourCC<"MLVI">,
      MLLL = IO::Common::FourCC<"MLLL">,
      MLND = IO::Common::FourCC<"MLND">,
      MLSI = IO::Common::FourCC<"MLSI">,
      MLLD = IO::Common::FourCC<"MLLD">,
      MLLN = IO::Common::FourCC<"MLLN">,
      MLLV = IO::Common::FourCC<"MLLV">,
      MLLI = IO::Common::FourCC<"MLLI">,
      MBMB = IO::Common::FourCC<"MBMB">
    };
  }


}

