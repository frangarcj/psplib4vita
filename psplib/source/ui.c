/** PSP helper library ***************************************/
/**                                                         **/
/**                           ui.c                          **/
/**                                                         **/
/** This file contains a simple GUI rendering library       **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include <malloc.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <psp2/rtc.h>
#include <psp2/power.h>


#include "psp.h"
#include "fileio.h"
#include "ctrl.h"
#include "ui.h"
#include "font.h"

#define MAX_DIR_LEN 1024

#define UI_ANIM_FRAMES   8
#define UI_ANIM_FOG_STEP 0x0f

#define CONTROL_BUTTON_MASK \
  (PSP2_CTRL_CIRCLE | PSP2_CTRL_TRIANGLE | PSP2_CTRL_CROSS | PSP2_CTRL_SQUARE | \
   PSP2_CTRL_LTRIGGER | PSP2_CTRL_RTRIGGER | PSP2_CTRL_SELECT | PSP2_CTRL_START)

static const char
  *AlertDialogButtonTemplate   = "\026\001\020/\026\002\020 Close",
  *ConfirmDialogButtonTemplate = "\026\001\020 Confirm\t\026\002\020 Cancel",
  *YesNoCancelDialogButtonTemplate =
    "\026\001\020 Yes\t\026"PSP_CHAR_SQUARE"\020 No\t\026\002\020 Cancel",

  *SelectorTemplate = "\026\001\020 Confirm\t\026\002\020 Cancel",

  *BrowserTemplates[] = {
    "\026\002\020 Cancel\t\026\001\020 Open",
    "\026\002\020 Cancel\t\026\001\020 Enter directory",
    "\026\002\020 Cancel\t\026\001\020 Open\t\026"PSP_CHAR_TRIANGLE"\020 Parent directory",
    "\026\002\020 Cancel\t\026\001\020 Enter directory\t\026"PSP_CHAR_TRIANGLE"\020 Parent directory"
   },

  *SplashStatusBarTemplate  = "\026\255\020/\026\256\020 Switch tabs",

  *OptionModeTemplate =
    "\026\245\020/\026\246\020 Select\t\026\247\020/\026\002\020 Cancel\t\026\250\020/\026\001\020 Confirm";

enum
{
  BrowserTemplateOpenTop  = 0,
  BrowserTemplateEnterTop = 1,
  BrowserTemplateOpen     = 2,
  BrowserTemplateEnter    = 3,
};

#define BROWSER_TEMPLATE_COUNT 4

struct UiPos
{
  int Index;
  int Offset;
  const PspMenuItem *Top;
};

/* TODO: dynamically allocate ?? */
//TODO static unsigned int __attribute__((aligned(16))) call_list[524288];//262144];

/* Gets status string - containing current time and battery information */
static void GetStatusString(char *status, int length)
{
  static char main_str[128], batt_str[32];
  SceRtcTime time;

  /* Get current time */
  sceRtcGetCurrentClockLocalTime(&time);

  /* Get the battery/power-related information */
  /*if (!scePowerIsBatteryExist()) sprintf(batt_str, PSP_CHAR_POWER);
  else
  {*/
    /* If the battery's online, display charging stats */
    int batt_time = scePowerGetBatteryLifeTime();
    int batt_percent = scePowerGetBatteryLifePercent();
    int i, charging = scePowerIsBatteryCharging();

    static int percentiles[] = { 60, 30, 12, 0 };
    for (i = 0; i < 4; i++)
      if (batt_percent >= percentiles[i])
        break;

    /* Fix for when battery switches state from AC to batt */
    batt_time = (batt_time >= 0) ? batt_time : 0;

    sprintf(batt_str, "%c%3i%% (%02i:%02i)",
      (charging) ? *PSP_CHAR_POWER : *PSP_CHAR_FULL_BATT + i,
      batt_percent, batt_time / 60, batt_time % 60);
  /*}*/

  /* Write the rest of the string */
  sprintf(main_str, "\270%2i/%2i %02i%c%02i %s ",
    time.month, time.day, time.hour, (time.microseconds > 500000) ? ':' : ' ',
    time.minutes, batt_str);

  strncpy(status, main_str, length);
  status[length - 1] = '\0';
}

static inline void RenderStatus()
{
  static char status[128];
  GetStatusString(status, sizeof(status));

  int width = pspFontGetTextWidth(UiMetric.Font, status);
  pspVideoPrint(UiMetric.Font, SCR_WIDTH - width, 0, status, PSP_COLOR_WHITE);
}

static void ReplaceIcons(char *string)
{
  char *ch;

  for (ch = string; *ch; ch++)
  {
    switch(*ch)
    {
    case '\001': *ch = pspUiGetButtonIcon(UiMetric.OkButton); break;
    case '\002': *ch = pspUiGetButtonIcon(UiMetric.CancelButton); break;
    }
  }
}

char pspUiGetButtonIcon(uint32_t button_mask)
{
  switch (button_mask)
  {
  case PSP2_CTRL_CROSS:    return *PSP_CHAR_CROSS;
  case PSP2_CTRL_CIRCLE:   return *PSP_CHAR_CIRCLE;
  case PSP2_CTRL_TRIANGLE: return *PSP_CHAR_TRIANGLE;
  case PSP2_CTRL_SQUARE:   return *PSP_CHAR_SQUARE;
  default:                return '?';
  }
}

void pspUiAlert(const char *message)
{
  PspImage *screen = NULL;
  int sx, sy, dx, dy, th, fh, mw, cw, w, h;
  int i, n = UI_ANIM_FRAMES;
  char *instr = strdup(AlertDialogButtonTemplate);
  ReplaceIcons(instr);

  mw = pspFontGetTextWidth(UiMetric.Font, message);
  cw = pspFontGetTextWidth(UiMetric.Font, instr);
  fh = pspFontGetLineHeight(UiMetric.Font);
  th = pspFontGetTextHeight(UiMetric.Font, message);

  w = ((mw > cw) ? mw : cw) + 50;
  h = th + fh * 3;
  sx = SCR_WIDTH / 2 - w / 2;
  sy = SCR_HEIGHT / 2 - h / 2;
  dx = sx + w;
  dy = sy + h;

  /* Intro animation */
  if (UiMetric.Animate)
  {
    /* Get copy of screen */
    screen = pspVideoGetVramBufferCopy();

    for (i = 0; i < n; i++)
    {
  	  pspVideoBegin();

  	  /* Clear screen */
  	  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

  	  /* Apply fog and draw frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0,0,0,UI_ANIM_FOG_STEP*i));
  	  pspVideoFillRect(SCR_WIDTH/2-(((dx-sx)/n)*i)/2,
  	    SCR_HEIGHT/2-(((dy-sy)/n)*i)/2,
  	    SCR_WIDTH/2+(((dx-sx)/n)*i)/2, SCR_HEIGHT/2+(((dy-sy)/n)*i)/2,
  	    COLOR(RED_32(UiMetric.MenuOptionBoxBg),
  	      GREEN_32(UiMetric.MenuOptionBoxBg),
  	      BLUE_32(UiMetric.MenuOptionBoxBg),(0xff/n)*i));

  	  pspVideoEnd();

      /* Swap buffers */
      pspVideoWaitVSync();
      pspVideoSwapBuffers();
  	}
 }

  pspVideoBegin();

  if (UiMetric.Animate)
    pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
    COLOR(0,0,0,UI_ANIM_FOG_STEP*n));
  pspVideoFillRect(sx, sy, dx, dy, UiMetric.MenuOptionBoxBg);
  pspVideoPrint(UiMetric.Font, SCR_WIDTH / 2 - mw / 2, sy + fh * 0.5, message,
    UiMetric.TextColor);
  pspVideoPrint(UiMetric.Font, SCR_WIDTH / 2 - cw / 2, dy - fh * 1.5, instr,
    UiMetric.TextColor);
  pspVideoGlowRect(sx, sy, dx - 1, dy - 1,
    COLOR(0xff,0xff,0xff,UI_ANIM_FOG_STEP*n), 2);

  pspVideoEnd();

  /* Swap buffers */
  pspVideoWaitVSync();
  pspVideoSwapBuffers();

  SceCtrlData pad;

  /* Loop until X or O is pressed */
  while (!ExitPSP)
  {
    if (!pspCtrlPollControls(&pad))
      continue;

    if (pad.buttons & UiMetric.OkButton || pad.buttons & UiMetric.CancelButton)
      break;
  }

  if (!ExitPSP && UiMetric.Animate)
  {
	  /* Exit animation */
	  for (i = n - 1; i >= 0; i--)
	  {
		  pspVideoBegin();

		  /* Clear screen */
		  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

		  /* Apply fog and draw frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0,0,0,UI_ANIM_FOG_STEP*i));
  	  pspVideoFillRect(SCR_WIDTH/2-(((dx-sx)/n)*i)/2,
  	    SCR_HEIGHT/2-(((dy-sy)/n)*i)/2,
  	    SCR_WIDTH/2+(((dx-sx)/n)*i)/2, SCR_HEIGHT/2+(((dy-sy)/n)*i)/2,
  	    COLOR(RED_32(UiMetric.MenuOptionBoxBg),
  	      GREEN_32(UiMetric.MenuOptionBoxBg),
  	      BLUE_32(UiMetric.MenuOptionBoxBg),(0xff/n)*i));

		  pspVideoEnd();

	    /* Swap buffers */
	    pspVideoWaitVSync();
	    pspVideoSwapBuffers();
		}
	}

  if (screen) pspImageDestroy(screen);
  free(instr);
}

