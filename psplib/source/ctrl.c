/* psplib/ctrl.c
   Controller routines

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

#include <time.h>
#include <psp2/ctrl.h>
#include <psp2/rtc.h>

#include "ctrl.h"

#define PSP_CTRL_DELAY     400
#define PSP_CTRL_THRESHOLD 50
#define PSP_CTRL_BUTTONS   12

static int PollingMode;
static uint64_t PushTime[PSP_CTRL_BUTTONS];
static const int ButtonMap[PSP_CTRL_BUTTONS] =
{
  PSP2_CTRL_UP,
  PSP2_CTRL_DOWN,
  PSP2_CTRL_LEFT,
  PSP2_CTRL_RIGHT,
  PSP2_CTRL_CROSS,
  PSP2_CTRL_CIRCLE,
  PSP2_CTRL_SQUARE,
  PSP2_CTRL_TRIANGLE,
  PSP2_CTRL_LTRIGGER,
  PSP2_CTRL_RTRIGGER,
  PSP2_CTRL_SELECT,
  PSP2_CTRL_START,
};

void pspCtrlInit()
{
  /* Init PSP controller */
  sceCtrlSetSamplingMode(PSP2_CTRL_MODE_ANALOG);

  PollingMode = PSP_CTRL_NORMAL;
}

int pspCtrlGetPollingMode()
{
  return PollingMode;
}

void pspCtrlSetPollingMode(int mode)
{
  /* If autorepeat is being shut off, wait until it's "safe" */
  if (PollingMode == PSP_CTRL_AUTOREPEAT)
  {
    int i;
    SceCtrlData p;
    uint64_t tick;
    int wait;

    do
    {
      wait = 0;

      if (!sceCtrlPeekBufferPositive(0,&p, 1))
        break;

      /* Get current tick count */
      sceRtcGetCurrentTick(&tick);

      /* If at least one button is being held, wait until */
      /* next autorepeat interval, or until it's released */
      for (i = 0; i < PSP_CTRL_BUTTONS; i++)
        if (tick < PushTime[i] && (p.buttons & ButtonMap[i]))
          { wait = 1; break; }
    }
    while (wait);
  }

  PollingMode = mode;

  /* If autorepeat is being turned on, initialize autorepeat data */
  if (mode == PSP_CTRL_AUTOREPEAT)
  {
    SceCtrlData p;
    int i;
    uint64_t tick;
    uint32_t tick_res;

    /* Poll the controls */
    if (sceCtrlPeekBufferPositive(0, &p, 1))
    {
      /* Get current tick count */
      sceRtcGetCurrentTick(&tick);
      tick_res = sceRtcGetTickResolution();

      /* Check each button */
      for (i = 0; i < PSP_CTRL_BUTTONS; i++)
        PushTime[i] = (p.buttons & ButtonMap[i]) ? tick + PSP_CTRL_DELAY * (tick_res / 1000) : 0;
    }
  }
}

int pspCtrlPollControls(SceCtrlData *pad)
{
  int stat;

  /* Simulate button autorepeat */
  if (PollingMode == PSP_CTRL_AUTOREPEAT)
  {
    SceCtrlData p;
    int stat, i;
    uint64_t tick;
    uint32_t tick_res;

    /* Poll the controls */
    if (!(stat = sceCtrlPeekBufferPositive(0, &p, 1)))
      return stat;

    /* Get current tick count */
    sceRtcGetCurrentTick(&tick);
    tick_res = sceRtcGetTickResolution();

    /* Check each button */
    for (i = 0; i < PSP_CTRL_BUTTONS; i++)
    {
      if (p.buttons & ButtonMap[i])
      {
        if (!PushTime[i] || tick >= PushTime[i])
        {
          /* Button was pushed for the first time, or time to repeat */
          pad->buttons |= ButtonMap[i];
          /* Compute next press time */
          PushTime[i] = tick + ((PushTime[i]) ? PSP_CTRL_THRESHOLD : PSP_CTRL_DELAY)
            * (tick_res / 1000);
        }
        else
        {
          /* No need to repeat yet */
          pad->buttons &= ~ButtonMap[i];
        }
      }
      else
      {
        /* Button was released */
        pad->buttons &= ~ButtonMap[i];
        PushTime[i] = 0;
      }
    }

    /* Copy analog stick status */
    pad->lx = p.lx;
    pad->ly = p.ly;

    /* Unset the analog stick bits */
    pad->buttons &= ~(PSP_CTRL_ANALUP
      | PSP_CTRL_ANALDOWN
      | PSP_CTRL_ANALLEFT
      | PSP_CTRL_ANALRIGHT);

    /* Set the bits based on analog stick status */
    if (pad->ly < 32) pad->buttons |= PSP_CTRL_ANALUP;
    else if (pad->ly >= 224) pad->buttons |= PSP_CTRL_ANALDOWN;
    if (pad->lx < 32) pad->buttons |= PSP_CTRL_ANALLEFT;
    else if (pad->lx >= 224) pad->buttons |= PSP_CTRL_ANALRIGHT;

    return stat;
  }

  /* Default is normal behavior */
  if (!(stat = sceCtrlPeekBufferPositive(0, pad, 1)))
    return stat;

  /* Unset the analog stick bits */
  pad->buttons &= ~(PSP_CTRL_ANALUP
    | PSP_CTRL_ANALDOWN
    | PSP_CTRL_ANALLEFT
    | PSP_CTRL_ANALRIGHT);

  /* Set the bits based on analog stick status */
  if (pad->ly < 32) pad->buttons |= PSP_CTRL_ANALUP;
  else if (pad->ly >= 224) pad->buttons |= PSP_CTRL_ANALDOWN;
  if (pad->lx < 32) pad->buttons |= PSP_CTRL_ANALLEFT;
  else if (pad->lx >= 224) pad->buttons |= PSP_CTRL_ANALRIGHT;

  return stat;
}
