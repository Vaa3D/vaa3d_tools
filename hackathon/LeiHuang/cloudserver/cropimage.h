#ifndef CROPIMAGE_H
#define CROPIMAGE_H
#include <QString>
#include <iostream>
#include "publicEnum.h"

#define b_VERBOSE_PRINT 0
typedef  long long V3DLONG;
typedef int BIT32_UNIT;

char checkMachineEndian()
{
    char e='N'; //for unknown endianness

    V3DLONG a=0x44332211; // eliminated 'int'
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

#if defined (Q_OS_WIN32)
    _fseeki64(fid, 0, SEEK_END);
    V3DLONG fileSize = _ftelli64(fid);
#endif

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
    fseek(fid, 0, SEEK_END);
    V3DLONG fileSize = ftell(fid);
#endif

    qDebug() << "file size: " << fileSize << endl;
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
    //long int mysz[4];
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
#if defined (Q_OS_WIN32)
    V3DLONG c,j,k;
    for (c = 0; c < sz[3]; c++)
    {
        for (k = startz; k < endz; k++)
        {
            for (j = starty; j< endy; j++)
            {
                rewind(fid);
                _fseeki64(fid, (V3DLONG)(head + (c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
                int dummy = _ftelli64(fid);
                dummy = fread(img+(c*cn+(k-startz)*kn + (j-starty)*tmpw)*unitSize,unitSize,tmpw,fid);
            }
        }
    }
#endif

#if defined (Q_OS_MAC) || defined (Q_OS_LINUX)
    V3DLONG c,j,k;
    for (c = 0; c < sz[3]; c++)
    {
        for (k = startz; k < endz; k++)
        {
            for (j = starty; j< endy; j++)
            {
                rewind(fid);
                fseek(fid, (V3DLONG)(head + (c*pgsz1 + k*pgsz2 + j*pgsz3 + startx)*unitSize), SEEK_SET);
                int dummy = ftell(fid);
                dummy = fread(img+(c*cn+(k-startz)*kn + (j-starty)*tmpw)*unitSize,unitSize,tmpw,fid);
            }
        }
    }
#endif
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

template <class T> bool extract_a_channel(T* data1d, V3DLONG *sz, V3DLONG c, void* &outimg)
{
    if (!data1d || !sz || c<0 || c>=sz[3])
    {
        printf("problem: c=[%ld] sz=[%p] szc=[%ld], data1d=[%p]\n",c, sz, sz[3], data1d);
        return false;
    }

    outimg = (void *) (data1d + c*sz[0]*sz[1]*sz[2]);

//    printf("ok c=[%ld]\n",c);
    return true;
}

int saveStack2Raw(const char * filename, const unsigned char * img, const V3DLONG * sz, int datatype)
{
    /* This function save a data stack to raw file */
    if (b_VERBOSE_PRINT)
        printf("size of [V3DLONG]=[%ld], [V3DLONG]=[%ld] [int]=[%ld], [short int]=[%ld], [double]=[%ld], [float]=[%ld]\n", sizeof(V3DLONG), sizeof(V3DLONG), sizeof(int), sizeof(short int), sizeof(double), sizeof(float));

    int berror = 0;
    V3DLONG i;

    FILE * fid = fopen(filename, "wb");
    if (!fid)
    {
        printf("Fail to open file for writing.\n");
        berror = 1;
        return berror;
    }

    /* Write header */

    char formatkey[] = "raw_image_stack_by_hpeng";
    int lenkey = strlen(formatkey);

    V3DLONG nwrite = fwrite(formatkey, 1, lenkey, fid);
    if (nwrite!=lenkey)
    {
        printf("File write error.\n");
        berror = 1;
        return berror;
    }

    char endianCodeMachine = checkMachineEndian();
    if (endianCodeMachine!='B' && endianCodeMachine!='L')
    {
        printf("This program only supports big- or little- endian but not other format. Cannot save data on this machine.\n");
        berror = 1;
        return berror;
    }

    nwrite = fwrite(&endianCodeMachine, 1, 1, fid);
    if (nwrite!=1)
    {
        printf("Error happened in file writing.\n");
        berror = 1;
        return berror;
    }

    short int dcode = (short int)datatype;
    if (dcode!=1 && dcode!=2 && dcode!=4)
    {
        printf("Unrecognized data type code [%d]. This code is not supported in this version.\n", dcode);
        berror = 1;
        return berror;
    }

    nwrite=fwrite(&dcode, 2, 1, fid);
    if (nwrite!=1)
    {
        printf("Writing file error.\n");
        berror = 1;
        return berror;
    }

    V3DLONG unitSize = datatype; /* temporarily I use the same number, which indicates the number of bytes for each data point (pixel). This can be extended in the future. */

    //short int mysz[4];
    BIT32_UNIT mysz[4];
    for (i=0;i<4;i++) mysz[i] = (BIT32_UNIT) sz[i];
    nwrite = fwrite(mysz, 4, 4, fid); /* because I have already checked the file size to be bigger than the header, no need to check the number of actual bytes read. */
    if (nwrite!=4)
    {
        printf("Writing file error.\n");
        berror = 1;
        return berror;
    }

    V3DLONG totalUnit = 1;
    for (i=0;i<4;i++)
    {
        totalUnit *= sz[i];
    }

    nwrite = fwrite(img, unitSize, totalUnit, fid);
    if (nwrite!=totalUnit)
    {
        printf("Something wrong in file writing. The program wrote [%ld data points] but the file says there should be [%ld data points].\n", nwrite, totalUnit);
        berror = 1;
        return berror;
    }

    /* clean and return */

    fclose(fid);

    return berror;
}

bool saveImage(const char filename[], const unsigned char * data1d, const V3DLONG * sz, const int datatype)
{
    if (!data1d || !filename || !sz)
    {
        printf("This image data is empty or the file name or the size pointer is invalid. Nothing done.\n");
        return false;
    }

    int dt;
    ImagePixelType curtype;
    switch (datatype)
    {
        case 1:  dt=1; curtype = V3D_UINT8; break;
        case 2:  dt=2; curtype = V3D_UINT16; break;
        case 4:  dt=4; curtype = V3D_FLOAT32; break;
        default:
            printf("The data type is unsupported. Nothing done.\n");
            return false;
            break;
    }


    if (saveStack2Raw(filename, data1d, sz, dt)!=0) //0 is no error //note that as I updated the saveStack2Raw to RAW-4-byte, the actual mask file cannot be read by the old wano program, i.e. the wano must be updated on Windows machine as well. 060921
    {
        printf("Error happens in writing RAW file stack [defined by Hanchuan Peng] [%s].\n", filename);
        return false;
    }

    return true;
}


bool cropRawimage(QString inputfile,QString outputfile,int xs,int ys,int zs,int xe,int ye,int ze,int c)
{
    unsigned char * datald = 0;
    V3DLONG *in_zz = 0;
    V3DLONG *in_sz = 0;


    char *input=new char[sizeof (inputfile.toStdString().c_str())];
    strcpy(input,inputfile.toStdString().c_str());

    int datatype;
    if (!loadRawRegion(input, datald, in_zz, in_sz,datatype,0,0,0,1,1,1))
    {
        return false;
    }

    if(datald) {delete []datald; datald = 0;}
    unsigned char * cropped_image = 0;

    if (!loadRawRegion(input, cropped_image, in_zz, in_sz,datatype,xs,ys,zs,xe+1,ye+1,ze+1))
    {
        return false;
    }

    if(c !=-1 && in_sz[3] > 1)
    {
        void* cropped_image_1channel = 0;
        switch(datatype)
        {
            case 1: extract_a_channel(cropped_image, in_sz, c, cropped_image_1channel); break;
            case 2: extract_a_channel((unsigned short int *)cropped_image, in_sz, c, cropped_image_1channel); break;
            case 4: extract_a_channel((float *)cropped_image, in_sz, c, cropped_image_1channel); break;
            default:return false;
        }

        in_sz[3] = 1;
        saveImage(outputfile.toStdString().c_str(),(unsigned char *)cropped_image_1channel,in_sz,datatype);
        if(cropped_image) {delete []cropped_image; cropped_image = 0;}
        if(cropped_image_1channel) {delete []cropped_image_1channel; cropped_image_1channel = 0;}
        return true;
    }

}

#endif // CROPIMAGE_H

