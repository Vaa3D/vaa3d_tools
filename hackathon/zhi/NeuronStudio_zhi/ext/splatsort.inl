/**********************************************************
    splatsort.h:
                Used for generating splat rendering arrays.


    Copyright CNIC, 2006
    Mount Sinani School of Medicine, New York, NY
    Last rev. date 1/23/2006

***********************************************************/
#ifndef SPLATSORT_H
#define SPLATSORT_H


typedef struct tagSPLATITEM
	{
	SPLATDEF def;
	struct tagSPLATITEM* next;
	}
	SPLATITEM;


typedef struct SPLATSORTER
	{
	 nsint        xdim;
	 nsint        ydim;
	 nsint        zdim;
	 nsint        count;
	 nsint        built;
	 SPLATITEM  **xlist;
	 SPLATITEM  **ylist;
	 SPLATITEM  **zlist;
	}
	SPLATSORTER;



SPLATSORTER* SplatSorterCreate(nsint xdim, nsint ydim, nsint zdim);

nsint SplatSorterInsert(SPLATSORTER* vs, SPLATDEF* def);

nsint SplatSorterBuild(SPLATSORTER* vs, SPLATDEF** pxlist,
                      SPLATDEF** pylist, SPLATDEF** pzlist);

void SplatSorterDestroy(SPLATSORTER* vs);


nssize SplatSorterSize( const SPLATSORTER *vs )
	{  return ( nssize )vs->count;  }


SPLATSORTER* SplatSorterCreate(nsint xdim, nsint ydim, nsint zdim)
{
    SPLATSORTER* vs;

    if(xdim<1) xdim=1;
    if(ydim<1) ydim=1;
    if(zdim<1) zdim=1;

    vs = (SPLATSORTER*) ns_malloc(sizeof(SPLATSORTER));
    if(vs==NULL) return NULL;

    vs->xlist = (SPLATITEM**) ns_calloc(xdim,sizeof(SPLATITEM*));
    vs->ylist = (SPLATITEM**) ns_calloc(ydim,sizeof(SPLATITEM*));
    vs->zlist = (SPLATITEM**) ns_calloc(zdim,sizeof(SPLATITEM*));

    if(vs->xlist==NULL||vs->ylist==NULL||vs->zlist==NULL)
    {
        ns_free(vs->xlist);
        ns_free(vs->ylist);
        ns_free(vs->zlist);
        ns_free(vs);
        return NULL;
    }
    vs->xdim = xdim;
    vs->ydim = ydim;
    vs->zdim = zdim;
    vs->count=0;
    vs->built=0;

    return vs;
}


nsint SplatSorterInsert(SPLATSORTER* vs, SPLATDEF* def)
{
    SPLATITEM* si;

    // prevent insertion to an already built list
    if(vs->built==1) return 0;

    //allocate a splat item to put in zlist
    si = (SPLATITEM*) ns_malloc(sizeof(SPLATITEM));
    if(si==NULL) return 0;

    // copy structure
    si->def = *def;

    // place in front of list
    si->next = vs->zlist[def->V.z];
    vs->zlist[def->V.z] = si;

    // increase counter
    vs->count++;

    return 1;
}


void SplatSorterDestroy(SPLATSORTER* vs)
{
    SPLATITEM **list;
    SPLATITEM *sihead,*sihold;
    nsint length,i;

    //destroy the lists. Notice that items endup in xlist after building.
    if(vs->built)
    {
        list = vs->xlist;
        length = vs->xdim;
    }
    else
    {
        list = vs->zlist;
        length = vs->zdim;
    }
    for(i=0;i<length;i++)
    {
        sihead = list[i];

        while(sihead!=NULL)
        {
            sihold = sihead->next;
            ns_free(sihead);
            sihead = sihold;
        }
        list[i] = NULL;
    }

    // destroy the lists
    ns_free(vs->xlist);
    ns_free(vs->ylist);
    ns_free(vs->zlist);

    // destroy structure
    ns_free(vs);


    return;
}


nsint SplatSorterBuild(SPLATSORTER* vs, SPLATDEF** pxlist,
                      SPLATDEF** pylist, SPLATDEF** pzlist)
{
    SPLATDEF *zlist,*ylist,*xlist;
    nsint i,pos;
    SPLATITEM *hold,*head;

    // make sure it doesn't build twice
    if(vs->built==1) return 0;

    // allocate all tree lists
    zlist = (SPLATDEF*) ns_malloc(sizeof(SPLATDEF)*vs->count);
    ylist = (SPLATDEF*) ns_malloc(sizeof(SPLATDEF)*vs->count);
    xlist = (SPLATDEF*) ns_malloc(sizeof(SPLATDEF)*vs->count);
    if(zlist==NULL||ylist==NULL||xlist==NULL)
    {
        ns_free(zlist);
        ns_free(ylist);
        ns_free(xlist);
        return 0;
    }


    //copy to z return array as items are moved from z to y list
    pos=0;
    for(i=0;i<vs->zdim;i++)
    {
        head = vs->zlist[i];

        while(head!=NULL)
        {
            hold = head->next;

            //copy to return array
            zlist[pos++] = head->def;

            // move to new location
            head->next = vs->ylist[head->def.V.y];
            vs->ylist[head->def.V.y] = head;

            head = hold;
        }

    }

    //copy to y return array as items are moved from y to x list
    pos=0;
    for(i=0;i<vs->ydim;i++)
    {
        head = vs->ylist[i];

        while(head!=NULL)
        {
            hold = head->next;

            //copy to return array
            ylist[pos++] = head->def;

            // move to new location
            head->next = vs->xlist[head->def.V.x];
            vs->xlist[head->def.V.x] = head;

            head = hold;
        }

    }

    // copy to x return array
    pos=0;
    for(i=0;i<vs->xdim;i++)
    {
        head = vs->xlist[i];

        while(head!=NULL)
        {
            //copy to return array
            xlist[pos++] = head->def;

            head = head->next;
        }
    }

    //set the pointers
    *pxlist = xlist;
    *pylist = ylist;
    *pzlist = zlist;

    vs->built=1;
    return 1;
}



#endif
