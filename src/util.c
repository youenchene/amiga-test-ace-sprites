#include <ace/utils/bitmap.h>

tBitMap *xFlipInterleavedBitmap(tBitMap *pBitMap) {
  tBitMap *pSrc = pBitMap;
  UBYTE uwRowSize= pSrc->BytesPerRow;
  UWORD uwHeight = pSrc->Rows;
  UWORD uwWidth = pSrc->BytesPerRow*8;
  UWORD uwDepth = pSrc->Depth;
  tBitMap *pDest = bitmapCreate(uwWidth, uwHeight, uwDepth, BMF_CLEAR | BMF_INTERLEAVED);
    for (UWORD bitplane = 0; bitplane < uwDepth; ++bitplane) {
        for (UWORD y = 0; y < uwHeight; ++y) {
            for (UWORD x = 0; x < uwRowSize/2; ++x) {
                UBYTE *pSrcPlane = pSrc->Planes[bitplane];
                UBYTE *pDestPlane = pDest->Planes[bitplane];
                pDestPlane[y * uwRowSize + x] = pSrcPlane[y * uwRowSize + uwRowSize - 1 - x];
            }
        }
    }
  return pDest;
}