int pspUiYesNoCancel(const char *message)
{
  PspImage *screen = NULL;
  int sx, sy, dx, dy, th, fh, mw, cw, w, h;
  int i, n = UI_ANIM_FRAMES;
  char *instr = strdup(YesNoCancelDialogButtonTemplate);
  ReplaceIcons(instr);

  mw = pspFontGetTextWidth(UiMetric.Font, message);
  cw = pspFontGetTextWidth(UiMetric.Font, instr);
  fh = pspFontGetLineHeight(UiMetric.Font);
  th = pspFontGetTextHeight(UiMetric.Font, message);

  w = ((mw > cw) ? mw : cw) + 50;
  h = th + fh * 3;
  sx = SCR_WIDTH / 2 - w / 2;
  sy = SCR_HEIGHT / 2 - h / 2;
  dx = sx + w;
  dy = sy + h;

  /* Intro animation */
  if (UiMetric.Animate)
  {
    /* Get copy of screen */
    screen = pspVideoGetVramBufferCopy();

    for (i = 0; i < n; i++)
    {
  	  pspVideoBegin();

  	  /* Clear screen */
  	  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

  	  /* Apply fog and draw frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0,0,0,UI_ANIM_FOG_STEP*i));
  	  pspVideoFillRect(SCR_WIDTH/2-(((dx-sx)/n)*i)/2,
  	    SCR_HEIGHT/2-(((dy-sy)/n)*i)/2,
  	    SCR_WIDTH/2+(((dx-sx)/n)*i)/2, SCR_HEIGHT/2+(((dy-sy)/n)*i)/2,
  	    COLOR(RED_32(UiMetric.MenuOptionBoxBg),
  	      GREEN_32(UiMetric.MenuOptionBoxBg),
  	      BLUE_32(UiMetric.MenuOptionBoxBg),(0xff/n)*i));

  	  pspVideoEnd();

      /* Swap buffers */
      pspVideoWaitVSync();
      pspVideoSwapBuffers();
  	}
  }

  pspVideoBegin();

  if (UiMetric.Animate)
    pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);
  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
    COLOR(0,0,0,UI_ANIM_FOG_STEP*n));
  pspVideoFillRect(sx, sy, dx, dy, UiMetric.MenuOptionBoxBg);
  pspVideoPrint(UiMetric.Font, SCR_WIDTH / 2 - mw / 2, sy + fh * 0.5, message,
    UiMetric.TextColor);
  pspVideoPrint(UiMetric.Font, SCR_WIDTH / 2 - cw / 2, dy - fh * 1.5, instr,
    UiMetric.TextColor);
  pspVideoGlowRect(sx, sy, dx - 1, dy - 1,
    COLOR(0xff,0xff,0xff,UI_ANIM_FOG_STEP*n), 2);

  pspVideoEnd();

  /* Swap buffers */
  pspVideoWaitVSync();
  pspVideoSwapBuffers();

  SceCtrlData pad;

  /* Loop until X or O is pressed */
  while (!ExitPSP)
  {
    if (!pspCtrlPollControls(&pad))
      continue;

    if (pad.buttons & UiMetric.OkButton || pad.buttons & UiMetric.CancelButton
      || pad.buttons & PSP2_CTRL_SQUARE) break;
  }

  if (!ExitPSP && UiMetric.Animate)
  {
	  /* Exit animation */
	  for (i = n - 1; i >= 0; i--)
	  {
		  pspVideoBegin();

		  /* Clear screen */
		  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

		  /* Apply fog and draw frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0,0,0,UI_ANIM_FOG_STEP*i));
  	  pspVideoFillRect(SCR_WIDTH/2-(((dx-sx)/n)*i)/2,
  	    SCR_HEIGHT/2-(((dy-sy)/n)*i)/2,
  	    SCR_WIDTH/2+(((dx-sx)/n)*i)/2, SCR_HEIGHT/2+(((dy-sy)/n)*i)/2,
  	    COLOR(RED_32(UiMetric.MenuOptionBoxBg),
  	      GREEN_32(UiMetric.MenuOptionBoxBg),
  	      BLUE_32(UiMetric.MenuOptionBoxBg),(0xff/n)*i));

		  pspVideoEnd();

	    /* Swap buffers */
	    pspVideoWaitVSync();
	    pspVideoSwapBuffers();
		}
	}

  if (screen) pspImageDestroy(screen);
  free(instr);

  if (pad.buttons & UiMetric.CancelButton) return PSP_UI_CANCEL;
  else if (pad.buttons & PSP2_CTRL_SQUARE) return PSP_UI_NO;
  else return PSP_UI_YES;
}

int pspUiConfirm(const char *message)
{
  PspImage *screen = NULL;
  int sx, sy, dx, dy, th, fh, mw, cw, w, h;
  int i, n = UI_ANIM_FRAMES;
  char *instr = strdup(ConfirmDialogButtonTemplate);
  ReplaceIcons(instr);

  mw = pspFontGetTextWidth(UiMetric.Font, message);
  cw = pspFontGetTextWidth(UiMetric.Font, instr);
  fh = pspFontGetLineHeight(UiMetric.Font);
  th = pspFontGetTextHeight(UiMetric.Font, message);

  w = ((mw > cw) ? mw : cw) + 50;
  h = th + fh * 3;
  sx = SCR_WIDTH / 2 - w / 2;
  sy = SCR_HEIGHT / 2 - h / 2;
  dx = sx + w;
  dy = sy + h;

  if (UiMetric.Animate)
  {
    /* Get copy of screen */
    screen = pspVideoGetVramBufferCopy();

    /* Intro animation */
    for (i = 0; i < n; i++)
    {
  	  pspVideoBegin();

  	  /* Clear screen */
  	  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

  	  /* Apply fog and draw frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0,0,0,UI_ANIM_FOG_STEP*i));
  	  pspVideoFillRect(SCR_WIDTH/2-(((dx-sx)/n)*i)/2,
  	    SCR_HEIGHT/2-(((dy-sy)/n)*i)/2,
  	    SCR_WIDTH/2+(((dx-sx)/n)*i)/2, SCR_HEIGHT/2+(((dy-sy)/n)*i)/2,
  	    COLOR(RED_32(UiMetric.MenuOptionBoxBg),
  	      GREEN_32(UiMetric.MenuOptionBoxBg),
  	      BLUE_32(UiMetric.MenuOptionBoxBg),(0xff/n)*i));

  	  pspVideoEnd();

      /* Swap buffers */
      pspVideoWaitVSync();
      pspVideoSwapBuffers();
  	}
  }

  pspVideoBegin();

  if (UiMetric.Animate)
    pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);
  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
    COLOR(0,0,0,UI_ANIM_FOG_STEP*n));
  pspVideoFillRect(sx, sy, dx, dy, UiMetric.MenuOptionBoxBg);
  pspVideoPrint(UiMetric.Font, SCR_WIDTH / 2 - mw / 2, sy + fh * 0.5, message,
    UiMetric.TextColor);
  pspVideoPrint(UiMetric.Font, SCR_WIDTH / 2 - cw / 2, dy - fh * 1.5, instr,
    UiMetric.TextColor);
  pspVideoGlowRect(sx, sy, dx - 1, dy - 1,
    COLOR(0xff,0xff,0xff,UI_ANIM_FOG_STEP*n), 2);

  pspVideoEnd();

  /* Swap buffers */
  pspVideoWaitVSync();
  pspVideoSwapBuffers();

  SceCtrlData pad;

  /* Loop until X or O is pressed */
  while (!ExitPSP)
  {
    if (!pspCtrlPollControls(&pad))
      continue;

    if (pad.buttons & UiMetric.OkButton || pad.buttons & UiMetric.CancelButton)
      break;
  }

  if (!ExitPSP && UiMetric.Animate)
  {
	  /* Exit animation */
	  for (i = n - 1; i >= 0; i--)
	  {
		  pspVideoBegin();

		  /* Clear screen */
		  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

		  /* Apply fog and draw frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0,0,0,UI_ANIM_FOG_STEP*i));
  	  pspVideoFillRect(SCR_WIDTH/2-(((dx-sx)/n)*i)/2,
  	    SCR_HEIGHT/2-(((dy-sy)/n)*i)/2,
  	    SCR_WIDTH/2+(((dx-sx)/n)*i)/2, SCR_HEIGHT/2+(((dy-sy)/n)*i)/2,
  	    COLOR(RED_32(UiMetric.MenuOptionBoxBg),
  	      GREEN_32(UiMetric.MenuOptionBoxBg),
  	      BLUE_32(UiMetric.MenuOptionBoxBg),(0xff/n)*i));

		  pspVideoEnd();

	    /* Swap buffers */
	    pspVideoWaitVSync();
	    pspVideoSwapBuffers();
		}
	}

  if (screen) pspImageDestroy(screen);
  free(instr);

  return pad.buttons & UiMetric.OkButton;
}

