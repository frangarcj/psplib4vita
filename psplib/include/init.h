/** PSP helper library ***************************************/
/**                                                         **/
/**                         init.h                          **/
/**                                                         **/
/** This file contains declarations for INI file loading/   **/
/** saving                                                  **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_INIT_H
#define _PSP_INIT_H

#ifdef __cplusplus
extern "C" {
#endif

struct PspInitPair
{
  char *Key;
  char *Value;
  struct PspInitPair *Next;
};

struct PspInitSection
{
  char *Name;
  struct PspInitPair *Head;
  struct PspInitSection *Next;
};

typedef struct
{
  struct PspInitSection *Head;
} PspInit;

PspInit* pspInitCreate();
void  pspInitDestroy(PspInit *init);

int   pspInitLoad(PspInit *init, const char *file);
int   pspInitSave(const PspInit *init, const char *file);

int   pspInitGetInt(const PspInit *init, const char *section, const char *key, int default_value);
void  pspInitSetInt(PspInit *init, const char *section, const char *key, int value);
char* pspInitGetString(const PspInit *init, const char *section, const char *key, const char *default_value);
void  pspInitSetString(PspInit *init, const char *section, const char *key, const char *string);

#ifdef __cplusplus
}
#endif

#endif // _PSP_INIT_H
