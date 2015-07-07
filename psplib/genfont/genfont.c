/** PSP helper library ***************************************/
/**                                                         **/
/**                          genfont.c                      **/
/**                                                         **/
/** This file contains a simple font converter. It converts **/
/** 'fd' format fonts (by Simon Tatham) into array          **/
/** used by the font rendering library                      **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include <stdio.h>

int main(int argc, char** argv)
{
  char line[256];
  int widths[256];
  int i,j,c;

  int ascent = 0;
  int height = 0;
  int ascii = 0;
  int chars = 0;

  scanf("height %i\n", &height);
  scanf("ascent %i\n", &ascent);

  printf("unsigned short _ch[][%i] = {\n", height);

  while (!feof(stdin))
  {
    if (scanf("\nchar %d\n", &ascii) < 1 || chars > 255)
      break;

    scanf("width %d\n", &widths[chars]);

    printf("  { ");

    for (i=0;i<height;i++)
    {
      scanf("%s", line);
      c=0;

      for (j=widths[chars];j>=0;j--)
        if (line[j]=='1')
          c|=1<<(widths[chars]-j-1);

      printf("0x%04x%s", c, (i<height-1) ? "," : "");
    }

    printf(" }, /* %i */\n", chars);
    chars++;
  }

  printf("};\n\nconst PspFont PspStockFont = \n{ %i, %i,\n  {\n", height, ascent);
  for (i = 0; i < 256; i++)
  {
    if (i % 4 == 0) printf("    ");
    printf("{ 0x%02x,_ch[0x%02x] },%s", widths[i], i, (i % 4 == 3) ? "\n" : "");
  }
  printf("  }\n};\n");

  return 0;
}