void pspUiFlashMessage(const char *message)
{
  PspImage *screen = NULL;
  int sx, sy, dx, dy, fh, mw, mh, w, h;
  int i, n = UI_ANIM_FRAMES;

  mw = pspFontGetTextWidth(UiMetric.Font, message);
  fh = pspFontGetLineHeight(UiMetric.Font);
  mh = pspFontGetTextHeight(UiMetric.Font, message);

  w = mw + 50;
  h = mh + fh * 2;
  sx = SCR_WIDTH / 2 - w / 2;
  sy = SCR_HEIGHT / 2 - h / 2;
  dx = sx + w;
  dy = sy + h;

  if (UiMetric.Animate)
  {
    /* Get copy of screen */
    screen = pspVideoGetVramBufferCopy();

    /* Intro animation */
    for (i = 0; i < n; i++)
    {
  	  pspVideoBegin();

  	  /* Clear screen */
  	  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

  	  /* Apply fog and draw frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0,0,0,UI_ANIM_FOG_STEP*i));
  	  pspVideoFillRect(SCR_WIDTH/2-(((dx-sx)/n)*i)/2,
  	    SCR_HEIGHT/2-(((dy-sy)/n)*i)/2,
  	    SCR_WIDTH/2+(((dx-sx)/n)*i)/2, SCR_HEIGHT/2+(((dy-sy)/n)*i)/2,
  	    COLOR(RED_32(UiMetric.MenuOptionBoxBg),
  	      GREEN_32(UiMetric.MenuOptionBoxBg),
  	      BLUE_32(UiMetric.MenuOptionBoxBg),(0xff/n)*i));

  	  pspVideoEnd();

      /* Swap buffers */
      pspVideoWaitVSync();
      pspVideoSwapBuffers();
  	}
  }

  pspVideoBegin();

  if (UiMetric.Animate)
    pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);
  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
    COLOR(0,0,0,UI_ANIM_FOG_STEP*n));
  pspVideoFillRect(sx, sy, dx, dy, UiMetric.MenuOptionBoxBg);
  pspVideoPrintCenter(UiMetric.Font,
    sx, sy + fh, dx, message, UiMetric.TextColor);
  pspVideoGlowRect(sx, sy, dx - 1, dy - 1,
    COLOR(0xff,0xff,0xff,UI_ANIM_FOG_STEP*n), 2);

  pspVideoEnd();

  /* Swap buffers */
  pspVideoWaitVSync();
  pspVideoSwapBuffers();

  if (screen) pspImageDestroy(screen);
}

void pspUiOpenBrowser(PspUiFileBrowser *browser, const char *start_path)
{
  PspMenu *menu;
  PspFile *file;
  PspFileList *list;
  const PspMenuItem *sel, *last_sel;
  PspMenuItem *item;
  SceCtrlData pad;
  char *instructions[BROWSER_TEMPLATE_COUNT];

  /* Initialize instruction strings */
  int i;
  for (i = 0; i < BROWSER_TEMPLATE_COUNT; i++)
  {
    instructions[i] = strdup(BrowserTemplates[i]);
    ReplaceIcons(instructions[i]);
  }

  if (!start_path)
    start_path = pspGetAppDirectory();

  char *cur_path = pspFileIoGetParentDirectory(start_path);
  const char *cur_file = pspFileIoGetFilename(start_path);
  struct UiPos pos;
  int lnmax, lnhalf;
  int sby, sbh, j, h, w, fh = pspFontGetLineHeight(UiMetric.Font);
  int sx, sy, dx, dy;
  int hasparent, is_dir;

  sx = UiMetric.Left;
  sy = UiMetric.Top + fh + UiMetric.TitlePadding;
  dx = UiMetric.Right;
  dy = UiMetric.Bottom;
  w = dx - sx - UiMetric.ScrollbarWidth;
  h = dy - sy;

  menu = pspMenuCreate();

  //TODO memset(call_list, 0, sizeof(call_list));

  int sel_top = 0, last_sel_top = 0, fast_scroll;

  /* Begin browsing (outer) loop */
  while (!ExitPSP)
  {
    sel = last_sel = NULL;
    pos.Top = NULL;
    pspMenuClear(menu);

    /* Load list of files for the selected path */
    if ((list = pspFileIoGetFileList(cur_path, browser->Filter)))
    {
      /* Check for a parent path, prepend .. if necessary */
      if ((hasparent =! pspFileIoIsRootDirectory(cur_path)))
      {
        item = pspMenuAppendItem(menu, "..", 0);
        item->Param = (void*)PSP_FILEIO_DIR;
      }

      /* Add a menu item for each file */
      for (file = list->First; file; file = file->Next)
      {
        /* Skip files that begin with '.' */
        if (file->Name && file->Name[0] == '.')
          continue;

        item = pspMenuAppendItem(menu, file->Name, 0);
        item->Param = (void*)file->Attrs;

        if (cur_file && strcmp(file->Name, cur_file) == 0)
          sel = item;
      }

      cur_file = NULL;

      /* Destroy the file list */
      pspFileIoDestroyFileList(list);
    }
    else
    {
      /* Check for a parent path, prepend .. if necessary */
      if ((hasparent =! pspFileIoIsRootDirectory(cur_path)))
      {
        item = pspMenuAppendItem(menu, "..", 0);
        item->Param = (void*)PSP_FILEIO_DIR;
      }
    }

    /* Initialize variables */
    lnmax = (dy - sy) / fh;
    lnhalf = lnmax >> 1;
    sbh = (menu->Count > lnmax) ? (int)((float)h * ((float)lnmax / (float)menu->Count)) : 0;

    pos.Index = pos.Offset = 0;

    if (!sel)
    {
      /* Select the first file/dir in the directory */
      if (menu->First && menu->First->Next)
        sel=menu->First->Next;
      else if (menu->First)
        sel=menu->First;
    }

    /* Compute index and offset of selected file */
    if (sel)
    {
      pos.Top = menu->First;
      for (item = menu->First; item != sel; item = item->Next)
      {
        if (pos.Index + 1 >= lnmax) { pos.Offset++; pos.Top=pos.Top->Next; }
        else pos.Index++;
      }
    }

    pspVideoWaitVSync();

    /* Begin navigation (inner) loop */
    while (!ExitPSP)
    {
      if (!pspCtrlPollControls(&pad))
        continue;

      fast_scroll = 0;

      /* Check the directional buttons */
      if (sel)
      {
        if ((pad.buttons & PSP2_CTRL_DOWN || pad.buttons & PSP_CTRL_ANALDOWN) && sel->Next)
        {
          if (pos.Index+1 >= lnmax) { pos.Offset++; pos.Top=pos.Top->Next; }
          else pos.Index++;
          sel=sel->Next;
          fast_scroll = pad.buttons & PSP_CTRL_ANALDOWN;
        }
        else if ((pad.buttons & PSP2_CTRL_UP || pad.buttons & PSP_CTRL_ANALUP) && sel->Prev)
        {
          if (pos.Index - 1 < 0) { pos.Offset--; pos.Top=pos.Top->Prev; }
          else pos.Index--;
          sel = sel->Prev;
          fast_scroll = pad.buttons & PSP_CTRL_ANALUP;
        }
        else if (pad.buttons & PSP2_CTRL_LEFT)
        {
          for (i=0; sel->Prev && i < lnhalf; i++)
          {
            if (pos.Index-1 < 0) { pos.Offset--; pos.Top=pos.Top->Prev; }
            else pos.Index--;
            sel=sel->Prev;
          }
        }
        else if (pad.buttons & PSP2_CTRL_RIGHT)
        {
          for (i=0; sel->Next && i < lnhalf; i++)
          {
            if (pos.Index + 1 >= lnmax) { pos.Offset++; pos.Top=pos.Top->Next; }
            else pos.Index++;
            sel=sel->Next;
          }
        }

        /* File/dir selection */
        if (pad.buttons & UiMetric.OkButton)
        {
          if (((unsigned int)sel->Param & PSP_FILEIO_DIR))
          {
            /* Selected a directory, descend */
            pspFileIoEnterDirectory(&cur_path, sel->Caption);
            break;
          }
          else
          {
            int exit = 1;

            /* Selected a file */
            if (browser->OnOk)
            {
              char *file = malloc((strlen(cur_path) + strlen(sel->Caption) + 1) * sizeof(char));
              sprintf(file, "%s%s", cur_path, sel->Caption);
              exit = browser->OnOk(browser, file);
              free(file);
            }

            if (exit) goto exit_browser;
            else continue;
          }
        }
      }

      if (pad.buttons & PSP2_CTRL_TRIANGLE)
      {
        if (!pspFileIoIsRootDirectory(cur_path))
        {
          pspFileIoEnterDirectory(&cur_path, "..");
          break;
        }
      }
      else if (pad.buttons & UiMetric.CancelButton)
      {
        if (browser->OnCancel)
          browser->OnCancel(browser, cur_path);
        goto exit_browser;
      }
      else if ((pad.buttons & CONTROL_BUTTON_MASK) && browser->OnButtonPress)
      {
        char *file = NULL;
        int exit;

        if (sel)
        {
          file = malloc((strlen(cur_path) + strlen(sel->Caption) + 1) * sizeof(char));
          sprintf(file, "%s%s", cur_path, sel->Caption);
        }

        exit = browser->OnButtonPress(browser,
          file, pad.buttons & CONTROL_BUTTON_MASK);

        if (file) free(file);
        if (exit) goto exit_browser;
      }

      is_dir = (unsigned int)sel->Param & PSP_FILEIO_DIR;

      //sceGuStart(GU_CALL, call_list);
      pspVideoBegin();


      /* Draw current path */
      pspVideoPrint(UiMetric.Font, sx, UiMetric.Top, cur_path,
        UiMetric.TitleColor);
      pspVideoDrawLine(UiMetric.Left, UiMetric.Top + fh - 1, UiMetric.Left + w,
        UiMetric.Top + fh - 1, UiMetric.TitleColor);

      const char *instruction;
      if (hasparent)
        instruction = instructions[(is_dir)
          ? BrowserTemplateEnter : BrowserTemplateOpen];
      else
        instruction = instructions[(is_dir)
          ? BrowserTemplateEnterTop : BrowserTemplateOpenTop];

      pspVideoPrintCenter(UiMetric.Font,
        sx, SCR_HEIGHT - fh, dx, instruction, UiMetric.StatusBarColor);

      /* Draw scrollbar */
      if (sbh > 0)
      {
        sby = sy + (int)((float)(h - sbh)
          * ((float)(pos.Offset + pos.Index) / (float)menu->Count));
        pspVideoFillRect(dx - UiMetric.ScrollbarWidth, sy, dx, dy,
          UiMetric.ScrollbarBgColor);
        pspVideoFillRect(dx - UiMetric.ScrollbarWidth, sby, dx, sby + sbh,
          UiMetric.ScrollbarColor);
      }

      /* Render the files */
      for (item = (PspMenuItem*)pos.Top, i = 0, j = sy;
        item && i < lnmax; item = item->Next, j += fh, i++)
      {
        if (item == sel) sel_top = j;

        pspVideoPrintClipped(UiMetric.Font, sx + 10, j, item->Caption, w - 10,
          "...", (item == sel) ? UiMetric.SelectedColor
            : ((unsigned int)item->Param & PSP_FILEIO_DIR)
            ? UiMetric.BrowserDirectoryColor : UiMetric.BrowserFileColor);
     }

      /* Render status information */
      RenderStatus();

      /* Perform any custom drawing */
      if (browser->OnRender)
        browser->OnRender(browser, "not implemented");

      pspVideoEnd();

      if (sel != last_sel && !fast_scroll && sel && last_sel
        && UiMetric.Animate)
      {
        /* Move animation */
        int f, n = 4;
        for (f = 1; f <= n; f++)
        {
          pspVideoBegin();

          /* Clear screen */
          if (!UiMetric.Background) pspVideoClearScreen();
          else pspVideoPutImage(UiMetric.Background, 0, 0,
            UiMetric.Background->Viewport.Width, UiMetric.Background->Height);

          /* Selection box */
          int box_top = last_sel_top-((last_sel_top-sel_top)/n)*f;
          pspVideoFillRect(sx, box_top, sx+w, box_top+fh,
            UiMetric.SelectedBgColor);

          //TODO sceGuCallList(call_list);

          pspVideoEnd();

          pspVideoWaitVSync();
          pspVideoSwapBuffers();
        }
      }

      pspVideoBegin();

      /* Clear screen */
      if (UiMetric.Background)
        pspVideoPutImage(UiMetric.Background, 0, 0,
          UiMetric.Background->Viewport.Width, UiMetric.Background->Height);
      else pspVideoClearScreen();

      /* Render selection box */
      if (sel) pspVideoFillRect(sx, sel_top, sx+w, sel_top+fh,
        UiMetric.SelectedBgColor);

      //TODO sceGuCallList(call_list);

      pspVideoEnd();

      /* Swap buffers */
      pspVideoWaitVSync();
      pspVideoSwapBuffers();

      last_sel = sel;
      last_sel_top = sel_top;
    }
  }

exit_browser:

  /* Free instruction strings */
  for (i = 0; i < BROWSER_TEMPLATE_COUNT; i++)
    free(instructions[i]);

  pspMenuDestroy(menu);
  free(cur_path);
}

