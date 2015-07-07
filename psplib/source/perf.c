/** PSP helper library ***************************************/
/**                                                         **/
/**                          perf.c                         **/
/**                                                         **/
/** This file contains performance-related PSP routines     **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/

#include <psp2/rtc.h>

#include "perf.h"

void pspPerfInitFps(PspFpsCounter *counter)
{
  counter->Fps = 0;
  counter->FrameCount = 0;
  counter->TicksPerSecond = (float)sceRtcGetTickResolution();
  sceRtcGetCurrentTick(&counter->LastTick);
}

float pspPerfGetFps(PspFpsCounter *counter)
{
  uint64_t current_tick;
  sceRtcGetCurrentTick(&current_tick);

  counter->FrameCount++;
  if (current_tick - counter->LastTick >= counter->TicksPerSecond)
  {
    /* A second elapsed; recompute FPS */
    counter->Fps = (float)counter->FrameCount
      / (float)((current_tick - counter->LastTick) / counter->TicksPerSecond);
    counter->LastTick = current_tick;
    counter->FrameCount = 0;
  }

  return counter->Fps;
}

/*
void pspPerfRecalibrateUpdates(Flack *updater, int update_freq, int frame_skip)
{
  updater->TicksPerSecond = (float)sceRtcGetTickResolution();
  updater->UpdateFreq = update_freq;

  if (updater->UpdateFreq)
  {
    updater->TicksPerUpdate = updater->TicksPerSecond
      / (updater->UpdateFreq / (frame_skip + 1));
    sceRtcGetCurrentTick(&updater->LastTick);
  }
}

void pspPerfWaitForUpdate(Flack *updater)
{
  uint64_t current_tick;

  if (updater->UpdateFreq)
  {
    do { sceRtcGetCurrentTick(&current_tick); }
    while (current_tick - updater->LastTick < updater->TicksPerUpdate);
    updater->LastTick = current_tick;
  }
}
*/
