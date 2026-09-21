/*-------------------------------------------------------------*/
/*--- Trimmed public header for the vendored libbzip2 1.0.8  ---*/
/*--- decompression-only port (decompress.c/huffman.c/       ---*/
/*--- crctable.c/randtable.c below are the unmodified         ---*/
/*--- upstream sources; see ../Bz2Decompress.h for the        ---*/
/*--- reason this exists: real Reticulum's Resource class     ---*/
/*--- bz2-compresses anything sent via Link/DIRECT delivery    ---*/
/*--- by default, and this firmware must be able to decode     ---*/
/*--- that even though it never produces compressed output    ---*/
/*--- itself (Resource.cpp forces _auto_compress=false).      ---*/
/*---                                                          ---*/
/*--- Upstream: bzip2/libbzip2 1.0.8, Copyright (C) 1996-2019  ---*/
/*--- Julian Seward <jseward@acm.org>, BSD-style license, see  ---*/
/*--- LICENSE in this directory. This header is cut down to    ---*/
/*--- just the buffer-based low-level API (no FILE-handle/     ---*/
/*--- stdio-                                                    ---*/
/*--- based convenience functions, no Windows DLL import       ---*/
/*--- machinery) - everything removed here is unreachable      ---*/
/*--- from decompress.c/huffman.c/crctable.c/randtable.c, so   ---*/
/*--- there is nothing to keep in sync with a real bzlib.h.   ---*/
/*-------------------------------------------------------------*/

#ifndef _BZLIB_H
#define _BZLIB_H

#ifdef __cplusplus
extern "C" {
#endif

#define BZ_RUN               0
#define BZ_FLUSH              1
#define BZ_FINISH             2

#define BZ_OK                0
#define BZ_RUN_OK            1
#define BZ_FLUSH_OK          2
#define BZ_FINISH_OK         3
#define BZ_STREAM_END        4
#define BZ_SEQUENCE_ERROR    (-1)
#define BZ_PARAM_ERROR       (-2)
#define BZ_MEM_ERROR         (-3)
#define BZ_DATA_ERROR        (-4)
#define BZ_DATA_ERROR_MAGIC  (-5)
#define BZ_IO_ERROR          (-6)
#define BZ_UNEXPECTED_EOF    (-7)
#define BZ_OUTBUFF_FULL      (-8)
#define BZ_CONFIG_ERROR      (-9)

typedef
   struct {
      char *next_in;
      unsigned int avail_in;
      unsigned int total_in_lo32;
      unsigned int total_in_hi32;

      char *next_out;
      unsigned int avail_out;
      unsigned int total_out_lo32;
      unsigned int total_out_hi32;

      void *state;

      void *(*bzalloc)(void *,int,int);
      void (*bzfree)(void *,void *);
      void *opaque;
   }
   bz_stream;

#define BZ_EXTERN extern
#define BZ_API(func) func

/*-- Core (low-level) library functions - decompression only. --*/

BZ_EXTERN int BZ_API(BZ2_bzDecompressInit) (
      bz_stream *strm,
      int       verbosity,
      int       small
   );

BZ_EXTERN int BZ_API(BZ2_bzDecompress) (
      bz_stream* strm
   );

BZ_EXTERN int BZ_API(BZ2_bzDecompressEnd)  (
      bz_stream *strm
   );

/*-- Utility function - decompression only. --*/

BZ_EXTERN int BZ_API(BZ2_bzBuffToBuffDecompress) (
      char*         dest,
      unsigned int* destLen,
      char*         source,
      unsigned int  sourceLen,
      int           small,
      int           verbosity
   );

#ifdef __cplusplus
}
#endif

#endif

/*-------------------------------------------------------------*/
/*--- end                                           bzlib.h ---*/
/*-------------------------------------------------------------*/