void pspUiOpenGallery(const PspUiGallery *gallery, const char *title)
{
  PspMenu *menu = gallery->Menu;
  const PspMenuItem *top, *item;
  SceCtrlData pad;
  PspMenuItem *sel = menu->Selected;

  int sx, sy, dx, dy,
    orig_w = 272, orig_h = 228, // defaults
    fh, c, i, j,
    sbh, sby,
    w, h,
    icon_w, icon_h,
    grid_w, grid_h,
    icon_idx, icon_off,
    rows, vis_v, vis_s,
    icons;
  const PspMenuItem *last_sel = NULL;

  /* Find first icon and save its width/height */
  for (item = menu->First; item; item = item->Next)
  {
    if (item->Icon)
    {
      orig_w = ((PspImage*)item->Icon)->Viewport.Width;
      orig_h = ((PspImage*)item->Icon)->Height;
      break;
    }
  }

  fh = pspFontGetLineHeight(UiMetric.Font);
  sx = UiMetric.Left;
  sy = UiMetric.Top + ((title) ? fh + UiMetric.TitlePadding : 0);
  dx = UiMetric.Right;
  dy = UiMetric.Bottom;
  w = (dx - sx) - UiMetric.ScrollbarWidth; // visible width
  h = dy - sy; // visible height
  icon_w = (w - UiMetric.GalleryIconMarginWidth
    * (UiMetric.GalleryIconsPerRow - 1)) / UiMetric.GalleryIconsPerRow; // icon width
  icon_h = (int)((float)icon_w
    / ((float)orig_w / (float)orig_h)); // icon height
  grid_w = icon_w + UiMetric.GalleryIconMarginWidth; // width of the grid
  grid_h = icon_h + (fh * 2); // half-space for margin + 1 line of text
  icons = menu->Count; // number of icons total
  rows = ceil((float)icons / (float)UiMetric.GalleryIconsPerRow); // number of rows total
  vis_v = h / grid_h; // number of rows visible at any time
  vis_s = UiMetric.GalleryIconsPerRow * vis_v; // max. number of icons visible on screen at any time
  int max_w = ((float)icon_w * 1.5); /* Maximized width  */
  int max_h = ((float)icon_h * 1.5); /* Maximized height */

  icon_idx = 0;
  icon_off = 0;
  top = menu->First;

  if (!sel)
  {
    /* Select the first icon */
    sel = menu->First;
  }
  else
  {
    /* Find the selected icon */
    for (item = menu->First; item; item = item->Next)
    {
      if (item == sel)
        break;

      if (++icon_idx >= vis_s)
      {
        icon_idx=0;
        icon_off += vis_s;
        top = item;
      }
    }

    if (item != sel)
    {
      /* Icon not found; reset to first icon */
      sel = menu->First;
      top = menu->First;
      icon_idx = 0;
      icon_off = 0;
    }
  }

  /* Compute height of scrollbar */
  sbh = ((float)vis_v / (float)(rows + (rows % vis_v))) * (float)h;

  /* Compute update frequency */
  uint32_t ticks_per_sec, ticks_per_upd;
  uint64_t current_tick, last_tick;

  ticks_per_sec = sceRtcGetTickResolution();
  sceRtcGetCurrentTick(&last_tick);
  ticks_per_upd = ticks_per_sec / UiMetric.MenuFps;

  //TODO memset(call_list, 0, sizeof(call_list));
  int sel_left = 0, max_left = 0;
  int sel_top = 0, max_top = 0;

  pspVideoWaitVSync();

  /* Begin navigation loop */
  while (!ExitPSP)
  {
    if (!pspCtrlPollControls(&pad))
      continue;

    /* Check the directional buttons */
    if (sel)
    {
      if (pad.buttons & PSP2_CTRL_RIGHT && sel->Next)
      {
        sel = sel->Next;
        if (++icon_idx >= vis_s)
        {
          icon_idx = 0;
          icon_off += vis_s;
          top = sel;
        }
      }
      else if (pad.buttons & PSP2_CTRL_LEFT && sel->Prev)
      {
        sel = sel->Prev;
        if (--icon_idx < 0)
        {
          icon_idx = vis_s-1;
          icon_off -= vis_s;
          for (i = 0; i < vis_s && top; i++) top = top->Prev;
        }
      }
      else if (pad.buttons & PSP2_CTRL_DOWN)
      {
        for (i = 0; sel->Next && i < UiMetric.GalleryIconsPerRow; i++)
        {
          sel = sel->Next;
          if (++icon_idx >= vis_s)
          {
            icon_idx = 0;
            icon_off += vis_s;
            top = sel;
          }
        }
      }
      else if (pad.buttons & PSP2_CTRL_UP)
      {
        for (i = 0; sel->Prev && i < UiMetric.GalleryIconsPerRow; i++)
        {
          sel = sel->Prev;
          if (--icon_idx < 0)
          {
            icon_idx = vis_s-1;
            icon_off -= vis_s;
            for (j = 0; j < vis_s && top; j++) top = top->Prev;
          }
        }
      }

      if (pad.buttons & UiMetric.OkButton)
      {
        pad.buttons &= ~UiMetric.OkButton;
        if (!gallery->OnOk || gallery->OnOk(gallery, sel))
          break;
      }
    }

    if (pad.buttons & UiMetric.CancelButton)
    {
      pad.buttons &= ~UiMetric.CancelButton;
      if (gallery->OnCancel)
        gallery->OnCancel(gallery, sel);
      break;
    }

    if ((pad.buttons & CONTROL_BUTTON_MASK) && gallery->OnButtonPress)
      if (gallery->OnButtonPress(gallery, sel, pad.buttons & CONTROL_BUTTON_MASK))
          break;

    if (last_sel != sel && last_sel && sel && sel->Icon && UiMetric.Animate)
    {
      /* "Implode" animation */
      int f = 1, n = 2;
//      for (f = n - 1; f > 0; f--)
//      {
        pspVideoBegin();

        /* Clear screen */
        if (!UiMetric.Background) pspVideoClearScreen();
        else pspVideoPutImage(UiMetric.Background, 0, 0,
          UiMetric.Background->Viewport.Width, UiMetric.Background->Height);

        //TODO sceGuCallList(call_list);

        pspVideoEnd();

        /* Render the menu items */
        for (i = sy, item = top; item && i + grid_h < dy; i += grid_h)
          for (j = sx, c = 0; item && c < UiMetric.GalleryIconsPerRow; j += grid_w, c++, item = item->Next)
            if (item->Icon && item != last_sel)
            {
              pspVideoBegin();
              pspVideoPutImage((PspImage*)item->Icon, j, i, icon_w, icon_h);
              pspVideoEnd();
            }

        pspVideoBegin();

        pspVideoPutImage((PspImage*)last_sel->Icon,
          sel_left-(icon_w+((max_w-icon_w)/n)*f)/2,
          sel_top-(icon_h+((max_h-icon_h)/n)*f)/2,
          icon_w+((max_w-icon_w)/n)*f,
          icon_h+((max_h-icon_h)/n)*f);

        pspVideoEnd();

        /* Swap buffers */
        pspVideoWaitVSync();
        pspVideoSwapBuffers();
//      }
    }

    //sceGuStart(GU_CALL, call_list);
    pspVideoBegin();

    /* Draw title */
    if (title)
    {
      pspVideoPrint(UiMetric.Font, UiMetric.Left, UiMetric.Top,
        title, UiMetric.TitleColor);
      pspVideoDrawLine(UiMetric.Left, UiMetric.Top + fh - 1, UiMetric.Left + w,
        UiMetric.Top + fh - 1, UiMetric.TitleColor);
    }

    /* Draw scrollbar */
    if (sbh < h)
    {
      sby = sy + (((float)icon_off / (float)UiMetric.GalleryIconsPerRow)
        / (float)(rows + (rows % vis_v))) * (float)h;
      pspVideoFillRect(dx - UiMetric.ScrollbarWidth,
        sy, dx, dy, UiMetric.ScrollbarBgColor);
      pspVideoFillRect(dx - UiMetric.ScrollbarWidth,
        sby, dx, sby+sbh, UiMetric.ScrollbarColor);
    }

    /* Draw instructions */
    if (sel && sel->HelpText)
    {
      static char help_copy[MAX_DIR_LEN];
      strncpy(help_copy, sel->HelpText, MAX_DIR_LEN);
      help_copy[MAX_DIR_LEN - 1] = '\0';
      ReplaceIcons(help_copy);

      pspVideoPrintCenter(UiMetric.Font,
        0, SCR_HEIGHT - fh, SCR_WIDTH, help_copy, UiMetric.StatusBarColor);
    }

    /* Render non-image components of each item */
    for (i = sy, item = top; item && i + grid_h < dy; i += grid_h)
    {
      for (j = sx, c = 0; item && c < UiMetric.GalleryIconsPerRow; j += grid_w, c++, item = item->Next)
      {
        if (item != sel)
        {
          pspVideoShadowRect(j - 1, i - 1, j + icon_w, i + icon_h, PSP_COLOR_BLACK, 3);
          pspVideoDrawRect(j - 1, i - 1, j + icon_w, i + icon_h, UiMetric.TextColor);

          if (item->Caption)
          {
            int cap_pos = j + icon_w / 2
              - pspFontGetTextWidth(UiMetric.Font, item->Caption) / 2;
            pspVideoPrint(UiMetric.Font, cap_pos,
              i + icon_h + (fh / 2), item->Caption, UiMetric.TextColor);
          }
        }
        else
        {
          sel_left = j + icon_w / 2;
          sel_top = i + icon_h / 2;

          sel_left = (sel_left-max_w/2 < sx) ? sx+max_w/2 : sel_left;
          sel_top = (sel_top-max_h/2 < UiMetric.Top)
            ? UiMetric.Top+max_h/2 : sel_top;
          sel_left = (sel_left+max_w/2 > dx) ? dx-max_w/2 : sel_left;
          sel_top = (sel_top+max_h/2 > dy) ? dy-max_h/2 : sel_top;
        }
      }
    }

    /* Render status information */
    RenderStatus();

    /* Perform any custom drawing */
    if (gallery->OnRender)
      gallery->OnRender(gallery, sel);

    pspVideoEnd();

    if (last_sel != sel && last_sel && sel && sel->Icon && UiMetric.Animate)
    {
      /* Popup animation */
      int f = 1, n = 2;
//      for (f = 1; f < n; f++)
//      {
        pspVideoBegin();

        /* Clear screen */
        if (!UiMetric.Background) pspVideoClearScreen();
        else pspVideoPutImage(UiMetric.Background, 0, 0,
          UiMetric.Background->Viewport.Width, UiMetric.Background->Height);

        //TODO sceGuCallList(call_list);

        pspVideoEnd();

        /* Render the menu items */
        for (i = sy, item = top; item && i + grid_h < dy; i += grid_h)
          for (j = sx, c = 0; item && c < UiMetric.GalleryIconsPerRow; j += grid_w, c++, item = item->Next)
            if (item->Icon && item != sel)
            {
              pspVideoBegin();
              pspVideoPutImage((PspImage*)item->Icon, j, i, icon_w, icon_h);
              pspVideoEnd();
            }

        pspVideoBegin();

        pspVideoPutImage((PspImage*)sel->Icon,
          sel_left-(icon_w+((max_w-icon_w)/n)*f)/2,
          sel_top-(icon_h+((max_h-icon_h)/n)*f)/2,
          icon_w+((max_w-icon_w)/n)*f,
          icon_h+((max_h-icon_h)/n)*f);

        pspVideoEnd();

        /* Swap buffers */
        pspVideoWaitVSync();
        pspVideoSwapBuffers();
//      }
    }

    pspVideoBegin();

    /* Clear screen */
    if (!UiMetric.Background) pspVideoClearScreen();
    else pspVideoPutImage(UiMetric.Background, 0, 0,
      UiMetric.Background->Viewport.Width, UiMetric.Background->Height);

    //TODO sceGuCallList(call_list);

    pspVideoEnd();

    /* Render the menu items */
    for (i = sy, item = top; item && i + grid_h < dy; i += grid_h)
      for (j = sx, c = 0; item && c < UiMetric.GalleryIconsPerRow; j += grid_w, c++, item = item->Next)
        if (item->Icon && item != sel)
        {
          pspVideoBegin();
          pspVideoPutImage((PspImage*)item->Icon, j, i, icon_w, icon_h);
          pspVideoEnd();
        }

    pspVideoBegin();

    if (sel && sel->Icon)
    {
      pspVideoPutImage((PspImage*)sel->Icon, sel_left-max_w/2, sel_top-max_h/2,
        max_w, max_h);
      pspVideoGlowRect(sel_left-max_w/2, sel_top-max_h/2,
        sel_left+max_w/2 - 1, sel_top+max_h/2 - 1,
        COLOR(0xff,0xff,0xff,UI_ANIM_FOG_STEP * UI_ANIM_FRAMES), 2);
    }

    if (sel && sel->Caption)
    {
      int cap_left = sel_left
        - pspFontGetTextWidth(UiMetric.Font, sel->Caption) / 2;
      pspVideoPrint(UiMetric.Font, cap_left,
        sel_top + max_h/2 - (fh + (fh - UiMetric.Font->Ascent)), sel->Caption,
        UiMetric.TextColor);
    }

    pspVideoEnd();

    last_sel = sel;

    /* Wait if needed */
    do { sceRtcGetCurrentTick(&current_tick); }
    while (current_tick - last_tick < ticks_per_upd);
    last_tick = current_tick;

    /* Swap buffers */
    pspVideoWaitVSync();
    pspVideoSwapBuffers();
  }

  menu->Selected = sel;
}

