#include "mc.h"


typedef struct
{
    int value;
    float x,y,z;
} MCCORNER;

typedef struct
{
    MCCORNER a,b,c,d,e,f,g,h;
} MCCUBE;

int mcpolys[23] =           {0,1,2,4,3,2,6,5,5,5,4,4,4,3,4,3,2,4,3,2,2,1,0};

unsigned char mcgold[23] =  {0,1,3,5,7,15,20,21,23,26,27,29,30,31,60,61,63,
                             90,91,95,125,127,255};

int mcindex[256] =          {0,1,1,2,1,3,2,4,1,2,3,4,2,4,4,5,1,2,3,4,6,7,7,
                             8,3,4,9,10,7,11,12,13,1,3,2,4,3,9,4,10,6,7,7,
                             11,7,12,8,13,2,4,4,5,7,12,11,13,7,8,12,13,14,
                             15,15,16,1,6,3,7,2,7,4,11,3,7,9,12,4,8,10,13,
                             3,7,9,12,7,14,12,15,9,12,17,18,12,15,18,19,2,
                             7,4,8,4,12,5,13,7,14,12,15,11,15,13,16,4,11,
                             10,13,8,15,13,16,12,15,18,19,15,20,19,21,1,3,
                             6,7,3,9,7,12,2,4,7,8,4,10,11,13,2,4,7,11,7,12,
                             14,15,4,5,12,13,8,13,15,16,3,9,7,12,9,17,12,
                             18,7,12,14,15,12,18,15,19,4,10,8,13,12,18,15,
                             19,11,13,15,16,15,19,20,21,2,7,7,14,4,12,8,15,
                             4,11,12,15,5,13,13,16,4,8,12,15,11,15,15,20,
                             10,13,18,19,13,16,19,21,4,12,11,15,10,18,13,
                             19,8,15,15,20,13,19,16,21,5,13,13,16,13,19,16,
                             21,13,16,19,21,16,21,21,22};

int mcrotat[256] =          {0,0,3,0,2,0,3,0,1,1,1,1,2,2,3,0,5,17,5,13,0,0,
                             13,0,17,17,0,0,22,0,0,0,4,18,18,18,4,3,14,3,3,
                             21,3,3,14,3,3,3,5,21,5,5,4,13,4,13,17,17,21,
                             21,0,0,13,0,7,2,19,20,19,2,19,2,7,15,2,2,15,2,
                             2,2,9,11,4,18,9,16,4,20,6,6,0,0,16,22,2,0,4,
                             18,20,18,4,20,4,20,7,3,14,3,7,14,14,3,9,11,4,
                             18,4,18,4,18,10,11,4,18,7,0,4,0,6,6,1,12,16,1,
                             23,1,16,12,1,1,16,1,1,1,6,6,5,5,16,22,1,12,22,
                             6,12,12,16,22,1,1,8,5,8,5,7,1,19,3,10,17,17,
                             21,7,1,23,1,8,5,5,5,9,5,4,5,10,17,17,17,10,17,
                             1,1,7,6,19,2,23,15,19,2,7,6,23,15,7,15,23,2,
                             11,6,8,5,9,16,9,2,6,6,6,6,16,16,16,2,10,11,8,
                             8,7,7,19,19,7,6,19,3,7,7,19,3,8,11,8,5,9,9,4,
                             5,10,6,8,6,7,7,4,0};



void    MCGetRotatedCube(MCCUBE* pdest,MCCUBE* psrc,int index);
void    MCInterpolate(MCCORNER* c1,MCCORNER* c2,MCPOINT* dest,int threshold);
int     MCTestTriangle(MCPOINT *p1,MCPOINT *p2, MCPOINT *p3);

#define TESTANDWRITE(a,b,c)  \
if(MCTestTriangle(&a,&b,&c)) \
{                            \
    ppoly->p1=a;             \
    ppoly->p2=b;             \
    ppoly->p3=c;             \
    ppoly++;                 \
    polycount++;             \
}                         



/*********************************************************
*   MCGetStrip():
*   This is the main routine. It returns a list of the
*   polygons found between the two layers of data.
*   It is up the caller to ns_free the returned structure
*   by calling MCFreeStrip() once done with it.
*
*       
*               e-----f    e,f,g,h are points in data2
*              /|    /| 
*             a-----b |  a,b,c,d are points in data1
*             | h---|-g
*             |/    |/
*             d-----c  
*
**********************************************************/

