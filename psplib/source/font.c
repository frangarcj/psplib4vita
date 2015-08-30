/* psplib/font.c
   Rudimentary bitmap font implementation

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

#include "font.h"
#include "stockfont.h"
#include <vita2d.h>

PspFont PspStockFont;

int pspFontGetLineHeight(PspFont *font)
{
  if(!font->font){
    font->font=vita2d_load_font_mem(stockfont,stockfont_size);
    font->Height=pspFontGetLineHeight(font);
    font->Ascent=font->Height;
  }
  return vita2d_font_text_height(font->font,PSP_FONT_SIZE,"A");
}

int pspFontGetTextWidth(PspFont *font, const char *string)
{
  if(!font->font){
    font->font=vita2d_load_font_mem(stockfont,stockfont_size);
    font->Height=pspFontGetLineHeight(font);
    font->Ascent=font->Height;
  }
  return vita2d_font_text_width(font->font,PSP_FONT_SIZE,string);
}

int pspFontGetTextHeight(PspFont *font, const char *string)
{
  const char *ch;
  int lines;

  for (ch = string, lines = 1; *ch; ch++)
    if (*ch == '\n') lines++;

  return lines * pspFontGetLineHeight(font);
}