void pspUiOpenMenu(const PspUiMenu *uimenu, const char *title)
{
  struct UiPos pos;
  PspMenu *menu = uimenu->Menu;
  const PspMenuItem *item;
  SceCtrlData pad;
  const PspMenuOption *temp_option;
  int lnmax;
  int sby, sbh, i, j, k, h, w, fh = pspFontGetLineHeight(UiMetric.Font);
  int sx, sy, dx, dy, sel_top = 0, last_sel_top = 0;
  int max_item_w = 0, item_w;
  int option_mode, max_option_w = 0;
  int arrow_w = pspFontGetTextWidth(UiMetric.Font, "\272");
  int anim_frame = 0, anim_incr = 1;
  PspMenuItem *sel = menu->Selected, *last_sel = NULL;

  sx = UiMetric.Left;
  sy = UiMetric.Top + ((title) ? (fh + UiMetric.TitlePadding) : 0);
  dx = UiMetric.Right;
  dy = UiMetric.Bottom;
  w = dx - sx - UiMetric.ScrollbarWidth;
  h = dy - sy;


  //TODO memset(call_list, 0, sizeof(call_list));

  /* Determine width of the longest caption */
  for (item = menu->First; item; item = item->Next)
  {
    if (item->Caption)
    {
      item_w = pspFontGetTextWidth(UiMetric.Font, item->Caption);
      if (item_w > max_item_w)
        max_item_w = item_w;
    }
  }

  /* Initialize variables */
  lnmax = (dy - sy) / fh;
  sbh = (menu->Count > lnmax) ? (int)((float)h * ((float)lnmax / (float)menu->Count)) : 0;

  pos.Index = 0;
  pos.Offset = 0;
  pos.Top = NULL;
  option_mode = 0;
  temp_option = NULL;

  int cur_x=0, min_x=0, max_x=0;
  int cur_y=0, min_y=0, max_y=0;

  /* Find first selectable item */
  if (!sel)
  {
    for (sel = menu->First; sel; sel = sel->Next)
      if (sel->Caption && sel->Caption[0] != '\t')
        break;
  }

  /* Compute index and offset of selected file */
  pos.Top = menu->First;
  for (item = menu->First; item != sel; item = item->Next)
  {
    if (pos.Index + 1 >= lnmax) { pos.Offset++; pos.Top = pos.Top->Next; }
    else pos.Index++;
  }

  pspVideoWaitVSync();
  PspMenuItem *last;
  struct UiPos last_valid;

  /* Compute update frequency */
  uint32_t ticks_per_sec, ticks_per_upd;
  uint64_t current_tick, last_tick;

  ticks_per_sec = sceRtcGetTickResolution();
  sceRtcGetCurrentTick(&last_tick);
  ticks_per_upd = ticks_per_sec / UiMetric.MenuFps;

  int fast_scroll;

  /* Begin navigation loop */
  while (!ExitPSP)
  {
    if (!pspCtrlPollControls(&pad))
      continue;

    fast_scroll = 0;
    anim_frame += (UiMetric.Animate) ? anim_incr : 0;
    if (anim_frame > 2 || anim_frame < 0)
      anim_incr *= -1;

    /* Check the directional buttons */
    if (sel)
    {
      if (pad.buttons & PSP2_CTRL_DOWN || pad.buttons & PSP_CTRL_ANALDOWN)
      {
        fast_scroll = pad.buttons & PSP_CTRL_ANALDOWN;

        if (option_mode)
        {
          if (temp_option->Next)
            temp_option = temp_option->Next;
        }
        else
        {
          if (sel->Next)
          {
            last = sel;
            last_valid = pos;

            for (;;)
            {
              if (pos.Index + 1 >= lnmax)
              {
                pos.Offset++;
                pos.Top = pos.Top->Next;
              }
              else pos.Index++;

              sel = sel->Next;

              if (!sel)
              {
                sel = last;
                pos = last_valid;
                break;
              }

              if (sel->Caption && sel->Caption[0] != '\t')
                break;
            }
          }
        }
      }
      else if (pad.buttons & PSP2_CTRL_UP || pad.buttons & PSP_CTRL_ANALUP)
      {
        fast_scroll = pad.buttons & PSP_CTRL_ANALUP;

        if (option_mode)
        {
          if (temp_option->Prev)
            temp_option = temp_option->Prev;
        }
        else
        {
          if (sel->Prev)
          {
            last = sel;
            last_valid = pos;

            for (;;)
            {
              if (pos.Index - 1 < 0)
              {
                pos.Offset--;
                pos.Top = pos.Top->Prev;
              }
              else pos.Index--;

              sel = sel->Prev;

              if (!sel)
              {
                sel = last;
                pos = last_valid;
                break;
              }

              if (sel->Caption && sel->Caption[0] != '\t')
                break;
            }
          }
        }
      }

      /* Recompute box bounds if scrolling in option mode */
      if (option_mode && (pad.buttons &
        (PSP2_CTRL_UP|PSP_CTRL_ANALUP|PSP2_CTRL_DOWN|PSP_CTRL_ANALDOWN)))
      {
        cur_x = sx + max_item_w + UiMetric.MenuItemMargin + 10;
        min_y = sy + pos.Index * fh;
        cur_y = min_y + fh / 2;
        max_y = sy + (pos.Index  + 1) * fh;
        min_x = cur_x - UiMetric.MenuItemMargin;
        max_x = cur_x + max_option_w + UiMetric.MenuItemMargin;
        cur_x += pspFontGetTextWidth(UiMetric.Font, " >");
        if (sel->Selected && sel->Selected->Text)
          cur_x += pspFontGetTextWidth(UiMetric.Font, sel->Selected->Text);

        const PspMenuOption *option;
        for (option = temp_option; option && min_y >= sy; option = option->Prev, min_y -= fh);
        for (option = temp_option->Next; option && max_y < dy; option = option->Next, max_y += fh);
        max_y += fh;
      }

      if (option_mode)
      {
        if (pad.buttons & PSP2_CTRL_RIGHT || pad.buttons & UiMetric.OkButton)
        {
          option_mode = 0;

          /* If the callback function refuses the change, restore selection */
          if (!uimenu->OnItemChanged || uimenu->OnItemChanged(uimenu, sel, temp_option))
            sel->Selected = temp_option;
        }
        else if (pad.buttons & PSP2_CTRL_LEFT  || pad.buttons & UiMetric.CancelButton)
        {
          option_mode = 0;

          if (pad.buttons & UiMetric.CancelButton)
            pad.buttons &= ~UiMetric.CancelButton;
        }

        if (!option_mode)
        {
          if (UiMetric.Animate)
          {
            /* Deflation animation */
            for (i = UI_ANIM_FRAMES - 1; i >= 0; i--)
            {
          	  pspVideoBegin();
              if (!UiMetric.Background) pspVideoClearScreen();
                else pspVideoPutImage(UiMetric.Background, 0, 0,
                  UiMetric.Background->Viewport.Width, UiMetric.Background->Height);

          	  //TODO pspVideoCallList(call_list);

              /* Perform any custom drawing */
              if (uimenu->OnRender)
                uimenu->OnRender(uimenu, sel);

          	  /* Clear screen */
          	  pspVideoFillRect(cur_x - ((cur_x - min_x) / UI_ANIM_FRAMES) * i,
          	    cur_y - ((cur_y - min_y) / UI_ANIM_FRAMES) * i,
          	    cur_x + ((max_x - cur_x) / UI_ANIM_FRAMES) * i,
          	    cur_y + ((max_y - cur_y) / UI_ANIM_FRAMES) * i,
                UiMetric.MenuOptionBoxBg);

              /* Selected option for the item */
              if (sel->Selected && sel->Selected->Text)
              pspVideoPrint(UiMetric.Font,
                sx + max_item_w + UiMetric.MenuItemMargin + 10,
                sy + pos.Index * fh, sel->Selected->Text, UiMetric.SelectedColor);

          	  pspVideoEnd();

              /* Swap buffers */
              pspVideoWaitVSync();
              pspVideoSwapBuffers();
          	}
          }
        }
      }
      else
      {
        if ((pad.buttons & PSP2_CTRL_RIGHT)
          && sel->Options && sel->Options->Next)
        {
          option_mode = 1;
          max_option_w = 0;
          int width;
          const PspMenuOption *option;

          /* Find the longest option caption */
          for (option = sel->Options; option; option = option->Next)
            if (option->Text && (width = pspFontGetTextWidth(UiMetric.Font, option->Text)) > max_option_w)
              max_option_w = width;

          temp_option = (sel->Selected) ? sel->Selected : sel->Options;

          /* Determine bounds */
          cur_x = sx + max_item_w + UiMetric.MenuItemMargin + 10;
          min_y = sy + pos.Index * fh;
          cur_y = min_y + fh / 2;
          max_y = sy + (pos.Index  + 1) * fh;
          min_x = cur_x - UiMetric.MenuItemMargin;
          max_x = cur_x + max_option_w + UiMetric.MenuItemMargin;
          cur_x += pspFontGetTextWidth(UiMetric.Font, " >");
          if (sel->Selected && sel->Selected->Text)
            cur_x += pspFontGetTextWidth(UiMetric.Font, sel->Selected->Text);

          for (option = temp_option; option && min_y >= sy; option = option->Prev, min_y -= fh);
          for (option = temp_option->Next; option && max_y < dy; option = option->Next, max_y += fh);
          max_y += fh;

          if (UiMetric.Animate)
          {
            /* Expansion animation */
            for (i = 0; i <= UI_ANIM_FRAMES; i++)
            {
          	  pspVideoBegin();

              if (!UiMetric.Background) pspVideoClearScreen();
                else pspVideoPutImage(UiMetric.Background, 0, 0,
                  UiMetric.Background->Viewport.Width,
                  UiMetric.Background->Height);

          	  //TODO pspVideoCallList(call_list);

              /* Perform any custom drawing */
              if (uimenu->OnRender)
                uimenu->OnRender(uimenu, sel);

          	  pspVideoFillRect(cur_x - ((cur_x - min_x) / UI_ANIM_FRAMES) * i,
          	    cur_y - ((cur_y - min_y) / UI_ANIM_FRAMES) * i,
          	    cur_x + ((max_x - cur_x) / UI_ANIM_FRAMES) * i,
          	    cur_y + ((max_y - cur_y) / UI_ANIM_FRAMES) * i,
                UiMetric.MenuOptionBoxBg);

          	  pspVideoEnd();

              /* Swap buffers */
              pspVideoWaitVSync();
              pspVideoSwapBuffers();
          	}
        	}
        }
        else if (pad.buttons & UiMetric.OkButton)
        {
          if (!uimenu->OnOk || uimenu->OnOk(uimenu, sel))
            break;
        }
      }
    }

    if (!option_mode)
    {
      if (pad.buttons & UiMetric.CancelButton)
      {
        if (uimenu->OnCancel)
          uimenu->OnCancel(uimenu, sel);
        break;
      }

      if ((pad.buttons & CONTROL_BUTTON_MASK) && uimenu->OnButtonPress)
      {
        if (uimenu->OnButtonPress(uimenu, sel, pad.buttons & CONTROL_BUTTON_MASK))
            break;
      }
    }

    /* Render to a call list */
    //sceGuStart(GU_CALL, call_list);
    pspVideoBegin();

    /* Draw instructions */
    if (sel)
    {
      const char *dirs = NULL;

      if (!option_mode && sel->HelpText)
      {
        static char help_copy[MAX_DIR_LEN];
        strncpy(help_copy, sel->HelpText, MAX_DIR_LEN);
        help_copy[MAX_DIR_LEN - 1] = '\0';
        ReplaceIcons(help_copy);

        dirs = help_copy;
      }
      else if (option_mode)
      {
        static char help_copy[MAX_DIR_LEN];
        strncpy(help_copy, OptionModeTemplate, MAX_DIR_LEN);
        help_copy[MAX_DIR_LEN - 1] = '\0';
        ReplaceIcons(help_copy);

        dirs = help_copy;
      }

      if (dirs)
        pspVideoPrintCenter(UiMetric.Font,
          0, SCR_HEIGHT - fh, SCR_WIDTH, dirs, UiMetric.StatusBarColor);
    }

    /* Draw title */
    if (title)
    {
      pspVideoPrint(UiMetric.Font, UiMetric.Left, UiMetric.Top,
        title, UiMetric.TitleColor);
      pspVideoDrawLine(UiMetric.Left, UiMetric.Top + fh - 1, UiMetric.Left + w,
        UiMetric.Top + fh - 1, UiMetric.TitleColor);
    }

    /* Render the menu items */
    for (item = pos.Top, i = 0, j = sy; item && i < lnmax; item = item->Next, j += fh, i++)
    {
      if (item->Caption)
      {
  	    /* Section header */
  	    if (item->Caption[0] == '\t')
    		{
    		  // if (i != 0) j += fh / 2;
          pspVideoPrint(UiMetric.Font, sx, j, item->Caption + 1, UiMetric.TitleColor);
          pspVideoDrawLine(sx, j + fh - 1, sx + w, j + fh - 1, UiMetric.TitleColor);
    		  continue;
    		}

        if (item == sel) sel_top = j;

        /* Item caption */
        pspVideoPrint(UiMetric.Font, sx + 10, j, item->Caption,
          (item == sel) ? UiMetric.SelectedColor : UiMetric.TextColor);

        if (!option_mode || item != sel)
        {
          /* Selected option for the item */
          if (item->Selected)
          {
            k = sx + max_item_w + UiMetric.MenuItemMargin + 10;
            k += pspVideoPrint(UiMetric.Font, k, j, item->Selected->Text,
              (item == sel) ? UiMetric.SelectedColor : UiMetric.TextColor);

            if (!option_mode && item == sel)
              if (sel->Options && sel->Options->Next)
                pspVideoPrint(UiMetric.Font, k + anim_frame, j, " >", UiMetric.MenuDecorColor);
          }
        }
      }
    }

    /* Render status information */
    RenderStatus();

    /* Draw scrollbar */
    if (sbh > 0)
    {
      sby = sy + (int)((float)(h - sbh) * ((float)(pos.Offset + pos.Index) / (float)menu->Count));
      pspVideoFillRect(dx - UiMetric.ScrollbarWidth, sy, dx, dy, UiMetric.ScrollbarBgColor);
      pspVideoFillRect(dx - UiMetric.ScrollbarWidth, sby, dx, sby + sbh, UiMetric.ScrollbarColor);
    }

    /* End writing to call list */
    pspVideoEnd();

    if (!option_mode && !fast_scroll && sel && last_sel
      && UiMetric.Animate && last_sel != sel)
    {
      /* Move animation */
      int f, n = 4;
      for (f = 1; f <= n; f++)
      {
        pspVideoBegin();

        /* Clear screen */
        if (!UiMetric.Background) pspVideoClearScreen();
        else pspVideoPutImage(UiMetric.Background, 0, 0,
          UiMetric.Background->Viewport.Width, UiMetric.Background->Height);

        int box_top = last_sel_top-((last_sel_top-sel_top)/n)*f;
        pspVideoFillRect(sx, box_top, sx+w, box_top+fh,
          UiMetric.SelectedBgColor);

        //TODO sceGuCallList(call_list);

        /* Perform any custom drawing */
        if (uimenu->OnRender)
          uimenu->OnRender(uimenu, sel);

        pspVideoEnd();

        /* Swap buffers */
        pspVideoWaitVSync();
        pspVideoSwapBuffers();
      }
    }

    /* Begin direct rendering */
    pspVideoBegin();

    /* Clear screen */
    if (!UiMetric.Background) pspVideoClearScreen();
    else pspVideoPutImage(UiMetric.Background, 0, 0,
      UiMetric.Background->Viewport.Width, UiMetric.Background->Height);

    /* Draw the highlight for selected item */
    if (!option_mode)
      pspVideoFillRect(sx, sel_top, sx+w, sel_top+fh,
        UiMetric.SelectedBgColor);

    //TODO pspVideoCallList(call_list);

    /* Perform any custom drawing */
    if (uimenu->OnRender)
      uimenu->OnRender(uimenu, sel);

    /* Render menu options */
    if (option_mode)
    {
      k = sx + max_item_w + UiMetric.MenuItemMargin + 10;
      int arrow_x = min_x + (UiMetric.MenuItemMargin / 2 - arrow_w / 2);
      const PspMenuOption *option;

      /* Background */
      pspVideoFillRect(min_x, min_y, max_x, max_y, UiMetric.MenuOptionBoxBg);
      pspVideoFillRect(min_x, sy + pos.Index * fh, max_x,
        sy + (pos.Index + 1) * fh, UiMetric.MenuSelOptionBg);
      pspVideoGlowRect(min_x, min_y, max_x - 1, max_y - 1,
        COLOR(0xff,0xff,0xff,UI_ANIM_FOG_STEP * UI_ANIM_FRAMES), 2);

      /* Render selected item + previous items */
      i = sy + pos.Index * fh;
      for (option = temp_option; option && i >= sy; option = option->Prev, i -= fh)
        pspVideoPrint(UiMetric.Font, k, i, option->Text, (option == temp_option)
          ? UiMetric.SelectedColor : UiMetric.MenuOptionBoxColor);

      /* Up arrow */
      if (option) pspVideoPrint(UiMetric.Font, arrow_x,
          i + fh + anim_frame, PSP_CHAR_UP_ARROW, UiMetric.MenuDecorColor);

      /* Render following items */
      i = sy + (pos.Index  + 1) * fh;
      for (option = temp_option->Next; option && i < dy; option = option->Next, i += fh)
        pspVideoPrint(UiMetric.Font, k, i, option->Text,
          UiMetric.MenuOptionBoxColor);

      /* Down arrow */
      if (option) pspVideoPrint(UiMetric.Font, arrow_x, i - fh - anim_frame,
          PSP_CHAR_DOWN_ARROW, UiMetric.MenuDecorColor);
    }

    pspVideoEnd();

    /* Wait if needed */
    do { sceRtcGetCurrentTick(&current_tick); }
    while (current_tick - last_tick < ticks_per_upd);
    last_tick = current_tick;

    /* Swap buffers */
    pspVideoWaitVSync();
    pspVideoSwapBuffers();

    last_sel = sel;
    last_sel_top = sel_top;
  }

  menu->Selected = sel;
}

