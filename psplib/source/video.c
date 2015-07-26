/* psplib/video.c
   Graphics rendering routines

   Copyright (C) 2007-2008 Akop Karapetyan

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Author contact information: pspdev@akop.org
*/

/* TODO: move ScratchBuffer into VRAM */


#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <psp2/rtc.h>
#include <psp2/display.h>
#include <vita2d.h>


#include "video.h"

#define SLICE_SIZE 64


const unsigned int PspFontColor[] =
{
  0, /* Restore */
  PSP_COLOR_BLACK,
  PSP_COLOR_RED,
  PSP_COLOR_GREEN,
  PSP_COLOR_BLUE,
  PSP_COLOR_GRAY,
  PSP_COLOR_YELLOW,
  PSP_COLOR_MAGENTA,
  PSP_COLOR_WHITE
};

struct TexVertex
{
  unsigned short u, v;
  unsigned short color;
  short x, y, z;
};

static unsigned int  VBlankFreq;

static inline int PutChar(const PspFont *font, int sx, int sy, unsigned char sym, int color);

void pspVideoInit()
{

  vita2d_init_advanced(8*1024*1024);
  vita2d_set_vblank_wait(0);
  /* Compute VBlank frequency */
  uint64_t t[2];
  int i;
  for (i = 0; i < 2; i++)
  {
    sceDisplayWaitVblankStart();
    sceRtcGetCurrentTick(&t[i]);
  }
  VBlankFreq = round(1.00 / ((double)(t[1] - t[0])
    * (1.00 / (double)sceRtcGetTickResolution())));

}


void pspVideoBeginList(void *list)
{
  //sceGuStart(GU_CALL, list);
  vita2d_start_drawing();
}

void pspVideoBegin()
{
  vita2d_start_drawing();
  //sceGuStart(GU_DIRECT, List);
}

void pspVideoEnd()
{
  /*sceGuFinish();
  sceGuSync(0, 0);*/
  vita2d_end_drawing();
}

void pspVideoPutImage(const PspImage *image, int dx, int dy, int dw, int dh)
{
  vita2d_texture *tex = image->Texture;
  float scalex = (float)dw/(float)image->Viewport.Width;
  float scaley = (float)dh/(float)image->Viewport.Height;
  vita2d_draw_texture_part_scale(tex, dx, dy, image->Viewport.X, image->Viewport.Y, image->Viewport.Width, image->Viewport.Height, scalex, scaley);
}

void pspVideoPutImageAlpha(const PspImage *image, int dx, int dy, int dw, int dh,
                           unsigned char alpha)
{

  return pspVideoPutImage(image,dx,dy,dw,dh);

}

void pspVideoSwapBuffers()
{
  //VramOffset = sceGuSwapBuffers();
  vita2d_swap_buffers();
}

void pspVideoShutdown()
{
  vita2d_fini();
}

void pspVideoWaitVSync()
{
  sceDisplayWaitVblankStart();
}

void pspVideoDrawLine(int sx, int sy, int dx, int dy, uint32_t color)
{
  vita2d_draw_line(sx,sy,dx,dy,color);
}

void pspVideoDrawRect(int sx, int sy, int dx, int dy, uint32_t color)
{
    pspVideoDrawLine(sx,sy,sx,dy,color);
    pspVideoDrawLine(sx,dy,dx,dy,color);
    pspVideoDrawLine(dx,dy,dx,sy,color);
    pspVideoDrawLine(dx,sy,sx,sy,color);
}

void pspVideoShadowRect(int sx, int sy, int dx, int dy, uint32_t color, int depth)
{
  int i;
  uint32_t alpha;
  color &= ~0xff000000;

  for (i = depth, alpha = 0x30000000; i > 0; i--, alpha += 0x20000000)
  {
    pspVideoDrawLine(sx + i, dy + i, dx + i, dy + i, color | alpha);
    pspVideoDrawLine(dx + i, sy + i, dx + i, dy + i + 1, color | alpha);
  }
}

void pspVideoGlowRect(int sx, int sy, int dx, int dy, uint32_t color, int radius)
{
  int i;
  uint32_t alpha;
  color &= ~0xff000000;

  for (i = radius, alpha = 0x30000000; i > 0; i--, alpha += 0x20000000)
    pspVideoDrawRect(sx - i, sy - i, dx + i, dy + i, color | alpha);
}

void pspVideoFillRect(int sx, int sy, int dx, int dy, uint32_t color)
{
  vita2d_draw_rectangle(sx,sy,dx-sx,dy-sy,color);
}

void pspVideoCallList(const void *list)
{
  //sceGuCallList(list);
}

void pspVideoClearScreen()
{
  vita2d_clear_screen();
}