MCSTRIP* MCGetStrip(unsigned char* data1, unsigned char* data2,
                    int nx, int ny, int threshold,
                    float x0,float y0,float z0,
                    float xstep, float ystep, float zstep)
{
    unsigned char imask[] = {1,2,4,8,16,32,64,128};
    static unsigned char* indexbuff=NULL;
    static int indexcount=0;
    unsigned char *pindex;
    int i,j,equiv,polycount;
    unsigned char *p1,*p2,*pa,*pb,*pc,*pd,*pe,*pf,*pg,*ph;
    MCSTRIP* strip;
    MCPOLY *ppoly;
    MCCUBE cube,rotated;
    MCPOINT ab,bc,cd,da,ef,fg,gh,he,ae,bf,cg,dh;

    /* make sure index buffer is large enough */
    if(  indexcount  <  ((nx-1)*(ny-1))  )
    {
        ns_free(indexbuff);
        indexcount = (nx-1)*(ny-1);
        indexbuff = (unsigned char*) ns_malloc(sizeof(char)*indexcount);
        if(indexbuff==NULL) return NULL;
    }

    /* calc index for each cube in data and estimate polygon count */
    polycount=0;
    p1 = data1;
    p2 = data2;
    pindex = indexbuff;
    for(j=1;j<ny;j++)
    {
        pa = p1;
        pb = p1+1;
        pc = p1+nx+1;
        pd = p1+nx;
        pe = p2;
        pf = p2+1;
        pg = p2+nx+1;
        ph = p2+nx;
        for(i=1;i<nx;i++)
        {
            /* make index */
            *pindex=0;
            if(*pa>=threshold) *pindex |= imask[7];
            if(*pb>=threshold) *pindex |= imask[6];
            if(*pc>=threshold) *pindex |= imask[5];
            if(*pd>=threshold) *pindex |= imask[4];
            if(*pe>=threshold) *pindex |= imask[3];
            if(*pf>=threshold) *pindex |= imask[2];
            if(*pg>=threshold) *pindex |= imask[1];
            if(*ph>=threshold) *pindex |= imask[0];

            if( *pindex>0&&*pindex<255 )
            {
                /* get equivalent golden cube */
                equiv = mcindex[*pindex];

                /* add polygon count */
                polycount += mcpolys[equiv];
            }

            /* update pointers */
            pa++;pb++;pc++;pd++;pe++;pf++;pg++;ph++;
            pindex++;
        }
        p1+=nx;
        p2+=nx;
    }

    /* allocate memory, if unable return NULL */
    strip = (MCSTRIP*) ns_malloc(sizeof(MCSTRIP));
    if(strip==NULL) return NULL;
    strip->precount = polycount;
    strip->polys = (MCPOLY*) ns_malloc(polycount*sizeof(MCPOLY));
    if(strip->polys==NULL)
    {
        ns_free(strip);
        return NULL;
    }

    /* generate the actual polygons */
    polycount = 0;
    p1 = data1;
    p2 = data2;
    pindex = indexbuff;
    ppoly = strip->polys;
    for(j=1;j<ny;j++)
    {
        pa = p1;
        pb = p1+1;
        pc = p1+nx+1;
        pd = p1+nx;
        pe = p2;
        pf = p2+1;
        pg = p2+nx+1;
        ph = p2+nx;
        for(i=1;i<nx;i++)
        {
            /* avoid cases with no polygons */
            if(*pindex>0 && *pindex<255)
            {
                /***********************************
                *     set up the eight corners
                ************************************/
                cube.a.value = *pa;
                cube.a.x = x0+(i-1)*xstep;
                cube.a.y = -y0-(j-1)*ystep;
                cube.a.z = -z0;

                cube.b.value = *pb;
                cube.b.x = x0+i*xstep;
                cube.b.y = -y0-(j-1)*ystep;
                cube.b.z = -z0;

                cube.c.value = *pc;
                cube.c.x = x0+i*xstep;
                cube.c.y = -y0-j*ystep;
                cube.c.z = -z0;

                cube.d.value = *pd;
                cube.d.x = x0+(i-1)*xstep;
                cube.d.y = -y0-j*ystep;
                cube.d.z = -z0;

                cube.e.value = *pe;
                cube.e.x = x0+(i-1)*xstep;
                cube.e.y = -y0-(j-1)*ystep;
                cube.e.z = -z0-zstep;

                cube.f.value = *pf;
                cube.f.x = x0+i*xstep;
                cube.f.y = -y0-(j-1)*ystep;
                cube.f.z = -z0-zstep;

                cube.g.value = *pg;
                cube.g.x = x0+i*xstep;
                cube.g.y = -y0-j*ystep;
                cube.g.z = -z0-zstep;

                cube.h.value = *ph;
                cube.h.x = x0+(i-1)*xstep;
                cube.h.y = -y0-j*ystep;
                cube.h.z = -z0-zstep;

                /* rotate cube to canonical orientation */
                MCGetRotatedCube(&rotated,&cube,*pindex);

                /**********************************************
                *   generate polygons depending on wich golden
                ***********************************************/
                equiv = mcindex[*pindex];
                switch(equiv)
                {
                    case 1:
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);

                        TESTANDWRITE(he,dh,gh)

                        break;

                    case 2:
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);

                        TESTANDWRITE(he,dh,cg)
                        TESTANDWRITE(he,cg,fg)

                        break;

                    case 3:
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);

                        TESTANDWRITE(he,dh,bf)
                        TESTANDWRITE(ef,he,bf)
                        TESTANDWRITE(bf,dh,gh)
                        TESTANDWRITE(bf,gh,fg)
                        
                        break;

                    case 4:
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);

                        TESTANDWRITE(he,dh,cg)
                        TESTANDWRITE(ef,he,cg)
                        TESTANDWRITE(bf,ef,cg)
                        
                        break;

                    case 5:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);

                        TESTANDWRITE(ae,dh,cg)
                        TESTANDWRITE(bf,ae,cg)

                        break;
                    case 6:
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);

                        TESTANDWRITE(ef,da,bf)
                        TESTANDWRITE(dh,da,ef)
                        TESTANDWRITE(fg,dh,ef)
                        TESTANDWRITE(fg,cd,dh)
                        TESTANDWRITE(bf,cd,fg)
                        TESTANDWRITE(bf,da,cd)

                        break;

                    case 7:
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);

                        TESTANDWRITE(he,da,bf)
                        TESTANDWRITE(ef,he,bf)
                        TESTANDWRITE(bf,da,cd)
                        TESTANDWRITE(bf,cd,gh)
                        TESTANDWRITE(bf,gh,fg)

                        break;
                    case 8:

                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);

                        TESTANDWRITE(he,da,bf)
                        TESTANDWRITE(ef,he,bf)
                        TESTANDWRITE(bf,da,cd)
                        TESTANDWRITE(bf,cd,cg)

                        break;

                    case 9:
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);

                        TESTANDWRITE(he,gh,dh)
                        TESTANDWRITE(ae,da,ef)
                        TESTANDWRITE(da,cd,cg)
                        TESTANDWRITE(ef,cg,fg)
                        TESTANDWRITE(ef,da,cg)
                        
                        break;

                    case 10:
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);

                        TESTANDWRITE(ae,da,ef)
                        TESTANDWRITE(da,cd,cg)
                        TESTANDWRITE(ef,cg,fg)
                        TESTANDWRITE(ef,da,cg)

                        break;

                    case 11:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);

                        TESTANDWRITE(fg,bf,gh)
                        TESTANDWRITE(bf,ae,gh)
                        TESTANDWRITE(ae,da,gh)
                        TESTANDWRITE(da,cd,gh)

                        break;

                    case 12:
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);

                        TESTANDWRITE(he,gh,dh)
                        TESTANDWRITE(ae,da,bf)
                        TESTANDWRITE(bf,da,cd)
                        TESTANDWRITE(bf,cd,cg)

                        break;

                    case 13:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);

                        TESTANDWRITE(ae,da,bf)
                        TESTANDWRITE(bf,da,cd)
                        TESTANDWRITE(bf,cd,cg)

                        break;

                    case 14:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate bc */
                        MCInterpolate(&rotated.b,&rotated.c,&bc,threshold);
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);

                        TESTANDWRITE(ae,da,bc)
                        TESTANDWRITE(bf,ae,bc)
                        TESTANDWRITE(he,fg,cg)
                        TESTANDWRITE(he,cg,dh)
                        
                        break;
                    case 15:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate bc */
                        MCInterpolate(&rotated.b,&rotated.c,&bc,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);

                        TESTANDWRITE(ae,da,bc)
                        TESTANDWRITE(bf,ae,bc)
                        TESTANDWRITE(fg,cg,gh)

                        break;

                    case 16:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate bc */
                        MCInterpolate(&rotated.b,&rotated.c,&bc,threshold);

                        TESTANDWRITE(ae,da,bc)
                        TESTANDWRITE(bf,ae,bc)
                        
                        break;

                    case 17:
                        /* interpolate ab */
                        MCInterpolate(&rotated.a,&rotated.b,&ab,threshold);
                        /* interpolate bc */
                        MCInterpolate(&rotated.b,&rotated.c,&bc,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);
                        /* interpolate he */
                        MCInterpolate(&rotated.h,&rotated.e,&he,threshold);
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate dh */
                        MCInterpolate(&rotated.d,&rotated.h,&dh,threshold);

                        TESTANDWRITE(he,gh,dh)
                        TESTANDWRITE(ae,da,ab)
                        TESTANDWRITE(bc,cd,cg)
                        TESTANDWRITE(ef,bf,fg)

                        break;

                    case 18:
                        /* interpolate ab */
                        MCInterpolate(&rotated.a,&rotated.b,&ab,threshold);
                        /* interpolate bc */
                        MCInterpolate(&rotated.b,&rotated.c,&bc,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate ef */
                        MCInterpolate(&rotated.e,&rotated.f,&ef,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate bf */
                        MCInterpolate(&rotated.b,&rotated.f,&bf,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);

                        TESTANDWRITE(ae,da,ab)
                        TESTANDWRITE(bc,cd,cg)
                        TESTANDWRITE(ef,bf,fg)

                        break;

                    case 19:
                        /* interpolate ab */
                        MCInterpolate(&rotated.a,&rotated.b,&ab,threshold);
                        /* interpolate bc */
                        MCInterpolate(&rotated.b,&rotated.c,&bc,threshold);
                        /* interpolate cd */
                        MCInterpolate(&rotated.c,&rotated.d,&cd,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);

                        TESTANDWRITE(ae,da,ab)
                        TESTANDWRITE(bc,cd,cg)

                        break;

                    case 20:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate ab */
                        MCInterpolate(&rotated.a,&rotated.b,&ab,threshold);
                        /* interpolate fg */
                        MCInterpolate(&rotated.f,&rotated.g,&fg,threshold);
                        /* interpolate cg */
                        MCInterpolate(&rotated.c,&rotated.g,&cg,threshold);
                        /* interpolate gh */
                        MCInterpolate(&rotated.g,&rotated.h,&gh,threshold);

                        TESTANDWRITE(fg,cg,gh)
                        TESTANDWRITE(ae,da,ab)

                        break;

                    case 21:
                        /* interpolate ae */
                        MCInterpolate(&rotated.a,&rotated.e,&ae,threshold);
                        /* interpolate da */
                        MCInterpolate(&rotated.d,&rotated.a,&da,threshold);
                        /* interpolate ab */
                        MCInterpolate(&rotated.a,&rotated.b,&ab,threshold);

                        TESTANDWRITE(ae,da,ab)

                        break;
                }
            }
            pa++;pb++;pc++;pd++;pe++;pf++;pg++;ph++;
            pindex++;
        }
        p1+=nx;
        p2+=nx;
    }
    strip->postcount = polycount;


    return strip;
}

