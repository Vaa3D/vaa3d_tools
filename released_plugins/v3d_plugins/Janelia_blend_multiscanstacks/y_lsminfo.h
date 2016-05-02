/*
 *  y_lsminfo.h
 *
 *  Zeiss .lsm info reading header
 *  created Jan. 25, 2012 by Yang Yu
 *  yuy@janelia.hhmi.org
 *
 */

#ifndef __Y_LSMINFO_H__
#define __Y_LSMINFO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

// following the rules from ITK .lsm reader
// Structure with LSM-specific data ( only in the first image directory).
// The size has been changed according to our data loading experiments. 
#define TIF_CZ_LSMINFO 34412
#define TIF_CZ_LSMINFO_SIZE_RESERVED 72
#define TIF_CZ_LSMINFO_SIZE 224
#define TIF_CZ_CHNINFO_SIZE 24

typedef char Int8_t;
typedef unsigned char UInt8_t;

typedef short Int16_t;
typedef unsigned short UInt16_t;

typedef int	    Int32_t;
typedef unsigned    UInt32_t;

typedef float       Float32_t;
typedef double      Float64_t;
typedef long double Float96_t;

typedef struct {
    UInt32_t    MagicNumber;
    UInt32_t    StructureSize;

    UInt32_t    DimensionX;
    UInt32_t    DimensionY;
    UInt32_t    DimensionZ;
    UInt32_t    DimensionChannels;
    UInt32_t    DimensionTime;

    UInt32_t    IntensityDataType;

    UInt32_t    ThumbnailX;
    UInt32_t    ThumbnailY;

    Float64_t   VoxelSizeX;
    Float64_t   VoxelSizeY;
    Float64_t   VoxelSizeZ;

    Float64_t   OriginX;
    Float64_t   OriginY;
    Float64_t   OriginZ;

    UInt16_t    ScanType;
    UInt16_t    SpectralScan;

    UInt32_t    DataType;

    UInt32_t    OffsetVectorOverlay;
    UInt32_t    OffsetInputLut;
    UInt32_t    OffsetOutputLut;
    UInt32_t    OffsetChannelColors;

    Float64_t   TimeIntervall;

    UInt32_t    OffsetChannelDataTypes;
    UInt32_t    OffsetScanInformation;
    UInt32_t    OffsetKsData;
    UInt32_t    OffsetTimeStamps;
    UInt32_t    OffsetEventList;
    UInt32_t    OffsetRoi;
    UInt32_t    OffsetBleachRoi;
    UInt32_t    OffsetNextRecording;

    UInt32_t    Reserved [ TIF_CZ_LSMINFO_SIZE_RESERVED ];

} zeiss_info; // itkLSMImageIO.cxx

typedef struct{
    Int32_t BlockSize;
    Int32_t NumberColors;
    Int32_t NumberNames;
    Int32_t ColorsOffset;
    Int32_t NamesOffset;
    Int32_t Mono;
}colorchannel_info;

typedef struct{
    int R, G, B; // UInt8_t
}Colors;


// Zeiss LSM INFO 
template <class Tidx>
class Y_LSMINFO
{
public:

    Y_LSMINFO(string imgname)
    {
        fn_image = imgname;
        
        if(fn_image.substr(fn_image.find_last_of(".") + 1)!="lsm")
        {
            cout<<"Error: invalid .lsm file"<<endl;
            return;
        }
    }

    ~Y_LSMINFO(){}

public:
    
    void loadHeader();

public:
    string               fn_image;
    
    zeiss_info           zi;
    colorchannel_info    ci;
    
    vector<Colors>       colorchannels;
};