void pspUiSplashScreen(PspUiSplash *splash)
{
  SceCtrlData pad;
  int fh = pspFontGetLineHeight(UiMetric.Font);

  while (!ExitPSP)
  {
    if (!pspCtrlPollControls(&pad))
      continue;

    if (pad.buttons & UiMetric.CancelButton)
    {
      if (splash->OnCancel) splash->OnCancel(splash, NULL);
      break;
    }

    if ((pad.buttons & CONTROL_BUTTON_MASK) && splash->OnButtonPress)
    {
      if (splash->OnButtonPress(splash, pad.buttons & CONTROL_BUTTON_MASK))
          break;
    }

    pspVideoBegin();

    /* Clear screen */
    if (UiMetric.Background)
      pspVideoPutImage(UiMetric.Background, 0, 0,
        UiMetric.Background->Viewport.Width, UiMetric.Background->Height);
    else
      pspVideoClearScreen();

    /* Draw instructions */
    const char *dirs = (splash->OnGetStatusBarText)
      ? splash->OnGetStatusBarText(splash)
      : SplashStatusBarTemplate;
    pspVideoPrintCenter(UiMetric.Font, UiMetric.Left,
      SCR_HEIGHT - fh, UiMetric.Right, dirs, UiMetric.StatusBarColor);

    /* Render status information */
    RenderStatus();

    /* Perform any custom drawing */
    if (splash->OnRender)
      splash->OnRender(splash, NULL);

    pspVideoEnd();

    /* Swap buffers */
    pspVideoWaitVSync();
    pspVideoSwapBuffers();
  }
}

