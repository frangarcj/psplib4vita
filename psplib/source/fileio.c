/** PSP helper library ***************************************/
/**                                                         **/
/**                        fileio.c                         **/
/**                                                         **/
/** This file contains file management routines             **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#include <psp2/types.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/dirent.h>
#include <psp2/io/stat.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include "fileio.h"

static int _pspFileIoCompareFilesByName(const void *s1, const void *s2);
static void _pspFileIoSortFileList(PspFileList *list);

// Returns a list of all files in a directory
// path: path containing the files
// filter: array of strings, each containing an extension. last element contains NULL ('\0')

PspFileList* pspFileIoGetFileList(const char *path, const char **filter)
{
  SceUID fd = sceIoDopen(path);

  if (fd < 0) return 0;

  SceIoDirent dir;
  memset(&dir, 0, sizeof(dir));

  PspFile *file = 0;
  PspFileList *list = (PspFileList*)malloc(sizeof(PspFileList));
  const char **pext;
  int loop;

  list->Count = 0;
  list->First = 0;
  list->Last = 0;

  while (sceIoDread(fd, &dir) > 0)
  {
    if (filter && !(PSP2_S_ISDIR(dir.d_stat.st_attr)))
    {
      /* Loop through the list of allowed extensions and compare */
      for (pext = filter, loop = 1; *pext; pext++)
      {
        if (pspFileIoEndsWith(dir.d_name, *pext))
        {
          loop = 0;
          break;
        }
      }

      if (loop) continue;
    }

    // Create a new file entry

    file = (PspFile*)malloc(sizeof(PspFile));
    file->Name = strdup(dir.d_name);
    file->Next = 0;
    file->Attrs = (PSP2_S_ISDIR(dir.d_stat.st_attr)) ? PSP_FILEIO_DIR : 0;

    list->Count++;

    // Update preceding element

    if (list->Last)
    {
      list->Last->Next = file;
      list->Last = file;
    }
    else
    {
      list->First = file;
      list->Last = file;
    }
  }

  sceIoDclose(fd);

  // Sort the files by name

  _pspFileIoSortFileList(list);

  return list;
}

static int _pspFileIoCompareFilesByName(const void *s1, const void *s2)
{
  PspFile *f1=*(PspFile**)s1, *f2=*(PspFile**)s2;
  if ((f1->Attrs&PSP_FILEIO_DIR)==(f2->Attrs&PSP_FILEIO_DIR))
    return strcasecmp(f1->Name, f2->Name);
  else if (f1->Attrs&PSP_FILEIO_DIR)
    return -1;
  else return 1;
}

static void _pspFileIoSortFileList(PspFileList *list)
{
  PspFile **files, *file;
  int i;

  if (list->Count < 1) return;

  /* Copy the file entries to an array */
  files = (PspFile**)malloc(sizeof(PspFile*) * list->Count);
  for (file = list->First, i = 0; file; file = file->Next, i++)
    files[i] = file;

  /* Sort the array */
  qsort((void*)files, list->Count, sizeof(PspFile*), _pspFileIoCompareFilesByName);

  /* Rearrange the file entries in the list */
  list->First = files[0];
  list->First->Next = NULL;

  for (i = 1; i < list->Count; i++)
    files[i - 1]->Next = files[i];

  list->Last = files[list->Count - 1];
  list->Last->Next = NULL;

  /* Free the temp. array */
  free(files);
}

void pspFileIoDestroyFileList(PspFileList* list)
{
  PspFile *file, *next;

  for (file = list->First; file; file = next)
  {
    next = file->Next;
    free(file->Name);
    free(file);
  }

  free(list);
}

void pspFileIoEnterDirectory(char **cur_path, char *dir)
{
  /* Same directory */
  if (strcmp(dir, ".") == 0)
    return;

  int len, pos;
  len = strlen(*cur_path);
  pos = len - 1;

  /* Ascend one level */
  if (strcmp(dir, "..") == 0)
  {
    for (; pos >= 0 && (*cur_path)[pos] == '/'; pos--);
    for (; pos >= 0 && (*cur_path)[pos] != '/'; pos--);

    if (pos >= 0)
      (*cur_path)[pos + 1] = '\0';

    return;
  }

  /* Descend one level */
  char *new_path = (char*)malloc(sizeof(char) * (strlen(dir) + len + 2)); // 2: '\0' + '/'
  sprintf(new_path, "%s%s/", *cur_path, dir);

  free(*cur_path);
  *cur_path = new_path;
}

char* pspFileIoGetParentDirectory(const char *path)
{
  char *pos = strrchr(path, '/');

  if (!pos) return NULL;

  char *parent = (char*)malloc(sizeof(char) * (pos - path + 2));
  strncpy(parent, path, pos - path + 1);
  parent[pos - path + 1] = '\0';

  return parent;
}

const char* pspFileIoGetFilename(const char *path)
{
  char *pos;
  if (!(pos = strrchr(path, '/')))
    return path;
  return pos + 1;
}

int pspFileIoIsRootDirectory(const char *path)
{
  char *pos = strchr(path, '/');
  return !(pos && (*(pos + 1)));
}

const char* pspFileIoGetFileExtension(const char *path)
{
  const char *filename = strrchr(path, '/');

  filename = (filename) ? filename + 1 : path;

  const char *ext = strrchr(filename, '.');

  return (ext) ? ext + 1 : filename + strlen(filename);
}

int pspFileIoEndsWith(const char *filename, const char *ext)
{
  int fn_len, ext_len;
  const char *file_ext;

  fn_len = strlen(filename);
  ext_len = strlen(ext);

  /* Filename must be at least 2 chars longer (period + a char) */
  if (fn_len < ext_len + 2)
    return 0;

  file_ext = filename + (fn_len - ext_len);
  if (*(file_ext - 1) == '.' && strcasecmp(file_ext, ext) == 0)
    return 1;

  return 0;
}

int pspFileIoCheckIfExists(const char *path)
{
  SceIoStat stat;
  memset(&stat, 0, sizeof(stat));

  if (sceIoGetstat(path, &stat) < 0)
    return 0;

  return 1;
}

int pspFileIoDelete(const char *path)
{
  return sceIoRemove(path) >= 0;
}
