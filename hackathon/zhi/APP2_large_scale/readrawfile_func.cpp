#ifndef __READRAWFILE_FUNC_CPP__
#define __READRAWFILE_FUNC_CPP__

#include "readRawfile_func.h"

#include "v3d_message.h"
#include <vector>
#include <iostream>
#include "basic_surf_objs.h"



#define b_VERBOSE_PRINT 0
#define DEFINE_NBYTE2G \
  V3DLONG nBytes2G = (V3DLONG(1024)*V3DLONG(1024)*V3DLONG(1024)-1)*V3DLONG(2);

typedef int BIT32_UNIT;

char checkMachineEndian();
void swap2bytes(void *targetp);
void swap4bytes(void *targetp);


int loadRawRegion(char * filename, unsigned char * & img, V3DLONG * & sz, V3DLONG * & region_sz,int & datatype,
                  V3DLONG startx, V3DLONG starty, V3DLONG startz,
                  V3DLONG endx, V3DLONG endy, V3DLONG endz)
{
    FILE * fid = fopen(filename, "rb");

    if (!fid)
    {
        printf("Fail to open file for reading.\n");
        return 0;
    }

    fseek (fid, 0, SEEK_END);
    V3DLONG fileSize = ftell(fid);
    rewind(fid);
/*
#endif
*/
    /* Read header */

    char formatkey[] = "raw_image_stack_by_hpeng";
    V3DLONG lenkey = strlen(formatkey);


#ifndef _MSC_VER //added by PHC, 2010-05-21
    if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
    {
        printf("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.\n");
        printf("The fseek-ftell produces a file size = %ld.", fileSize);
        return 1;
    }
#endif

    char * keyread = new char [lenkey+1];
    if (!keyread)
    {
        printf("Fail to allocate memory.\n");

        return 0;
    }

    V3DLONG nread = fread(keyread, 1, lenkey, fid);
    if (nread!=lenkey) {
        printf("File unrecognized or corrupted file.\n");

        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    keyread[lenkey] = '\0';

    V3DLONG i;
    if (strcmp(formatkey, keyread)) /* is non-zero then the two strings are different */
    {
        printf("Unrecognized file format.\n");
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    char endianCodeData;
    int dummy = fread(&endianCodeData, 1, 1, fid);
    printf("The data endian code is [%c]\n", endianCodeData);
    if (endianCodeData!='B' && endianCodeData!='L')
    {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;    }

    char endianCodeMachine;
    endianCodeMachine = checkMachineEndian();
    printf("The machine endian code is [%c]\n", endianCodeMachine);
    if (endianCodeMachine!='B' && endianCodeMachine!='L')
    {
        printf("This program only supports big- or little- endian but not other format. Check your data endian.\n");
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
       }

    int b_swap = (endianCodeMachine==endianCodeData)?0:1;

    short int dcode = 0;
    dummy = fread(&dcode, 2, 1, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
    if (b_swap)
        swap2bytes((void *)&dcode);

    switch (dcode)
    {
        case 1:
            datatype = 1; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */
            break;

        case 2:
            datatype = 2;
            break;

        case 4:
            datatype = 4;
            break;

        default:
            if (keyread) {delete []keyread; keyread=0;}
            return  0;
    }

    V3DLONG unitSize = datatype; // temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future.

    BIT32_UNIT mysz[4];
    mysz[0]=mysz[1]=mysz[2]=mysz[3]=0;
    int tmpn=(int)fread(mysz, 4, 4, fid); // because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read.
    if (tmpn!=4) {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    if (b_swap)
    {
        for (i=0;i<4;i++)
        {
            //swap2bytes((void *)(mysz+i));
            if (b_VERBOSE_PRINT)
                printf("mysz raw read unit[%ld]: [%d] ", i, mysz[i]);
            swap4bytes((void *)(mysz+i));
            if (b_VERBOSE_PRINT)
                printf("swap unit: [%d][%0x] \n", mysz[i], mysz[i]);
        }
    }
    if (sz) {delete []sz; sz=0;}
    sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img.
    if (!sz)
    {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    V3DLONG totalUnit = 1;

    for (i=0;i<4;i++)
    {
        sz[i] = (V3DLONG)mysz[i];
        totalUnit *= sz[i];
    }

    V3DLONG tmpw = endx - startx;
    V3DLONG tmph = endy - starty;
    V3DLONG tmpz = endz - startz;

    V3DLONG head = 4*4+2+1+lenkey; // header_len ?
    V3DLONG pgsz1=sz[2]*sz[1]*sz[0], pgsz2=sz[1]*sz[0], pgsz3=sz[0];
    V3DLONG cn = tmpw*tmph*tmpz;
    V3DLONG kn = tmpw*tmph;
    V3DLONG total = tmpw*tmph*tmpz*sz[3];

    if (region_sz) {delete []region_sz; region_sz=0;}
    region_sz = new V3DLONG [4]; // reallocate the memory if the input parameter is non-null. Note that this requests the input is also an NULL point, the same to img.
    if (!region_sz)
    {
        if (keyread) {delete []keyread; keyread=0;}
        return 0;
    }

    region_sz[0] = tmpw;
    region_sz[1] = tmph;
    region_sz[2] = tmpz;
    region_sz[3] = sz[3];

    if (img) {delete []img; img=0;}
    V3DLONG totalBytes = V3DLONG(unitSize)*V3DLONG(total);
    try
    {
        img = new unsigned char [totalBytes];
    }
    catch (...)
    {
        if (keyread) {delete []keyread; keyread=0;}
        if (sz) {delete []sz; sz=0;}
        return 0;
    }

    //V3DLONG count=0; // unused
    V3DLONG c,j,k;
    for (c = 0; c < sz[3]; c++)
    {
        for (k = startz; k < endz; k++)
        {
            for (j = starty; j< endy; j++)
            {
                rewind(fid);
                fseek(fid, (long)(head+(c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
                int dummy = ftell(fid);
                dummy = fread(img+(c*cn+(k-startz)*kn + (j-starty)*tmpw)*unitSize,unitSize,tmpw,fid);
            }
        }
    }
    // swap the data bytes if necessary

    if (b_swap==1)
    {
        if (unitSize==2)
        {
            for (i=0;i<total; i++)
            {
                swap2bytes((void *)(img+i*unitSize));
            }
        }
        else if (unitSize==4)
        {
            for (i=0;i<total; i++)
            {
                swap4bytes((void *)(img+i*unitSize));
            }
        }
    }

    // clean and return

    if (keyread) {delete [] keyread; keyread = 0;}
    fclose(fid); //bug fix on 060412
    return 1;


}


char checkMachineEndian()
{
    char e='N'; //for unknown endianness

    V3DLONG int a=0x44332211;
    unsigned char * p = (unsigned char *)&a;
    if ((*p==0x11) && (*(p+1)==0x22) && (*(p+2)==0x33) && (*(p+3)==0x44))
        e = 'L';
    else if ((*p==0x44) && (*(p+1)==0x33) && (*(p+2)==0x22) && (*(p+3)==0x11))
        e = 'B';
    else if ((*p==0x22) && (*(p+1)==0x11) && (*(p+2)==0x44) && (*(p+3)==0x33))
        e = 'M';
    else
        e = 'N';

    //printf("[%c] \n", e);
    return e;
}

void swap2bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
}

void swap4bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+3);
    *(tp+3) = a;
    a = *(tp+1);
    *(tp+1) = *(tp+2);
    *(tp+2) = a;
}

#endif