void MCFreeStrip(MCSTRIP* strip)
{
    ns_free(strip->polys);
    ns_free(strip);

    return;
}

/* macro to compute intersection in one direction by interpolation */
#define FB(a,fa,c,fc,b) ((b-a)*(fc-fa)/(c-a)+fa)


void MCInterpolate(MCCORNER* c1, MCCORNER* c2,MCPOINT* dest,int threshold)
{
    /* first handle cases where threshold is on corner */
    if(c1->value==threshold)
    {
        /* compute coordinates by avoiding corners */
        dest->x = 0.9f*c1->x+0.1f*c2->x;
        dest->y = 0.9f*c1->y+0.1f*c2->y;
        dest->z = 0.9f*c1->z+0.1f*c2->z;
    }
    else if(c2->value==threshold)
    {
        /* compute coordinates by avoiding corners */
        dest->x = 0.9f*c2->x+0.1f*c1->x;
        dest->y = 0.9f*c2->y+0.1f*c1->y;
        dest->z = 0.9f*c2->z+0.1f*c1->z;
    }
    else
    {
        /* interpolate x */
        if(c1->x==c2->x) dest->x = c1->x;
        else dest->x = FB(c1->value,c1->x,c2->value,c2->x,threshold);

        /* interpolate y */
        if(c1->y==c2->y) dest->y = c1->y;
        else dest->y = FB(c1->value,c1->y,c2->value,c2->y,threshold);

        /* interpolate z */
        if(c1->z==c2->z) dest->z = c1->z;
        else dest->z = FB(c1->value,c1->z,c2->value,c2->z,threshold);
    }

    return;
}
                                   
