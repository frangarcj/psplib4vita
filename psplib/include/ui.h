/** PSP helper library ***************************************/
/**                                                         **/
/**                           ui.h                          **/
/**                                                         **/
/** This file contains declarations for a simple GUI        **/
/** rendering library                                       **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_UI_H
#define _PSP_UI_H

#include "video.h"
#include "menu.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct PspUiMetric
{
  const PspImage *Background;
  const PspFont *Font;
  uint64_t CancelButton;
  uint64_t OkButton;
  int Left;
  int Top;
  int Right;
  int Bottom;
  uint32_t ScrollbarColor;
  uint32_t ScrollbarBgColor;
  int ScrollbarWidth;
  uint32_t TextColor;
  uint32_t SelectedColor;
  uint32_t SelectedBgColor;
  uint32_t StatusBarColor;
  int MenuFps;

  uint32_t DialogFogColor;

  uint32_t BrowserFileColor;
  uint32_t BrowserDirectoryColor;

  int GalleryIconsPerRow;
  int GalleryIconMarginWidth;

  int MenuItemMargin;
  uint32_t MenuSelOptionBg;
  uint32_t MenuOptionBoxColor;
  uint32_t MenuOptionBoxBg;
  uint32_t MenuDecorColor;

  int TitlePadding;
  uint32_t TitleColor;
  uint32_t TabBgColor;
  int Animate;
} PspUiMetric;

typedef struct PspUiFileBrowser
{
  void (*OnRender)(const void *browser, const void *path);
  int  (*OnOk)(const void *browser, const void *file);
  int  (*OnCancel)(const void *gallery, const void *parent_dir);
  int  (*OnButtonPress)(const struct PspUiFileBrowser* browser,
         const char *selected, uint32_t button_mask);
  const char **Filter;
  void *Userdata;
} PspUiFileBrowser;

typedef struct PspUiMenu
{
  PspMenu *Menu;
  void (*OnRender)(const void *uimenu, const void *item);
  int  (*OnOk)(const void *menu, const void *item);
  int  (*OnCancel)(const void *menu, const void *item);
  int  (*OnButtonPress)(const struct PspUiMenu *menu, PspMenuItem* item,
         uint32_t button_mask);
  int  (*OnItemChanged)(const struct PspUiMenu *menu, PspMenuItem* item,
         const PspMenuOption* option);
} PspUiMenu;

typedef struct PspUiGallery
{
  PspMenu *Menu;
  void (*OnRender)(const void *gallery, const void *item);
  int  (*OnOk)(const void *gallery, const void *item);
  int  (*OnCancel)(const void *gallery, const void *item);
  int  (*OnButtonPress)(const struct PspUiGallery *gallery, PspMenuItem* item,
          uint32_t button_mask);
  void *Userdata;
} PspUiGallery;

typedef struct PspUiSplash
{
  void (*OnRender)(const void *splash, const void *null);
  int  (*OnCancel)(const void *splash, const void *null);
  int  (*OnButtonPress)(const struct PspUiSplash *splash, uint32_t button_mask);
  const char* (*OnGetStatusBarText)(const struct PspUiSplash *splash);
} PspUiSplash;

#define PSP_UI_YES     2
#define PSP_UI_NO      1
#define PSP_UI_CANCEL  0

#define PSP_UI_CONFIRM 1

char pspUiGetButtonIcon(uint32_t button_mask);

void pspUiOpenBrowser(PspUiFileBrowser *browser, const char *start_path);
void pspUiOpenGallery(const PspUiGallery *gallery, const char *title);
void pspUiOpenMenu(const PspUiMenu *uimenu, const char *title);
void pspUiSplashScreen(PspUiSplash *splash);

int  pspUiConfirm(const char *message);
int  pspUiYesNoCancel(const char *message);
void pspUiAlert(const char *message);
void pspUiFlashMessage(const char *message);
const PspMenuItem* pspUiSelect(const char *title, const PspMenu *menu);

void pspUiFadeout();

PspUiMetric UiMetric;

#ifdef __cplusplus
}
#endif

#endif  // _PSP_UI_H
