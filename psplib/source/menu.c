/** PSP helper library ***************************************/
/**                                                         **/
/**                          menu.c                         **/
/**                                                         **/
/** This file contains a generic menu system                **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include <malloc.h>
#include <string.h>

#include "menu.h"

static void _pspMenuDestroyItem(PspMenuItem* item);

PspMenu* pspMenuCreate()
{
  PspMenu* menu = (PspMenu*)malloc(sizeof(PspMenu));

  menu->Count = 0;
  menu->First = NULL;
  menu->Last = NULL;
  menu->Selected = NULL;

  return menu;
}

void pspMenuClear(PspMenu *menu)
{
  PspMenuItem *item, *next;

  for (item = menu->First; item; item = next)
  {
    next = item->Next;
    _pspMenuDestroyItem(item);
  }

  menu->Count = 0;
  menu->First = NULL;
  menu->Last = NULL;
  menu->Selected = NULL;
}

void pspMenuDestroy(PspMenu* menu)
{
  pspMenuClear(menu);
  free(menu);
}

void pspMenuLoad(PspMenu *menu, const PspMenuItemDef *def)
{
  PspMenuItem *item;
  const PspMenuOptionDef *option_def;

  /* If the menu is already loaded, clear it */
  if (menu->First) pspMenuClear(menu);

  /* Initialize menu */
  for (; def->ID || def->Caption; def++)
  {
    /* Append the item */
    item = pspMenuAppendItem(menu, def->Caption, def->ID);

    /* Add the options */
    if (def->OptionList)
      for (option_def = def->OptionList; option_def->Text; option_def++)
        pspMenuAppendOption(item, option_def->Text, option_def->Value, 0);

    /* Select proper option */
    if (def->SelectedIndex >= 0)
      pspMenuSelectOptionByIndex(item, def->SelectedIndex);

    /* Set help text */
    item->HelpText = (def->HelpText) ? strdup(def->HelpText) : NULL;
  }
}

PspMenuOption* pspMenuAppendOption(PspMenuItem *item, const char *text, const void *value, int select)
{
  PspMenuOption *option, *o;

  if (!(option=(PspMenuOption*)malloc(sizeof(PspMenuOption))))
    return NULL;

  if (!(option->Text=strdup(text)))
  {
    free(option);
    return NULL;
  }

  option->Value=value;
  option->Next=NULL;

  if (item->Options)
  {
    for (o=item->Options; o->Next; o=o->Next);
    o->Next=option;
    option->Prev=o;
  }
  else
  {
    item->Options=option;
    option->Prev=NULL;
  }

  if (select) item->Selected=option;

  return option;
}

void pspMenuSetCaption(PspMenuItem *item, const char *caption)
{
  if (item->Caption) free(item->Caption);
  item->Caption = (caption) ? strdup(caption) : NULL;
}

void pspMenuSetHelpText(PspMenuItem *item, const char *helptext)
{
  if (item->HelpText) free(item->HelpText);
  item->HelpText = (helptext) ? strdup(helptext) : NULL;
}

void pspMenuSelectOptionByValue(PspMenuItem *item, const void *value)
{
  PspMenuOption *option;

  for (option = item->Options; option; option = option->Next)
    if (option->Value == value) { item->Selected = option; break; }
}

void pspMenuSelectOptionByIndex(PspMenuItem *item, int index)
{
  PspMenuOption *option;
  int i;

  for (i = 0, option = item->Options; i <= index && option; i++, option = option->Next)
    if (i == index) { item->Selected = option; break; }
}

void pspMenuModifyOption(PspMenuOption *option, const char *text, const void *value)
{
  free(option->Text);

  option->Text=NULL;
  option->Value=value;

  if (!(option->Text=strdup(text)))
    return;
}

void pspMenuClearOptions(PspMenuItem *item)
{
  PspMenuOption *option, *next;

  for (option=item->Options; option; option=next)
  {
    next=option->Next;
    free(option->Text);
    free(option);
  }

  item->Selected=NULL;
  item->Options=NULL;
}

PspMenuItem* pspMenuFindItemById(PspMenu *menu, unsigned int id)
{
  PspMenuItem *item;
  for (item=menu->First; item; item=item->Next)
    if (item->ID == id)
      return item;

  return NULL;
}

PspMenuItem* pspMenuGetNthItem(PspMenu *menu, int index)
{
  PspMenuItem *item;
  int i;

  for (item=menu->First, i=0; item; item=item->Next, i++)
    if (i == index) 
      return item;

  return NULL;
}

PspMenuItem* pspMenuAppendItem(PspMenu *menu, const char *caption, 
  unsigned int id)
{
  PspMenuItem* item = (PspMenuItem*)malloc(sizeof(PspMenuItem));

  if (!item) return NULL;

  if (caption)
  {
    if (!(item->Caption = strdup(caption)))
    {
      free(item);
      return NULL;
    }
  }
  else item->Caption = NULL;

  item->HelpText = NULL;
  item->Icon = NULL;
  item->ID = id;
  item->Param = NULL;
  item->Options = NULL;
  item->Selected = NULL;
  item->Next = NULL;
  item->Prev = menu->Last;
  menu->Count++;

  if (menu->Last)
  {
    menu->Last->Next = item;
    menu->Last = item;
  }
  else menu->First = menu->Last = item;

  return item;
}

void _pspMenuDestroyItem(PspMenuItem* item)
{
  if (item->Caption) free(item->Caption);
  if (item->HelpText) free(item->HelpText);

  pspMenuClearOptions(item);

  free(item);
}
