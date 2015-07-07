/** PSP helper library ***************************************/
/**                                                         **/
/**                         util.c                          **/
/**                                                         **/
/** This file contains various utility routines             **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include "util.h"

#include <string.h>
#include <malloc.h>
#include <stdio.h>

#include "fileio.h"
#include "video.h"

int pspUtilSavePngSeq(const char *path, const char *filename, const PspImage *image)
{
  char *full_path;

  /* Allocate enough space for the file path */
  if (!(full_path = (char*)malloc(sizeof(char) * (strlen(path) + strlen(filename) + 10))))
    return 0;

  /* Loop until first free screenshot slot is found */
  int i = 0;
  do
  {
    sprintf(full_path, "%s%s-%02i.png", path, filename, i);
  } while (pspFileIoCheckIfExists(full_path) && ++i < 100);

  /* Save the screenshot */
  return pspImageSavePng(full_path, image);
}

int pspUtilSaveVramSeq(const char *path, const char *filename)
{
  PspImage* vram = pspVideoGetVramBufferCopy();
  if (!vram) return 0;

  int exit_code = pspUtilSavePngSeq(path, filename, vram);
  pspImageDestroy(vram);

  return exit_code;
}

