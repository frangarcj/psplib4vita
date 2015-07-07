/** PSP helper library ***************************************/
/**                                                         **/
/**                        fileio.h                         **/
/**                                                         **/
/** This file contains declarations for the file management **/
/** routines                                                **/
/**                                                         **/
/** Copyright (C) Akop Karapetyan 2007                      **/
/**     You are not allowed to distribute this software     **/
/**     commercially. Please, notify me, if you make any    **/
/**     changes to this file.                               **/
/*************************************************************/
#ifndef _PSP_FILEIO_H
#define _PSP_FILEIO_H

#ifdef __cplusplus
extern "C" {
#endif

#define PSP_FILEIO_DIR          0x01
#define PSP_FILEIO_MAX_PATH_LEN 1024

struct PspFileIoFile
{
  char *Name;
  struct PspFileIoFile *Next;
  unsigned int Attrs;
};

typedef struct PspFileIoFile PspFile;

struct PspFileIoFileList
{
  int Count;
  struct PspFileIoFile *First;
  struct PspFileIoFile *Last;
};

typedef struct PspFileIoFileList PspFileList;

PspFileList* pspFileIoGetFileList(const char* path, const char **filter);
void         pspFileIoDestroyFileList(PspFileList* list);
void         pspFileIoEnterDirectory(char **cur_path, char *dir);
char*        pspFileIoGetParentDirectory(const char *path);
const char*  pspFileIoGetFilename(const char *path);
const char*  pspFileIoGetFileExtension(const char *path);
int          pspFileIoIsRootDirectory(const char *path);
int          pspFileIoCheckIfExists(const char *path);
int          pspFileIoEndsWith(const char *filename, const char *ext);
int          pspFileIoDelete(const char *path);

#ifdef __cplusplus
}
#endif

#endif // _PSP_FILEIO_H
