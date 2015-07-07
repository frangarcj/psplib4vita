/** PSP helper library ***************************************/
/**                                                         **/
/**                          ctrl.h                         **/
/**                                                         **/
/** This file contains declarations for the controller      **/
/** interaction routines                                    **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_CTRL_H
#define _PSP_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <psp2/ctrl.h>

/* These bits are currently unused */
#define PSP_CTRL_ANALUP    0x00000002
#define PSP_CTRL_ANALDOWN  0x00000004
#define PSP_CTRL_ANALLEFT  0x00000400
#define PSP_CTRL_ANALRIGHT 0x00000800

#define PSP_CTRL_NORMAL     0
#define PSP_CTRL_AUTOREPEAT 1

void pspCtrlInit();
int  pspCtrlGetPollingMode();
void pspCtrlSetPollingMode(int mode);
int  pspCtrlPollControls(SceCtrlData *pad);

#ifdef __cplusplus
}
#endif

#endif // _PSP_FILEIO_H
