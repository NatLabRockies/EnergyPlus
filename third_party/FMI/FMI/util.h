///////////////////////////////////////////////////////
/// \file   util.h
///
/// \brief  utility functions
///
/// \author Wangda Zuo, Thierry S. Nouidui, Michael Wetter
///         Simulation Research Group,
///         LBNL,
///         WZuo@lbl.gov
///
/// \date   2011-10-10
///
///
/// This deader file defines functions in util.c
///
///////////////////////////////////////////////////////

#ifdef _WIN32
#include <windows.h>
#define IS_WINDOWS 1
#else
#define IS_WINDOWS 0
#define HANDLE void *
/* See http://www.yolinux.com/TUTORIALS/LibraryArchives-StaticAndDynamic.html */
#include <sys/stat.h> // for creating dirs on Linux
#include <stddef.h>   // defines size_t
#endif

// static char* getfmuPat(const char* fmuFilNam);
int delete (char *tmpPat);

void doubleToCommaString(char *buffer, double r);

char *getTmpPath(const char *nam, size_t length);

void printDebug(const char *msg);

void printfDebug(const char *str1, const char *str2);

void printfIntDebug(const char *str1, const int integer);

void printError(const char *msg);

void printfError(const char *str1, const char *str2);

void setDebug(void);

int unpack(const char *fmuFilNam, const char *tmpPat);

int unpackminizip(const char *fmuFilNam, char *tmpPat);