/* this routine uses a dot product to figure out if triagle is valid */
int     MCTestTriangle(MCPOINT *p1,MCPOINT *p2, MCPOINT *p3)
{
    static float distance;
    static float v1[3],v2[3];
    static float cpx,cpy,cpz;

    /* calculate two vectors, one from p1 to p2 and one from p1 tp p3 */
    v1[0] = p2->x - p1->x;  /* x */
    v1[1] = p2->y - p1->y;  /* y */
    v1[2] = p2->z - p1->z;  /* z */

    v2[0] = p3->x - p1->x;  /* x */
    v2[1] = p3->y - p1->y;  /* y */
    v2[2] = p3->z - p1->z;  /* z */

    cpx = v1[1]*v2[2] - v1[2]*v2[1];
    cpy = v1[2]*v2[0] - v1[0]*v2[2];
    cpz = v1[0]*v2[1] - v1[1]*v2[0];

    /* return 0 to indicate invalid polygon */
    if(cpx==0.0f&&cpy==0.0f&&cpz==0.0f) return 0;

    return 1;
}

void    MCGetRotatedCube(MCCUBE* pdest,MCCUBE* psrc,int index)
{
    /********************************************
    *   swap corners depending on rotation
    *********************************************/
    switch(mcrotat[index])
    {
        case 0:
            /* do the identity here */
            pdest->a = psrc->a;
            pdest->b = psrc->b;
            pdest->c = psrc->c;
            pdest->d = psrc->d;
            pdest->e = psrc->e;
            pdest->f = psrc->f;
            pdest->g = psrc->g;
            pdest->h = psrc->h;
            break;
        case 1:
            pdest->a = psrc->b;
            pdest->b = psrc->c;
            pdest->c = psrc->d;
            pdest->d = psrc->a;
            pdest->e = psrc->f;
            pdest->f = psrc->g;
            pdest->g = psrc->h;
            pdest->h = psrc->e;
            break;
        case 2:
            pdest->a = psrc->c;
            pdest->b = psrc->d;
            pdest->c = psrc->a;
            pdest->d = psrc->b;
            pdest->e = psrc->g;
            pdest->f = psrc->h;
            pdest->g = psrc->e;
            pdest->h = psrc->f;
            break;
        case 3:
            pdest->a = psrc->d;
            pdest->b = psrc->a;
            pdest->c = psrc->b;
            pdest->d = psrc->c;
            pdest->e = psrc->h;
            pdest->f = psrc->e;
            pdest->g = psrc->f;
            pdest->h = psrc->g;
            break;
        case 4:
            pdest->a = psrc->h;
            pdest->b = psrc->e;
            pdest->c = psrc->a;
            pdest->d = psrc->d;
            pdest->e = psrc->g;
            pdest->f = psrc->f;
            pdest->g = psrc->b;
            pdest->h = psrc->c;
            break;
        case 5:
            pdest->a = psrc->e;
            pdest->b = psrc->f;
            pdest->c = psrc->b;
            pdest->d = psrc->a;
            pdest->e = psrc->h;
            pdest->f = psrc->g;
            pdest->g = psrc->c;
            pdest->h = psrc->d;
            break;
        case 6:
            pdest->a = psrc->f;
            pdest->b = psrc->g;
            pdest->c = psrc->c;
            pdest->d = psrc->b;
            pdest->e = psrc->e;
            pdest->f = psrc->h;
            pdest->g = psrc->d;
            pdest->h = psrc->a;
            break;
        case 7:
            pdest->a = psrc->g;
            pdest->b = psrc->h;
            pdest->c = psrc->d;
            pdest->d = psrc->c;
            pdest->e = psrc->f;
            pdest->f = psrc->e;
            pdest->g = psrc->a;
            pdest->h = psrc->b;
            break;
        case 8:
            pdest->a = psrc->f;
            pdest->b = psrc->e;
            pdest->c = psrc->h;
            pdest->d = psrc->g;
            pdest->e = psrc->b;
            pdest->f = psrc->a;
            pdest->g = psrc->d;
            pdest->h = psrc->c;
            break;
        case 9:
            pdest->a = psrc->e;
            pdest->b = psrc->h;
            pdest->c = psrc->g;
            pdest->d = psrc->f;
            pdest->e = psrc->a;
            pdest->f = psrc->d;
            pdest->g = psrc->c;
            pdest->h = psrc->b;
            break;
        case 10:
            pdest->a = psrc->h;
            pdest->b = psrc->g;
            pdest->c = psrc->f;
            pdest->d = psrc->e;
            pdest->e = psrc->d;
            pdest->f = psrc->c;
            pdest->g = psrc->b;
            pdest->h = psrc->a;
            break;
        case 11:
            pdest->a = psrc->g;
            pdest->b = psrc->f;
            pdest->c = psrc->e;
            pdest->d = psrc->h;
            pdest->e = psrc->c;
            pdest->f = psrc->b;
            pdest->g = psrc->a;
            pdest->h = psrc->d;
            break;
        case 12:
            pdest->a = psrc->c;
            pdest->b = psrc->b;
            pdest->c = psrc->f;
            pdest->d = psrc->g;
            pdest->e = psrc->d;
            pdest->f = psrc->a;
            pdest->g = psrc->e;
            pdest->h = psrc->h;
            break;
        case 13:
            pdest->a = psrc->b;
            pdest->b = psrc->a;
            pdest->c = psrc->e;
            pdest->d = psrc->f;
            pdest->e = psrc->c;
            pdest->f = psrc->d;
            pdest->g = psrc->h;
            pdest->h = psrc->g;
            break;
        case 14:
            pdest->a = psrc->a;
            pdest->b = psrc->d;
            pdest->c = psrc->h;
            pdest->d = psrc->e;
            pdest->e = psrc->b;
            pdest->f = psrc->c;
            pdest->g = psrc->g;
            pdest->h = psrc->f;
            break;
        case 15:
            pdest->a = psrc->d;
            pdest->b = psrc->c;
            pdest->c = psrc->g;
            pdest->d = psrc->h;
            pdest->e = psrc->a;
            pdest->f = psrc->b;
            pdest->g = psrc->f;
            pdest->h = psrc->e;
            break;
        case 16:
            pdest->a = psrc->c;
            pdest->b = psrc->g;
            pdest->c = psrc->h;
            pdest->d = psrc->d;
            pdest->e = psrc->b;
            pdest->f = psrc->f;
            pdest->g = psrc->e;
            pdest->h = psrc->a;
            break;
        case 17:
            pdest->a = psrc->b;
            pdest->b = psrc->f;
            pdest->c = psrc->g;
            pdest->d = psrc->c;
            pdest->e = psrc->a;
            pdest->f = psrc->e;
            pdest->g = psrc->h;
            pdest->h = psrc->d;
            break;
        case 18:
            pdest->a = psrc->a;
            pdest->b = psrc->e;
            pdest->c = psrc->f;
            pdest->d = psrc->b;
            pdest->e = psrc->d;
            pdest->f = psrc->h;
            pdest->g = psrc->g;
            pdest->h = psrc->c;
            break;
        case 19:
            pdest->a = psrc->d;
            pdest->b = psrc->h;
            pdest->c = psrc->e;
            pdest->d = psrc->a;
            pdest->e = psrc->c;
            pdest->f = psrc->g;
            pdest->g = psrc->f;
            pdest->h = psrc->b;
            break;
        case 20:
            pdest->a = psrc->e;
            pdest->b = psrc->a;
            pdest->c = psrc->d;
            pdest->d = psrc->h;
            pdest->e = psrc->f;
            pdest->f = psrc->b;
            pdest->g = psrc->c;
            pdest->h = psrc->g;
            break;
        case 21:
            pdest->a = psrc->f;
            pdest->b = psrc->b;
            pdest->c = psrc->a;
            pdest->d = psrc->e;
            pdest->e = psrc->g;
            pdest->f = psrc->c;
            pdest->g = psrc->d;
            pdest->h = psrc->h;
            break;
        case 22:
            pdest->a = psrc->g;
            pdest->b = psrc->c;
            pdest->c = psrc->b;
            pdest->d = psrc->f;
            pdest->e = psrc->h;
            pdest->f = psrc->d;
            pdest->g = psrc->a;
            pdest->h = psrc->e;
            break;
        case 23:
            pdest->a = psrc->h;
            pdest->b = psrc->d;
            pdest->c = psrc->c;
            pdest->d = psrc->g;
            pdest->e = psrc->e;
            pdest->f = psrc->a;
            pdest->g = psrc->b;
            pdest->h = psrc->f;
            break;
    }
    return;
}

