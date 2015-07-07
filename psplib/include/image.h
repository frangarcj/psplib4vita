/** PSP helper library ***************************************/
/**                                                         **/
/**                         image.h                         **/
/**                                                         **/
/** This file contains declarations for image manipulation  **/
/** routines                                                **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_IMAGE_H
#define _PSP_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <psp2/gxm.h>

#define PSP_IMAGE_INDEXED 8
#define PSP_IMAGE_16BPP   16

#define GU_PSM_T8 SCE_GXM_TEXTURE_FORMAT_R8
#define GU_PSM_5551 SCE_GXM_TEXTURE_FORMAT_A1R5G5B5
	
typedef struct
{
  int X;
  int Y;
  int Width;
  int Height;
} PspViewport;

typedef struct
{
  int Width;
  int Height;
  void *Pixels;
  PspViewport Viewport;
  char FreeBuffer;
  char BytesPerPixel;
  char Depth;
  char PowerOfTwo;
  unsigned int TextureFormat;
  /* TODO: don't allocate if not necessary */
  unsigned short __attribute__((aligned(16))) Palette[256];
} PspImage;

/* Create/destroy */
PspImage* pspImageCreate(int width, int height, int bits_per_pixel);
PspImage* pspImageCreateVram(int width, int height, int bits_per_pixel);
PspImage* pspImageCreateOptimized(int width, int height, int bpp);
void      pspImageDestroy(PspImage *image);

PspImage* pspImageRotate(const PspImage *orig, int angle_cw);
PspImage* pspImageCreateThumbnail(const PspImage *image);
PspImage* pspImageCreateCopy(const PspImage *image);
void      pspImageClear(PspImage *image, unsigned int color);

PspImage* pspImageLoadPng(const char *path);
int       pspImageSavePng(const char *path, const PspImage* image);
PspImage* pspImageLoadPngFd(FILE *fp);
int       pspImageSavePngFd(FILE *fp, const PspImage* image);

int pspImageBlur(const PspImage *original, PspImage *blurred);
int pspImageDiscardColors(const PspImage *original);

#ifdef __cplusplus
}
#endif

#endif  // _PSP_IMAGE_H
