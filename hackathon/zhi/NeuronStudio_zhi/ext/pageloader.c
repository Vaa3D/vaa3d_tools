#include "pageloader.h"


#define STRIDE  3000



DWORD WINAPI _prefetchproc(LPVOID param)
{
    PAGELOADER *pl;
    const unsigned char *newend,*pos;
    int dummybuck=0;


    pl = param;

    while(pl->terminate==0)
    {
        if(pl->newaccess)
        {
            pl->newaccess = 0;
            newend = pl->start + pl->window;
            if(newend>(pl->data+pl->bytes-1)) newend=pl->data+pl->bytes-1;
            if(pl->end<((pl->start)-1)) pl->end = pl->start-1;

            for(pos=pl->end+1;pos<=newend;pos=pos+STRIDE)
            {
                dummybuck += (*pos);
            }
            pl->end = newend;
        }
        Sleep(0);
    }

    //return 0;
    ExitThread(0);
}


PAGELOADER* CreatePageLoader(const unsigned char* data, size_t bytes, size_t window)
{
    PAGELOADER *pl;

    
    // allocate memory
    pl = malloc(sizeof(PAGELOADER));
    if(pl==NULL) return NULL;

    //set members
    pl->data = data;
    pl->bytes = bytes;
    pl->window = window;
    pl->newaccess = 1; //force loading on creation
    pl->start = data;
    pl->end = NULL;
    pl->terminate = 0;


    //start paging thread
    pl->threadhandle = CreateThread(NULL,0,_prefetchproc,pl,0,NULL);
    if(pl->threadhandle==NULL)
    {
        free(pl);
        return NULL;
    }

    // wait for prefetch thread to get started 
    while(pl->newaccess==1)Sleep(0);

    return pl;
}



void DestroyPageLoader(PAGELOADER* pl)
{
    if(pl==NULL) return;
    pl->terminate=1;
    WaitForSingleObject(pl->threadhandle,INFINITE);
    free(pl);
    return;
}


void AnnounceAccess(PAGELOADER* pl, const unsigned char* address)
{
	if( NULL != pl )
	{
    pl->start = address;
    pl->newaccess = 1;

    // wait for prefetch thread to get started 
    while(pl->newaccess==1)Sleep(0);
	}

    return;
}
