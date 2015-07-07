/** PSP helper library ***************************************/
/**                                                         **/
/**                         init.c                          **/
/**                                                         **/
/** This file contains routines for INI file loading/saving **/
/** Its idea is based on a similar library by Marat         **/
/** Fayzullin                                               **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include "init.h"

#include <string.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>

PspInit* pspInitCreate()
{
  PspInit *init = (PspInit*)malloc(sizeof(PspInit));
  if (!init) return NULL;

  init->Head = NULL;

  return init;
}
static struct PspInitSection* CreateSection(const char *name)
{
  struct PspInitSection *section
    = (struct PspInitSection*)malloc(sizeof(struct PspInitSection));
  section->Head = NULL;
  section->Next = NULL;
  section->Name = strdup(name);

  return section;
}

static struct PspInitPair *CreatePair(char *string)
{
  char *ptr;
  if (!(ptr = strchr(string, '=')))
    return NULL;

  int len;
  char *name, *value;

  /* Copy NAME */
  len = ptr - string;
  if (!(name = (char*)malloc(sizeof(char) * (len + 1))))
    return NULL;
  strncpy(name, string, len);
  name[len] = '\0';

  /* Copy VALUE */
  if (!(value = strdup(ptr + 1)))
  {
    free(name);
    return NULL;
  }
  len = strlen(value);
  if (value[len - 1] == '\n') value[len - 1] = '\0';

  /* Create struct */
  struct PspInitPair *pair
    = (struct PspInitPair*)malloc(sizeof(struct PspInitPair));

  if (!pair)
  {
    free(name);
    free(value);
    return NULL;
  }

  pair->Key = name;
  pair->Value = value;
  pair->Next = NULL;

  return pair;
}

int pspInitLoad(PspInit *init, const char *filename)
{
  FILE *file;
  if (!(file = fopen(filename, "r"))) return 0;

  struct PspInitSection *current_section = NULL;
  struct PspInitPair *tail;
  char string[512], name[512];
  char *ptr;
  int len;

  /* Create unnamed section */
  current_section = NULL;
  tail = NULL;

  while(!feof(file) && fgets(string, sizeof(string), file))
  {
    /* TODO: Skip whitespace */
    /* New section */
    if (string[0] == '[')
    {
      if ((ptr = strrchr(string, ']')))
      {
        len = ptr - string - 1;
        strncpy(name, string + 1, len);
        name[len] = '\0';

        if (current_section)
        {
          current_section->Next = CreateSection(name);
          current_section = current_section->Next;
        }
        else
        {
          current_section = init->Head = CreateSection(name);
        }

        tail = NULL;
      }
    }
    else if (string[0] =='#'); /* Do nothing - comment */
    else
    {
      /* No section defined - create empty section */
      if (!current_section)
      {
        current_section = CreateSection(strdup(""));
        init->Head = current_section;
        tail = NULL;
      }

      struct PspInitPair *pair = CreatePair(string);
      if ((pair = CreatePair(string)))
      {
        if (tail) tail->Next = pair;
        else current_section->Head = pair;
        tail = pair;
      }
    }
  }

  fclose(file);
  return 1;
}

int pspInitSave(const PspInit *init, const char *filename)
{
  FILE *file ;
  if (!(file= fopen(filename, "w"))) return 0;

  struct PspInitSection *section;
  struct PspInitPair *pair;

  for (section = init->Head; section; section = section->Next)
  {
    fprintf(file, "[%s]\n", section->Name);
    for (pair = section->Head; pair; pair = pair->Next)
      fprintf(file, "%s=%s\n", pair->Key, pair->Value);
  }

  fclose(file);
  return 1;
}

static struct PspInitSection* FindSection(const PspInit *init, const char *section_name)
{
  struct PspInitSection *section;

  for (section = init->Head; section; section = section->Next)
    if (strcmp(section_name, section->Name) == 0)
      return section;

  return NULL;
}

static struct PspInitPair* FindPair(const struct PspInitSection *section, const char *key_name)
{
  struct PspInitPair *pair;
  for (pair = section->Head; pair; pair = pair->Next)
    if (strcmp(pair->Key, key_name) == 0)
      return pair;

  return NULL;
}

static struct PspInitPair* Locate(const PspInit *init, const char *section_name, const char *key_name)
{
  struct PspInitSection *section;
  if (!(section = FindSection(init, section_name)))
    return NULL;

  return FindPair(section, key_name);
}

static struct PspInitPair* LocateOrCreate(PspInit *init, const char *section_name, const char *key_name)
{
  struct PspInitSection *section = FindSection(init, section_name);
  struct PspInitPair *pair = NULL;

  if (section) pair = FindPair(section, key_name);
  else
  {
    /* Create section */
    section = CreateSection(section_name);

    if (!init->Head) init->Head = section;
    else
    {
      struct PspInitSection *s;
      for (s = init->Head; s->Next; s = s->Next); /* Find the tail */
      s->Next = section;
    }
  }

  if (!pair)
  {
    /* Create pair */
    pair = (struct PspInitPair*)malloc(sizeof(struct PspInitPair));
    pair->Key = strdup(key_name);
    pair->Value = NULL;
    pair->Next = NULL;

    if (!section->Head) section->Head = pair;
    else
    {
      struct PspInitPair *p;
      for (p = section->Head; p->Next; p = p->Next); /* Find the tail */
      p->Next = pair;
    }
  }

  return pair;
}

int pspInitGetInt(const PspInit *init, const char *section, const char *key, int default_value)
{
  struct PspInitPair *pair = Locate(init, section, key);
  return (pair) ? atoi(pair->Value) : default_value;
}

char* pspInitGetString(const PspInit *init, const char *section, const char *key, const char *default_value)
{
  struct PspInitPair *pair = Locate(init, section, key);
  if (pair) return strdup(pair->Value);
  else if (default_value) return strdup(default_value);

  return NULL;
}

void pspInitSetInt(PspInit *init, const char *section, const char *key, int value)
{
  struct PspInitPair *pair;
  if (!(pair = LocateOrCreate(init, section, key))) return;

  /* Replace the value */
  if (pair->Value) free(pair->Value);
  char temp[64];
  sprintf(temp, "%i", value);
  pair->Value = strdup(temp);
}

void pspInitSetString(PspInit *init, const char *section, const char *key, const char *string)
{
  struct PspInitPair *pair;
  if (!(pair = LocateOrCreate(init, section, key))) return;

  /* Replace the value */
  if (pair->Value) free(pair->Value);
  pair->Value = strdup(string);
}

void pspInitDestroy(PspInit *init)
{
  struct PspInitSection *section, *next_section;
  struct PspInitPair *pair, *next_pair;

  for (section = init->Head; section; section = next_section)
  {
    next_section = section->Next;

    if (section->Name) free(section->Name);
    for (pair = section->Head; pair; pair = next_pair)
    {
      next_pair = pair->Next;
      if (pair->Key) free(pair->Key);
      if (pair->Value) free(pair->Value);

      free(pair);
    }

    free(section);
  }

  free(init);
}
