/** PSP helper library ***************************************/
/**                                                         **/
/**                          psp.c                          **/
/**                                                         **/
/** This file contains core PSP management routines, such   **/
/** as program startup/shutdown, callbacks, etc...          **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include <time.h>
#include <malloc.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>

#include "psp.h"
#include "fileio.h"

struct PspCallback
{
  void (*Handler)(void *param);
  void *Param;
};

static struct PspCallback ExitCallback;
int ExitPSP;

static int _callback_thread(SceSize args, void* argp);
static int _callback(int notifyId, int notifyCount, int notifyArg, void *common);

static struct PspContext
{
  char *AppDirectory;
} pspC;

void pspInit(char *app_path)
{
  ExitPSP = 0;
  pspC.AppDirectory = pspFileIoGetParentDirectory(app_path);

  ExitCallback.Handler = NULL;
  ExitCallback.Param = NULL;
}

const char* pspGetAppDirectory()
{
  return pspC.AppDirectory;
}

void pspShutdown()
{
  free(pspC.AppDirectory);
  sceKernelExitProcess(0);
}

void pspSetClockFrequency(int freq)
{
  if (freq < 222) freq = 222;
  else if (freq > 333) freq = 333;
  //scePowerSetClockFrequency(freq, freq, freq/2);
}

int pspGetBatteryTime()
{
  return scePowerGetBatteryLifeTime();
}

int pspGetBatteryPercent()
{
  return scePowerGetBatteryLifePercent();
}

void pspGetTime(PspTime *time)
{
  time_t t;
  struct tm *tm;

  sceKernelLibcTime(&t);
  tm = localtime(&t);

  time->Second = tm->tm_sec;
  time->Minute = tm->tm_min;
  time->Hour = tm->tm_hour;
  time->Day = tm->tm_mday;
  time->Month = tm->tm_mon + 1;
  time->Year = 1900 + tm->tm_year;
}

static int _callback(int notifyId, int notifyCount, int notifyArg, void *common)
{
  struct PspCallback* callback = (struct PspCallback*)common;

  callback->Handler(callback->Param);
  return 0;
}

static int _callback_thread(SceSize args, void* argp)
{
  int cbid;

  if (ExitCallback.Handler)
  {
    cbid = sceKernelCreateCallback("Exit Callback", 0, _callback, &ExitCallback);
    //TODO sceKernelRegisterExitCallback(cbid);
  }

  //TODO
  sceKernelDelayThreadCB(1000000);

  //sceKernelSleepThreadCB();

  return 0;
}

int pspRegisterCallback(CallbackType type, void (*func)(void *param), void *param)
{
  switch (type)
  {
  case PSP_EXIT_CALLBACK:
    ExitCallback.Handler = func;
    ExitCallback.Param = param;
    break;
  default:
    return 0;
  }

  return 1;
}

int pspStartCallbackThread()
{
  int thid;
  //TODO "update_thread", _callback_thread,0x10000100, 0x10000, 0, 0, NULL
  if ((thid = sceKernelCreateThread("update_thread", _callback_thread, 0x11, 0xFA0, 0, 0,NULL)) < 0)
    return 0;

  sceKernelStartThread(thid, 0, NULL);

  return thid;
}