const PspMenuItem* pspUiSelect(const char *title, const PspMenu *menu)
{
  const PspMenuItem *sel, *item, *last_sel = NULL;
  struct UiPos pos;
  int lnmax, lnhalf;
  int i, j, h, w, fh = pspFontGetLineHeight(UiMetric.Font);
  int sx, sy, dx, dy;
  int anim_frame = 0, anim_incr = 1;
  int arrow_w = pspFontGetTextWidth(UiMetric.Font, PSP_CHAR_DOWN_ARROW);
  int widest = 100;
  int sel_top = 0, last_sel_top = 0;
  SceCtrlData pad;

  char *help_text = strdup(SelectorTemplate);
  ReplaceIcons(help_text);

  //TODO memset(call_list, 0, sizeof(call_list));

  /* Determine width of the longest caption */
  for (item = menu->First; item; item = item->Next)
  {
    if (item->Caption)
    {
      int item_w = pspFontGetTextWidth(UiMetric.Font, item->Caption);
      if (item_w > widest)
        widest = item_w;
    }
  }

  widest += UiMetric.MenuItemMargin * 2;

  sx = SCR_WIDTH - widest;
  sy = UiMetric.Top;
  dx = SCR_WIDTH;
  dy = UiMetric.Bottom;
  w = dx - sx;
  h = dy - sy;

  uint32_t ticks_per_sec, ticks_per_upd;
  uint64_t current_tick, last_tick;

  /* Initialize variables */
  lnmax = (dy - sy) / fh;
  lnhalf = lnmax >> 1;

  sel = menu->First;
  pos.Top = menu->First;
  pos.Index = pos.Offset = 0;

  pspVideoWaitVSync();

  /* Compute update frequency */
  ticks_per_sec = sceRtcGetTickResolution();
  sceRtcGetCurrentTick(&last_tick);
  ticks_per_upd = ticks_per_sec / UiMetric.MenuFps;

  /* Get copy of screen */
  PspImage *screen = pspVideoGetVramBufferCopy();

  if (UiMetric.Animate)
  {
    /* Intro animation */
    for (i = 0; i < UI_ANIM_FRAMES; i++)
    {
  	  pspVideoBegin();

  	  /* Clear screen */
  	  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

  	  /* Apply fog and draw right frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0, 0, 0, UI_ANIM_FOG_STEP * i));
  	  pspVideoFillRect(SCR_WIDTH - (i * (widest / UI_ANIM_FRAMES)),
        0, dx, SCR_HEIGHT, UiMetric.MenuOptionBoxBg);

  	  pspVideoEnd();

      /* Swap buffers */
      pspVideoWaitVSync();
      pspVideoSwapBuffers();
  	}
  }

  int fast_scroll;

  /* Begin navigation loop */
  while (!ExitPSP)
  {
    if (!pspCtrlPollControls(&pad))
      continue;

    fast_scroll = 0;

    /* Incr/decr animation frame */
    anim_frame += (UiMetric.Animate) ? anim_incr : 0;
    if (anim_frame > 2 || anim_frame < 0)
      anim_incr *= -1;

    /* Check the directional buttons */
    if (sel)
    {
      if ((pad.buttons & PSP2_CTRL_DOWN || pad.buttons & PSP_CTRL_ANALDOWN)
        && sel->Next)
      {
        fast_scroll = pad.buttons & PSP_CTRL_ANALDOWN;
        if (pos.Index + 1 >= lnmax) { pos.Offset++; pos.Top = pos.Top->Next; }
        else pos.Index++;
        sel = sel->Next;
      }
      else if ((pad.buttons & PSP2_CTRL_UP || pad.buttons & PSP_CTRL_ANALUP)
        && sel->Prev)
      {
        fast_scroll = pad.buttons & PSP_CTRL_ANALUP;
        if (pos.Index - 1 < 0) { pos.Offset--; pos.Top = pos.Top->Prev; }
        else pos.Index--;
        sel = sel->Prev;
      }
      else if (pad.buttons & PSP2_CTRL_LEFT)
      {
        for (i = 0; sel->Prev && i < lnhalf; i++)
        {
          if (pos.Index - 1 < 0) { pos.Offset--; pos.Top = pos.Top->Prev; }
          else pos.Index--;
          sel = sel->Prev;
        }
      }
      else if (pad.buttons & PSP2_CTRL_RIGHT)
      {
        for (i = 0; sel->Next && i < lnhalf; i++)
        {
          if (pos.Index + 1 >= lnmax) { pos.Offset++; pos.Top = pos.Top->Next; }
          else pos.Index++;
          sel=sel->Next;
        }
      }

      if (pad.buttons & UiMetric.OkButton) break;
    }

    if (pad.buttons & UiMetric.CancelButton) { sel = NULL; break; }

    /* Render to a call list */
    //sceGuStart(GU_CALL, call_list);
    pspVideoBegin();

    /* Apply fog and draw frame */
    pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
      COLOR(0, 0, 0, UI_ANIM_FOG_STEP * UI_ANIM_FRAMES));
    pspVideoGlowRect(sx, 0, dx - 1, SCR_HEIGHT - 1,
      COLOR(0xff,0xff,0xff,UI_ANIM_FOG_STEP * UI_ANIM_FRAMES), 2);

    /* Title */
    if (title)
      pspVideoPrintCenter(UiMetric.Font, sx, 0, dx,
        title, UiMetric.TitleColor);

    /* Render the items */
    for (item = (PspMenuItem*)pos.Top, i = 0, j = sy;
      item && i < lnmax; item = item->Next, j += fh, i++)
    {
      if (item == sel) sel_top = j;
      pspVideoPrintClipped(UiMetric.Font, sx + 10, j, item->Caption, w - 10,
        "...", (item == sel) ? UiMetric.SelectedColor : UiMetric.TextColor);
    }

    /* Up arrow */
    if (pos.Top->Prev) pspVideoPrint(UiMetric.Font, SCR_WIDTH - arrow_w * 2,
        sy + anim_frame, PSP_CHAR_UP_ARROW, UiMetric.MenuDecorColor);

    /* Down arrow */
    if (item) pspVideoPrint(UiMetric.Font, SCR_WIDTH - arrow_w * 2,
        dy - fh - anim_frame, PSP_CHAR_DOWN_ARROW, UiMetric.MenuDecorColor);

    /* Shortcuts */
    pspVideoPrintCenter(UiMetric.Font, sx, SCR_HEIGHT - fh, dx,
      help_text, UiMetric.StatusBarColor);

    pspVideoEnd();

    if (sel != last_sel && !fast_scroll && sel && last_sel
      && UiMetric.Animate)
    {
      /* Move animation */
      int f, n = 4;
      for (f = 1; f <= n; f++)
      {
        pspVideoBegin();

        /* Clear screen */
        pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);
        pspVideoFillRect(sx, 0, dx, SCR_HEIGHT, UiMetric.MenuOptionBoxBg);

        /* Selection box */
        int box_top = last_sel_top-((last_sel_top-sel_top)/n)*f;
        pspVideoFillRect(sx, box_top, sx + w, box_top + fh,
          UiMetric.SelectedBgColor);

        //TODO sceGuCallList(call_list);

        pspVideoEnd();

        pspVideoWaitVSync();
        pspVideoSwapBuffers();
      }
    }

    pspVideoBegin();

    /* Clear screen */
    pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);
    pspVideoFillRect(sx, 0, dx, SCR_HEIGHT, UiMetric.MenuOptionBoxBg);

    if (sel) pspVideoFillRect(sx, sel_top, sx + w, sel_top + fh,
      UiMetric.SelectedBgColor);

    //TODO sceGuCallList(call_list);

    pspVideoEnd();

    /* Wait if needed */
    do { sceRtcGetCurrentTick(&current_tick); }
    while (current_tick - last_tick < ticks_per_upd);
    last_tick = current_tick;

    /* Swap buffers */
    pspVideoWaitVSync();
    pspVideoSwapBuffers();

    last_sel = sel;
    last_sel_top = sel_top;
  }

  if (UiMetric.Animate)
  {
    /* Exit animation */
    for (i = UI_ANIM_FRAMES - 1; i >= 0; i--)
    {
  	  pspVideoBegin();

  	  /* Clear screen */
  	  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

  	  /* Apply fog and draw right frame */
  	  pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT,
  	    COLOR(0, 0, 0, UI_ANIM_FOG_STEP * i));
  	  pspVideoFillRect(SCR_WIDTH - (i * (widest / UI_ANIM_FRAMES)),
        0, dx, SCR_HEIGHT, UiMetric.MenuOptionBoxBg);

  	  pspVideoEnd();

      /* Swap buffers */
      pspVideoWaitVSync();
      pspVideoSwapBuffers();
  	}
  }

  free(help_text);
  pspImageDestroy(screen);

  return sel;
}

void pspUiFadeout()
{
  /* Get copy of screen */
  PspImage *screen = pspVideoGetVramBufferCopy();

  /* Exit animation */
  int i, alpha;
  for (i = 0; i < UI_ANIM_FRAMES; i++)
  {
	  pspVideoBegin();

	  /* Clear screen */
	  pspVideoPutImage(screen, 0, 0, screen->Viewport.Width, screen->Height);

	  /* Apply fog */
	  alpha = (0x100/UI_ANIM_FRAMES)*i-1;
	  if (alpha > 0)
	    pspVideoFillRect(0, 0, SCR_WIDTH, SCR_HEIGHT, COLOR(0,0,0,alpha));

	  pspVideoEnd();

    /* Swap buffers */
    pspVideoWaitVSync();
    pspVideoSwapBuffers();
	}

  pspImageDestroy(screen);
}
