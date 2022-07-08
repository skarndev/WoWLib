#pragma once

namespace IO::Common::WorldConstants
{
   /**
   * Size of one map tile (ADT) in yards.
   */
   constexpr float TILE_SIZE = 533.33333f;

   /**
    * Half size of one map tile (ADT) in yards.
    */
   constexpr float TILE_HALFSIZE = TILE_SIZE / 2.f;

   /**
    * Size of one map chunk (ADT::MCNK) in yards.
    */
   constexpr float CHUNK_SIZE = TILE_SIZE / 16.f;

   // Size of one mini chunk (ADT::MCNK / 4) in yards.
   constexpr float MINICHUNK_SIZE = CHUNK_SIZE / 4.f;

   // Size of one patch (ADT::MCNK / 8) in yards.
   constexpr float PATCH_SIZE = MINICHUNK_SIZE / 2.f;

   // Size of one alphamap (ADT::MCNK::MCAL) pixel in yards.
   constexpr float ALPHAMAP_PIXEL_SIZE = CHUNK_SIZE / 64.f;

   // Radius of one map chunk (ADT::MCNK) in yards.
   constexpr double MAPCHUNK_RADIUS = 47.140452079103168293389624140323; //sqrt((533.33333/16)^2 + (533.33333/16)^2)

   // Number of vertices per outer row of map chunk (ADT::MCNK) vertex buffers.
   constexpr unsigned N_VERTS_CHUNK_ROW_OUTER = 9;

   // Number of vertices per inner row of map chunk (ADT::MCNK) vertex buffers.
   constexpr unsigned N_VERTS_CHUNK_ROW_INNER = 8;

  /* Size (length) of map chunk (ADT::MCNK) vertex buffers.
      Map chunk buffer is:
         1    2    3    4    5    6    7    8    9
           10   11   12   13   14   15   16   17
         18   19   20   21   22   23   24   25   26
           27   28   29   30   31   32   33   34
         35   36   37   38   39   40   41   42   43
           44   45   46   47   48   49   50   51
         52   53   54   55   56   57   58   59   60
           61   62   63   64   65   66   67   68
         69   70   71   72   73   74   75   76   77
           78   79   80   81   82   83   84   85
         86   87   88   89   90   91   92   93   94
           95   96   97   98  99  100  101  102
        103  104  105  106  107  108  109  110  111
          112  113  114  115  116  117  118  119
        120  121  122  123  124  125  126  127  128
          129  130  131  132  133  134  135  136
        137  138  139  140  141  142  143  144  145

   */
   constexpr unsigned CHUNK_BUF_SIZE = N_VERTS_CHUNK_ROW_OUTER * N_VERTS_CHUNK_ROW_OUTER
       + N_VERTS_CHUNK_ROW_INNER * N_VERTS_CHUNK_ROW_INNER;

   /* Number of minichunks per map chunk (ADT::MCNK).
      Minichunk buffer is:
        x   x   x
          x   x
        x   x   x
          x   x
        x   x   x
   */
   constexpr unsigned MINICHUNKS_PER_CHUNK = 8;

   /* Number of patches per map chunk.
      Patch buffer is:
        x   x
          x
        x   x
   */
   constexpr unsigned PATCHES_PER_CHUNK = MINICHUNKS_PER_CHUNK * 4;

   // Number oof vertices per one map patch.
   constexpr unsigned VERTICES_PER_PATCH = 5;

   // Number of chunks (ADT::MCNK) per one map tile.
   constexpr unsigned CHUNKS_PER_TILE = 16 * 16;

   // Maximum number of tiles per map
   constexpr unsigned MAX_TILES_PER_MAP = 64 * 64;

   // Number of bytes per high resolution alphamap (ADT::MCNK::MCAL).
   constexpr unsigned N_BYTES_PER_HIGHRES_ALPHA = 4096;

   // Number of bytes per low resolution alphamap (ADT::MCNK::MCAL).
   constexpr unsigned N_BYTES_PER_LOWRES_ALPHA = 2048;

   // Alphamap dimension (width / height of an alphamap (ADT::MCNK::MCAL)).
   constexpr unsigned ALPHAMAP_DIM = 64;

   // Number of pixels per alphamap (ADT::MCNK::MCAL)
   constexpr unsigned N_PIXELS_PER_ALPHAMAP = N_BYTES_PER_HIGHRES_ALPHA;

   // Max number of texture layers (ADT::MCNK::MCAL / ADT::MCNK::MCLY) per chunk (ADT::MCNK).
   constexpr unsigned CHUNK_MAX_TEXTURE_LAYERS = 4;

   // Shadow map dimensions (width / height) of a shadowmap (ADT::MCNK::MCSH)
   constexpr unsigned SHADOWMAP_DIM = ALPHAMAP_DIM;

   // Number of pixels per shadowmap (ADT::MCNK::MCSH)
   constexpr unsigned N_PIXELS_PER_SHADOWMAP = SHADOWMAP_DIM * SHADOWMAP_DIM;
}