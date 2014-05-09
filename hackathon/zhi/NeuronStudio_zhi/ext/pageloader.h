/***************************************************************
    Pager.h: Experimental module to test if forcing reading of
             pages ahead could help performance.

****************************************************************/
#include <stdlib.h>
#include <windows.h>


#ifndef __PAGE_LOADER_H__
#define __PAGE_LOADER_H__


typedef struct tagPAGELOADER
{
    const unsigned char* data;
    size_t bytes;
    size_t window;
    int newaccess;
    const unsigned char* start;
    const unsigned char* end;
    int terminate;
    HANDLE threadhandle;
} PAGELOADER;


PAGELOADER* CreatePageLoader(const unsigned char* data, size_t bytes, size_t window);

void DestroyPageLoader(PAGELOADER* pl);

void AnnounceAccess(PAGELOADER* pl, const unsigned char* address);


#endif/* __PAGE_LOADER_H__ */
