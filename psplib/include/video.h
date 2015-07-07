/** PSP helper library ***************************************/
/**                                                         **/
/**                          video.h                        **/
/**                                                         **/
/** This file contains declarations for the video rendering **/
/** library                                                 **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_VIDEO_H
#define _PSP_VIDEO_H

#include <psp2/types.h>

#include "font.h"
#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_COLOR_WHITE	   (uint32_t)0xffffffff
#define PSP_COLOR_BLACK	   (uint32_t)0xff000000
#define PSP_COLOR_GRAY	   (uint32_t)0xffcccccc
#define PSP_COLOR_DARKGRAY (uint32_t)0xff777777
#define PSP_COLOR_RED	     (uint32_t)0xff0000ff
#define PSP_COLOR_GREEN	   (uint32_t)0xff00ff00
#define PSP_COLOR_BLUE	   (uint32_t)0xffff0000
#define PSP_COLOR_YELLOW   (uint32_t)0xff00ffff
#define PSP_COLOR_MAGENTA  (uint32_t)0xffff00ff

#define PSP_VIDEO_UNSCALED    0
#define PSP_VIDEO_FIT_HEIGHT  1
#define PSP_VIDEO_FILL_SCREEN 2

#define BUF_WIDTH 512
#define SCR_WIDTH 480
#define SCR_HEIGHT 272

#define COLOR(r,g,b,a) (((int)(r)&0xff)|(((int)(g)&0xff)<<8)|\
  (((int)(b)&0xff)<<16)|(((int)(a)&0xff)<<24))

#define RGB(r,g,b)   (((((b)>>3)&0x1F)<<10)|((((g)>>3)&0x1F)<<5)|\
  (((r)>>3)&0x1F)|0x8000)
#define RED(pixel)   ((((pixel))&0x1f)*0xff/0x1f)
#define GREEN(pixel) ((((pixel)>>5)&0x1f)*0xff/0x1f)
#define BLUE(pixel)  ((((pixel)>>10)&0x1f)*0xff/0x1f)

#define RGB_32(r,g,b)    COLOR(r,g,b,0xff)
#define RGBA_32(r,g,b,a) COLOR(r,g,b,a)

#define RED_32(c)   ((c)&0xff)
#define GREEN_32(c) (((c)>>8)&0xff)
#define BLUE_32(c)  (((c)>>16)&0xff)
#define ALPHA_32(c) (((c)>>24)&0xff)

extern const unsigned int PspFontColor[];

typedef struct PspVertex
{
  unsigned int color;
  short x, y, z;
} PspVertex;

void pspVideoInit();
void pspVideoShutdown();
void pspVideoClearScreen();
void pspVideoWaitVSync();
void pspVideoSwapBuffers();

void pspVideoBegin();
void pspVideoEnd();

void pspVideoDrawLine(int sx, int sy, int dx, int dy, uint32_t color);
void pspVideoDrawRect(int sx, int sy, int dx, int dy, uint32_t color);
void pspVideoFillRect(int sx, int sy, int dx, int dy, uint32_t color);

int pspVideoPrint(const PspFont *font, int sx, int sy, const char *string, uint32_t color);
int pspVideoPrintCenter(const PspFont *font, int sx, int sy, int dx, const char *string, uint32_t color);
int pspVideoPrintN(const PspFont *font, int sx, int sy, const char *string, int count, uint32_t color);
int pspVideoPrintClipped(const PspFont *font, int sx, int sy, const char* string, int max_w, char* clip, uint32_t color);
int pspVideoPrintNRaw(const PspFont *font, int sx, int sy, const char *string, int count, uint32_t color);
int pspVideoPrintRaw(const PspFont *font, int sx, int sy, const char *string, uint32_t color);

void pspVideoPutImage(const PspImage *image, int dx, int dy, int dw, int dh);

void pspVideoGlowRect(int sx, int sy, int dx, int dy, uint32_t color, int radius);
void pspVideoShadowRect(int sx, int sy, int dx, int dy, uint32_t color, int depth);

PspImage* pspVideoGetVramBufferCopy();

void pspVideoBeginList(void *list);
void pspVideoCallList(const void *list);

void* pspVideoAllocateVramChunk(unsigned int bytes);

unsigned int pspVideoGetVSyncFreq();

#ifdef __cplusplus
}
#endif

#endif  // _PSP_VIDEO_H
