/** PSP helper library ***************************************/
/**                                                         **/
/**                          perf.h                         **/
/**                                                         **/
/** This file contains declarations for performance-related **/
/** PSP routines                                            **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_PERF_H
#define _PSP_PERF_H

#include <psp2/types.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PspFpsCounter
{
  float TicksPerSecond;
  int FrameCount;
  uint64_t LastTick;
  float Fps;
} PspFpsCounter;

void  pspPerfInitFps(PspFpsCounter *counter);
float pspPerfGetFps(PspFpsCounter *counter);

#ifdef __cplusplus
}
#endif

#endif // _PSP_PERF_H