inline int PutChar(const PspFont *font, int sx, int sy, unsigned char sym, int color)
{
  /* Instead of a tab, skip 4 spaces */
  if (sym == (uint8_t)'\t')
    return font->Chars[(int)' '].Width * 4;

  /* This function should be rewritten to write directly to VRAM, probably */
  int h, i, j, w;
  w = font->Chars[(int)sym].Width;
  h = font->Height;

  unsigned short row;
  int shift;
  int pw = 1;

  /* Initialize pixel values */
  for (i = 0; i < h; i++)
  {
    for (j = 0; j < w; j++)
    {
      row = font->Chars[(int)sym].Char[i];
      shift = w - j;

      if (row & (1 << shift))
      {
        if (j == 0 || !(row & (1 << (shift + 1))))
        {vita2d_draw_rectangle((sx+j-1)*pw,(sy+i)*pw,pw,pw,0xff000000);}
        vita2d_draw_rectangle((sx+j)*pw,(sy+i)*pw,pw,pw,color);
        vita2d_draw_rectangle((sx+j+1)*pw,(sy+i)*pw,pw,pw,0xff000000);
      }
      else if (i > 0 && i < h - 1)
      {
        if ((i > 0) && (font->Chars[(int)sym].Char[i - 1] & (1 << shift)))
        { vita2d_draw_rectangle((sx+j)*pw,(sy+i)*pw,pw,pw,0xff000000);}
        else if ((i < h - 1) && (font->Chars[(int)sym].Char[i + 1] & (1 << shift)))
        { vita2d_draw_rectangle((sx+j)*pw,(sy+i)*pw,pw,pw,0xff000000);}
      }
    }
  }

  /*for (j = 0; j < w; j++)
    if (font->Chars[(int)sym].Char[h - 1] & (1 << (w - j)))
    { vita2d_draw_rectangle(sx+j,sy+h,0xff000000);}*/

  /* Return total width */
  return w;
}

int pspVideoPrint(const PspFont *font, int sx, int sy, const char *string, uint32_t color)
{
  return pspVideoPrintN(font, sx, sy, string, -1, color);
}

int pspVideoPrintCenter(const PspFont *font, int sx, int sy, int dx, const char *string, uint32_t color)
{
  const unsigned char *ch;
  int width, c = color, max;

  width = pspFontGetTextWidth(font, string);
  sx += (dx - sx) / 2 - width / 2;

  for (ch = (unsigned char*)string, width = 0, max = 0; *ch; ch++)
  {
    if (*ch < 32)
    {
      if (*ch >= PSP_FONT_RESTORE && *ch <= PSP_FONT_WHITE)
      {
        c = (*ch == PSP_FONT_RESTORE) ? color : PspFontColor[(int)(*ch) - PSP_FONT_RESTORE];
        continue;
      }
      else if (*ch == '\n')
      {
        sy += font->Height;
        width = 0;
        continue;
      }
    }

    width += PutChar(font, sx + width, sy, (uint8_t)(*ch), c);
    if (width > max) max = width;
  }

  return max;
}

int pspVideoPrintN(const PspFont *font, int sx, int sy, const char *string, int count, uint32_t color)
{
  const unsigned char *ch;
  int width, i, c = color, max;

  for (ch = (unsigned char*)string, width = 0, i = 0, max = 0; *ch && (count < 0 || i < count); ch++, i++)
  {
    if (*ch < 32)
    {
      if (*ch >= PSP_FONT_RESTORE && *ch <= PSP_FONT_WHITE)
      {
        c = (*ch == PSP_FONT_RESTORE) ? color : PspFontColor[(int)(*ch) - PSP_FONT_RESTORE];
        continue;
      }
      else if (*ch == '\n')
      {
        sy += font->Height;
        width = 0;
        continue;
      }
    }

    width += PutChar(font, sx + width, sy, (uint8_t)(*ch), c);
    if (width > max) max = width;
  }

  return max;
}

int pspVideoPrintClipped(const PspFont *font, int sx, int sy, const char* string, int max_w, char* clip, uint32_t color)
{
  int str_w = pspFontGetTextWidth(font, string);

  if (str_w <= max_w)
    return pspVideoPrint(font, sx, sy, string, color);

  int w, len;
  const char *ch;
  int clip_w = pspFontGetTextWidth(font, clip);

  for (ch=string, w=0, len=0; *ch && (w + clip_w < max_w); ch++, len++)
  {
    if (*ch == '\t') w += font->Chars[(uint8_t)' '].Width * 4;
    else w += font->Chars[(uint8_t)(*ch)].Width;
  }

  w = pspVideoPrintN(font, sx, sy, string, len - 1, color);
  pspVideoPrint(font, sx + w, sy, clip, color);

  return w + clip_w;
}

PspImage* pspVideoGetVramBufferCopy()
{
  int i, j;
  unsigned short *pixel;
  uint32_t *vram_addr = vita2d_get_current_fb();
  uint32_t pixv;
  PspImage *image;

  if (!(image = pspImageCreate(SCR_WIDTH, SCR_HEIGHT, PSP_IMAGE_16BPP)))
    return NULL;

  image->Viewport.Width = SCR_WIDTH;

  for (i = 0; i < image->Height; i++)
  {
    for (j = 0; j < image->Viewport.Width; j++)
    {
      pixel = (unsigned short*)image->Pixels + (i * image->Width + j);
      pixv = *(vram_addr + (i * BUF_WIDTH + j));
      *pixel = RGB(RED_32(pixv),GREEN_32(pixv),BLUE_32(pixv));
    }
  }

  return image;
}

void* pspVideoAllocateVramChunk(unsigned int bytes)
{
  return NULL;
}

unsigned int pspVideoGetVSyncFreq()
{
  return VBlankFreq;
}
