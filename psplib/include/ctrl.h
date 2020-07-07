/* psplib/ctrl.h
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

#ifndef _PSP_CTRL_H
#define _PSP_CTRL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <psp2/ctrl.h>

enum {
	PSP_CTRL_SELECT	= SCE_CTRL_SELECT,	//!< Select button.
	PSP_CTRL_START		= SCE_CTRL_START,	//!< Start button.
	PSP_CTRL_UP		= SCE_CTRL_UP,	//!< Up D-Pad button.
	PSP_CTRL_RIGHT		= SCE_CTRL_RIGHT,	//!< Right D-Pad button.
	PSP_CTRL_DOWN		= SCE_CTRL_DOWN,	//!< Down D-Pad button.
	PSP_CTRL_LEFT		= SCE_CTRL_LEFT,	//!< Left D-Pad button.
	PSP_CTRL_LTRIGGER	= SCE_CTRL_LTRIGGER,	//!< Left trigger.
	PSP_CTRL_RTRIGGER	= SCE_CTRL_RTRIGGER,	//!< Right trigger.
	PSP_CTRL_TRIANGLE	= SCE_CTRL_TRIANGLE,	//!< Triangle button.
	PSP_CTRL_CIRCLE	= SCE_CTRL_CIRCLE,	//!< Circle button.
	PSP_CTRL_CROSS		= SCE_CTRL_CROSS,	//!< Cross button.
	PSP_CTRL_SQUARE	= SCE_CTRL_SQUARE,	//!< Square button.
	PSP_CTRL_ANY		= SCE_CTRL_INTERCEPTED	//!< Any input intercepted.
};

/** Controller mode. */

enum {
	/** Digitial buttons only. */
	PSP_CTRL_MODE_DIGITAL = SCE_CTRL_MODE_DIGITAL,
	/** Digital buttons + Analog support. */
	PSP_CTRL_MODE_ANALOG = SCE_CTRL_MODE_ANALOG,
	/** Same as ::SCE_CTRL_MODE_ANALOG, but with larger range for analog sticks. */
	PSP_CTRL_MODE_ANALOG_WIDE = SCE_CTRL_MODE_ANALOG_WIDE
};


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
