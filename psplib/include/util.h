/** PSP helper library ***************************************/
/**                                                         **/
/**                         util.h                          **/
/**                                                         **/
/** This file contains declarations for various utility     **/
/** routines                                                **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_UTIL_H
#define _PSP_UTIL_H

#include "image.h"

#ifdef __cplusplus
extern "C" {
#endif

int pspUtilSavePngSeq(const char *path, const char *filename, const PspImage *image);
int pspUtilSaveVramSeq(const char *path, const char *filename);

#ifdef __cplusplus
}
#endif

#endif // _PSP_UTIL_H