// load header info
template <class Tidx> 
void Y_LSMINFO<Tidx> :: loadHeader()
{
    /// open lsm file
    FILE * lsm = fopen(fn_image.c_str(), "rb");
    if (!lsm)
    {
        printf("Fail to open file for reading.\n");
        return;
    }
    char *pbuf = NULL;
    try
    {
        pbuf = new char [512];
    }
    catch(...)
    {
        cout<<"Fail to allocate memory"<<endl;
        return;
    }
    
    /// read header
    fseek(lsm, 8, SEEK_SET);
    Tidx num_entries=0;
    fread(pbuf, 2, 1, lsm);
    num_entries = *((UInt16_t *)pbuf);
    
    Tidx entries_start_pos = ftell(lsm);
    
    UInt32_t offset;
    UInt16_t typecode;
    UInt32_t count, bytes;
    
    fseek(lsm, entries_start_pos, SEEK_SET);
    
    for(Tidx i=0; i<num_entries; i++)
    {
        offset=0;
        typecode=0;
        count=0;
        bytes=0;
        
        UInt16_t tagcode;
        fread(pbuf, 2, 1, lsm); // tagcode
        tagcode = *((UInt16_t *)pbuf);
        
        if(tagcode==TIF_CZ_LSMINFO) break;
        else
        {
            fread(pbuf, 2, 1, lsm); // typecode
            typecode = *((UInt16_t *)pbuf);
            
            fread(pbuf, 4, 1, lsm); // count
            count = *((UInt32_t *)pbuf);
            
            switch(typecode)
            {
                case 1: // uint8
                case 2: // uchar
                case 6: // int8
                case 7: // undef8
                    bytes = 1;
                    break;
                    
                case 3: // uint16
                case 8: // int16
                    bytes = 2;
                    break;
                    
                case 4: // uint32
                case 9: // int32
                case 11: // float32
                    bytes = 4;
                    break;
                    
                case 5: // uint32
                case 10: // int32
                case 12: // float64
                    bytes = 8;
                    break;
                    
                default:
                    break;
            }
            
            if(bytes*count > 4)
            {
                fread(pbuf, 4, 1, lsm); // offset
                offset = *((UInt32_t *)pbuf);
            }
            
            switch(typecode)
            {
                case 1: // uint8
                case 2: // uchar
                case 6: // int8
                case 7: // undef8
                    fread(pbuf, 1, count, lsm);
                    break;
                    
                    //case 3: // uint16
                case 8: // int16
                    fread(pbuf, 2, count, lsm);
                    break;
                    
                case 3: // uint16
                case 4: // uint32
                case 9: // int32
                case 11: // float32
                    fread(pbuf, 4, count, lsm);
                    break;
                    
                case 5: // uint32
                case 10: // int32
                case 12: // float64
                    fread(pbuf, 8, count, lsm);
                    break;
                    
                default:
                    break;
                    
            }
        }
    }
    
    fread(pbuf, 2, 1, lsm); // typecode
    typecode = *((UInt16_t *)pbuf);
    
    fread(pbuf, 4, 1, lsm); // count
    count = *((UInt32_t *)pbuf);
    
    switch(typecode)
    {
        case 1: // uint8
        case 2: // uchar
        case 6: // int8
        case 7: // undef8
            bytes = 1;
            break;
            
        case 3: // uint16
        case 8: // int16
            bytes = 2;
            break;
            
        case 4: // uint32
        case 9: // int32
        case 11: // float32
            bytes = 4;
            break;
            
        case 5: // uint32
        case 10: // int32
        case 12: // float64
            bytes = 8;
            break;
            
        default:
            break;
    }
    
    if(bytes*count > 4)
    {
        fread(pbuf, 4, 1, lsm);
        offset = *((UInt32_t *)pbuf);
    }
    
    /// read lsm info
    fseek(lsm, offset, SEEK_SET);    
    fread(pbuf, 1, TIF_CZ_LSMINFO_SIZE, lsm); //
    
    zi = *(reinterpret_cast<zeiss_info*>(pbuf));
    
    /// read color channel info
    fseek(lsm, zi.OffsetChannelColors, SEEK_SET);
    fread(pbuf, 1, TIF_CZ_CHNINFO_SIZE, lsm);

    ci = *(reinterpret_cast<colorchannel_info*>(pbuf));
    
    // read colors
    fseek(lsm, zi.OffsetChannelColors+ci.ColorsOffset, SEEK_SET);
    
    for(Tidx i=0; i<ci.NumberColors; i++)
    {
        fread(pbuf, 1, 4, lsm); // 4 bytes

        Colors c;

        c.R = int (pbuf[0]) & 255;
        c.G = int (pbuf[1] >> 8 ) & 255;
        c.B = int (pbuf[2] >> 16) & 255;
        
        this->colorchannels.push_back(c);
    }
    
    // read names
    // ...
    
    //
    fclose(lsm);
    
    // de-alloc
    if(pbuf){delete []pbuf; pbuf=0;}
    
    //
    return;
}


#endif // __Y_LSMINFO_H__

