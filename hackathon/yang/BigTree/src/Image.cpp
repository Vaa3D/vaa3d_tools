// Image.cpp

#include "Image.h"

//
void ssclear(stringstream *stringStreamInMemory)
{
    stringStreamInMemory->clear();
    stringStreamInMemory->str(string());
    delete stringStreamInMemory;
}

//
int makeDir(const char *filepath)
{
    //
    struct stat info;

    // if dir does not exist
    if( stat( filepath, &info ) != 0 )
    {
        if(mkdir(filepath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
        {
            cout<<"Fail to create folder: "<< filepath <<endl;
            return -1;
        }
    }
    //    else
    //    {
    //        cout<<"folder "<<filepath<<" already exists"<<endl;
    //    }

    //
    return 0;
}

//
char *copyFile(const char *srcFile, const char *dstFile)
{
    //
    std::ifstream  src(srcFile, std::ios::binary);
    std::ofstream  dst(dstFile, std::ios::binary);

    dst << src.rdbuf();

    src.close();
    dst.close();

    //
    return (char *) 0;
}

//
float fastmax(const std::vector<float>& v)
{
    float shared_max;
#pragma omp parallel
    {
        float max = std::numeric_limits<float>::max();
#pragma omp for nowait
        for(size_t ii=0; ii<v.size(); ++ii)
        {
            max = std::max(v[ii], max);
        }
#pragma omp critical
        {
            shared_max = std::min(shared_max, max);
        }
    }
    return shared_max;
}

//
void halveSample(uint8* img, int height, int width, int depth, int method, int bytes_chan)
{
    float A,B,C,D,E,F,G,H;

    int w,h,d;
    w = width/2;
    h = height/2;
    d = depth/2;

    //
    if ( bytes_chan == 1 )
    {
        if ( method == HALVE_BY_MEAN )
        {
            for(long z=0; z<d; z++)
            {
                for(long i=0; i<h; i++)
                {
                    for(long j=0; j<w; j++)
                    {
                        //computing 8-neighbours
                        A = img[2*z*width*height + 2*i*width + 2*j];
                        B = img[2*z*width*height + 2*i*width + (2*j+1)];
                        C = img[2*z*width*height + (2*i+1)*width + 2*j];
                        D = img[2*z*width*height + (2*i+1)*width + (2*j+1)];
                        E = img[(2*z+1)*width*height + 2*i*width + 2*j];
                        F = img[(2*z+1)*width*height + 2*i*width + (2*j+1)];
                        G = img[(2*z+1)*width*height + (2*i+1)*width + 2*j];
                        H = img[(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];

                        //computing mean
                        img[z*w*h + i*w + j] = (uint8) round((A+B+C+D+E+F+G+H)/(float)8);
                    }
                }

            }
        }
        else if ( method == HALVE_BY_MAX )
        {
            #pragma omp parallel for collapse(3)
            for(long z=0; z<d; z++)
            {
                for(long i=0; i<h; i++)
                {
                    for(long j=0; j<w; j++)
                    {
                        //computing max of 8-neighbours
                        A = img[2*z*width*height + 2*i*width + 2*j];
                        B = img[2*z*width*height + 2*i*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img[2*z*width*height + (2*i+1)*width + 2*j];
                        if ( B > A ) A = B;
                        B = img[2*z*width*height + (2*i+1)*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img[(2*z+1)*width*height + 2*i*width + 2*j];
                        if ( B > A ) A = B;
                        B = img[(2*z+1)*width*height + 2*i*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img[(2*z+1)*width*height + (2*i+1)*width + 2*j];
                        if ( B > A ) A = B;
                        B = img[(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];
                        if ( B > A ) A = B;
                        //computing max
                        img[z*w*h + i*w + j] = (uint8) round(A);
                    }
                }
            }


            // fast max downsampling
//            for(long z=0; z<d; z++)
//            {
//                long ofz1 = 2*z*width*height;
//                long ofz2 = (2*z+1)*width*height;
//                for(long i=0; i<h; i++)
//                {
//                    long ofy1 = ofz1 + 2*i*width;
//                    long ofy2 = ofz1 + (2*i+1)*width;
//                    long ofy3 = ofz2 + 2*i*width;
//                    long ofy4 = ofz2 + (2*i+1)*width;

//                    vector<float> v;
//                    v.clear();
//                    for(long j=0; j<w; j++)
//                    {
//                        // computing max of 8-neighbours
//                        v.push_back(img[ofy1 + 2*j]);
//                        v.push_back(img[ofy1 + 2*j + 1]);
//                        v.push_back(img[ofy2 + 2*j]);
//                        v.push_back(img[ofy2 + 2*j + 1]);
//                        v.push_back(img[ofy3 + 2*j]);
//                        v.push_back(img[ofy3 + 2*j + 1]);
//                        v.push_back(img[ofy4 + 2*j]);
//                        v.push_back(img[ofy4 + 2*j + 1]);

//                        //computing max
//                        img[z*w*h + i*w + j] = (uint8) round(fastmax(v));
//                    }
//                }
//            }
        }
        else
        {
            cout<<"halveSample(...): invalid halving method\n";
            return;
        }
    }
    else if ( bytes_chan == 2 )
    {
        uint16 *img16 = (uint16 *) img;

        if ( method == HALVE_BY_MEAN ) {

            for(long z=0; z<d; z++)
            {
                for(long i=0; i<h; i++)
                {
                    for(long j=0; j<w; j++)
                    {
                        //computing 8-neighbours
                        A = img16[2*z*width*height + 2*i*width + 2*j];
                        B = img16[2*z*width*height + 2*i*width + (2*j+1)];
                        C = img16[2*z*width*height + (2*i+1)*width + 2*j];
                        D = img16[2*z*width*height + (2*i+1)*width + (2*j+1)];
                        E = img16[(2*z+1)*width*height + 2*i*width + 2*j];
                        F = img16[(2*z+1)*width*height + 2*i*width + (2*j+1)];
                        G = img16[(2*z+1)*width*height + (2*i+1)*width + 2*j];
                        H = img16[(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];

                        //computing mean
                        img16[z*w*h + i*w + j] = (uint16) round((A+B+C+D+E+F+G+H)/(float)8);
                    }
                }
            }
        }
        else if ( method == HALVE_BY_MAX )
        {
            for(long z=0; z<d; z++)
            {
                for(long i=0; i<h; i++)
                {
                    for(long j=0; j<w; j++)
                    {
                        //computing max of 8-neighbours
                        A = img16[2*z*width*height + 2*i*width + 2*j];
                        B = img16[2*z*width*height + 2*i*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img16[2*z*width*height + (2*i+1)*width + 2*j];
                        if ( B > A ) A = B;
                        B = img16[2*z*width*height + (2*i+1)*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img16[(2*z+1)*width*height + 2*i*width + 2*j];
                        if ( B > A ) A = B;
                        B = img16[(2*z+1)*width*height + 2*i*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img16[(2*z+1)*width*height + (2*i+1)*width + 2*j];
                        if ( B > A ) A = B;
                        B = img16[(2*z+1)*width*height + (2*i+1)*width + (2*j+1)];
                        if ( B > A ) A = B;

                        //computing max
                        img16[z*w*h + i*w + j] = (uint16) round(A);
                    }
                }
            }
        }
        else
        {
            cout<<"halveSample(...): invalid halving method\n";
            return;
        }

    }
    else
    {
        cout<<"halveSample(...): invalid number of bytes per channel\n";
        return;
    }
}

//
void halveSample2D(uint8* img, int height, int width, int depth, int method, int bytes_chan)
{
    float A,B,C,D;

    //
    if ( bytes_chan == 1 )
    {
        if ( method == HALVE_BY_MEAN )
        {
            for(long z=0; z<depth; z++)
            {
                for(long i=0; i<height/2; i++)
                {
                    for(long j=0; j<width/2; j++)
                    {
                        // computing 8-neighbours
                        A = img[z*width*height + 2*i*width + 2*j];
                        B = img[z*width*height + 2*i*width + (2*j+1)];
                        C = img[z*width*height + (2*i+1)*width + 2*j];
                        D = img[z*width*height + (2*i+1)*width + (2*j+1)];

                        //computing mean
                        img[z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) round((A+B+C+D)/(float)4);
                    }
                }
            }
        }
        else if ( method == HALVE_BY_MAX )
        {
            for(long z=0; z<depth; z++)
            {
                for(long i=0; i<height/2; i++)
                {
                    for(long j=0; j<width/2; j++)
                    {
                        //computing max of 8-neighbours
                        A = img[z*width*height + 2*i*width + 2*j];
                        B = img[z*width*height + 2*i*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img[z*width*height + (2*i+1)*width + 2*j];
                        if ( B > A ) A = B;
                        B = img[z*width*height + (2*i+1)*width + (2*j+1)];
                        if ( B > A ) A = B;

                        //computing mean
                        img[z*(width/2)*(height/2) + i*(width/2) + j] = (uint8) round(A);
                    }
                }
            }
        }
    }
    else if ( bytes_chan == 2 )
    {
        uint16 *img16 = (uint16 *) img;

        if ( method == HALVE_BY_MEAN )
        {
            for(long z=0; z<depth; z++)
            {
                for(long i=0; i<height/2; i++)
                {
                    for(long j=0; j<width/2; j++)
                    {
                        //computing 8-neighbours
                        A = img16[z*width*height + 2*i*width + 2*j];
                        B = img16[z*width*height + 2*i*width + (2*j+1)];
                        C = img16[z*width*height + (2*i+1)*width + 2*j];
                        D = img16[z*width*height + (2*i+1)*width + (2*j+1)];

                        //computing mean
                        img16[z*(width/2)*(height/2) + i*(width/2) + j] = (uint16) round((A+B+C+D)/(float)4);
                    }
                }
            }
        }
        else if ( method == HALVE_BY_MAX )
        {
            for(long z=0; z<depth; z++)
            {
                for(long i=0; i<height/2; i++)
                {
                    for(long j=0; j<width/2; j++)
                    {
                        //computing max of 8-neighbours
                        A = img16[z*width*height + 2*i*width + 2*j];
                        B = img16[z*width*height + 2*i*width + (2*j+1)];
                        if ( B > A ) A = B;
                        B = img16[z*width*height + (2*i+1)*width + 2*j];
                        if ( B > A ) A = B;
                        B = img16[z*width*height + (2*i+1)*width + (2*j+1)];
                        if ( B > A ) A = B;

                        //computing mean
                        img16[z*(width/2)*(height/2) + i*(width/2) + j] = (uint16) round(A);
                    }
                }
            }
        }
    }
}

//
void copydata(unsigned char *psrc, uint32 stride_src, unsigned char *pdst, uint32 stride_dst, uint32 width, uint32 len)
{
    for ( uint32 i=0; i<len; i++, psrc+=stride_src, pdst+=stride_dst )
    {
        memcpy(pdst, psrc, width*sizeof(unsigned char));
    }
}

//
void swap2bytes(void *targetp)
{
    unsigned char * tp = (unsigned char *)targetp;
    unsigned char a = *tp;
    *tp = *(tp+1);
    *(tp+1) = a;
}

//
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

//
char *loadTiffMetaInfo(char* filename, uint32 &sz0, uint32  &sz1, uint32  &sz2, uint32  &sz3, uint16 &datatype)
{
    //
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    //
    TIFF *input = TIFFOpen(filename,"r");
    if (!input)
    {
        return ((char *) "Cannot open the file.");
    }

    if (!TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &sz0))
    {
        TIFFClose(input);
        return ((char *) "Image width of undefined.");
    }

    if (!TIFFGetField(input, TIFFTAG_IMAGELENGTH, &sz1))
    {
        TIFFClose(input);
        return ((char *) "Image length of undefined.");
    }

    if (!TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &datatype))
    {
        TIFFClose(input);
        return ((char *) "Undefined bits per sample.");
    }

//    if (!TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &sz3))
//    {
//        sz3 = 1;
//    }

    sz3 = 1; // hard coded here 6/8/2018

    uint16 Cpage;
    if (!TIFFGetField(input, TIFFTAG_PAGENUMBER, &Cpage, &sz2) || sz2==0)
    {
        sz2 = 1;
        while ( TIFFReadDirectory(input) )
        {
            sz2++;
        }
    }
    datatype /= 8;

    //
    TIFFClose(input);

    //
    return ((char *) 0);
}

//
void readTiff( stringstream *dataStreamInMemory, unsigned char *&img, unsigned int img_width, unsigned int img_height, unsigned int first, unsigned int last, int starti, int endi, int startj, int endj )
{
    //
    TIFF* input = TIFFStreamOpen("MemTIFF", (istream *)dataStreamInMemory);

    //
    uint32 rps;
    uint16 spp, bpp, photo, comp, planar_config;
    int StripsPerImage,LastStripSize;

    //
    if (!TIFFGetField(input, TIFFTAG_IMAGEWIDTH, &img_width))
    {
        TIFFClose(input);
        ssclear(dataStreamInMemory);
        return;
    }

    if (!TIFFGetField(input, TIFFTAG_IMAGELENGTH, &img_height))
    {
        TIFFClose(input);
        ssclear(dataStreamInMemory);
        return;
    }

    if (!TIFFGetField(input, TIFFTAG_BITSPERSAMPLE, &bpp))
    {
        TIFFClose(input);
        ssclear(dataStreamInMemory);
        return;
    }

    if (!TIFFGetField(input, TIFFTAG_SAMPLESPERPIXEL, &spp))
    {
        spp = 1;
    }

    int b_swap=TIFFIsByteSwapped(input);

    //
    if (!TIFFGetField(input, TIFFTAG_PHOTOMETRIC, &photo))
    {
        TIFFClose(input);
        ssclear(dataStreamInMemory);
        return;
    }

    if (!TIFFGetField(input, TIFFTAG_COMPRESSION, &comp))
    {
        TIFFClose(input);
        ssclear(dataStreamInMemory);
        return;
    }

    if (!TIFFGetField(input, TIFFTAG_PLANARCONFIG, &planar_config))
    {
        TIFFClose(input);
        ssclear(dataStreamInMemory);
        return;
    }

    //
    starti = (starti == -1) ? 0 : starti;
    endi   = (endi == -1) ? img_height-1 : endi;
    startj = (startj == -1) ? 0 : startj;
    endj   = (endj == -1) ? img_width-1 : endj;

    // file is internally tiled
    if (TIFFIsTiled(input))
    {
        uint32 tilewidth;
        uint32 tilelength;
        uint32 tiledepth;
        tsize_t tilenum;
        tsize_t tilesize;
        tsize_t tilenum_width;
        tsize_t tilenum_length;
        ttile_t tile;
        tdata_t data;
        unsigned char *psrc; // pointer in the tile buffer to the top left pixel of the current block to be copied
        unsigned char *pdst; // pointer in the image buffer to the top left pixel of the current block to be filled
        uint32 stride_src;
        uint32 stride_dst;
        int i; // row index in the slice of the top left pixel of the current block to be filled
        int j; // column index in the slice of the top left pixel of the current block to be filled
        uint32 width; // width of the current block to be filled (in pixels)
        uint32 len; // length of the current block to be filled (in pixels)
        int page;

        // checks
        if ( TIFFGetField(input, TIFFTAG_TILEDEPTH, &tiledepth) )
        {
            cout<<"Tiling among slices (z direction) not supported."<<endl;
            return;
        }
        if ( spp > 1 )
        {
            if ( TIFFGetField(input, TIFFTAG_PLANARCONFIG, &planar_config) )
            {
                if ( planar_config > 1 )
                {
                    cout<<"Non-interleaved multiple channels not supported with tiling."<<endl;
                    return;
                }
            }
        }

        // tiling is in x,y only
        TIFFGetField(input, TIFFTAG_TILEWIDTH, &tilewidth);
        TIFFGetField(input, TIFFTAG_TILELENGTH, &tilelength);
        tilenum = TIFFNumberOfTiles(input);
        tilesize = TIFFTileSize(input);
        tilenum_width  = (img_width % tilewidth) ? (img_width / tilewidth) + 1 : img_width / tilewidth;
        tilenum_length = (img_height % tilelength) ? (img_height / tilelength) + 1 : img_height / tilelength;

        data = new unsigned char[tilesize];
        stride_src = tilewidth * spp; // width of tile (in bytes)
        stride_dst = (endj - startj + 1) * spp; // width of subregion (in bytes)

        page = 0;
        do {

            psrc = ((unsigned char *)data) + ((starti % tilelength)*tilewidth + (startj % tilewidth)) * spp; // in the first tile skip (starti % tilelength) rows plus (startj % tilewidth) pixels
            pdst = img; // the buffer has the size of the subregion
            len = tilelength - (starti % tilelength); // rows to be copied for the first row of tiles
            tile = TIFFComputeTile(input,startj,starti,0,0); // index of the first tile to be copied in the current row of tiles
            for ( i=starti; i<=endi; ) {
                width = tilewidth - (startj%tilewidth); // width of the first block to be copied/filled
                for ( j=startj; j<=endj; ) {
                    TIFFReadEncodedTile(input,tile,data,(tsize_t) -1); // read tile into tile buffer
                    copydata (psrc,stride_src,pdst,stride_dst,(width * spp),len); // copy the block
                    j += width;
                    tile++; // index of the next tile in the same row of tiles
                    psrc = ((unsigned char *)data) + ((i % tilelength)*tilewidth) * spp; // the block in the next tile begins just after (i % tilelength) rows
                    pdst += width * spp; // the block in the image buffer move forward of width pixels
                    width = (((tile%tilenum_width) + 1) * tilewidth <= (endj+1)) ? tilewidth : ((endj+1)%tilewidth); // if the next tile in the row is all within the subregion, width is tilewidth otherwise it is shorter
                }
                i += len;
                tile = TIFFComputeTile(input,startj,i,0,0); // index of the first tile to be copied in the current row of tiles
                psrc = ((unsigned char *)data) + ((i % tilelength)*tilewidth + (startj % tilewidth)) * spp; // in the first tile of the next row of tiles skip (i % tilelength) rows plus (startj % tilewidth) pixels
                pdst = img + ((i-starti) * stride_dst); // the block in the image buffer begin after (i-starti) rows
                len = (((tile/tilenum_width) + 1) * tilelength <= (endi+1)) ? tilelength : ((endi+1)%tilelength); // if the next row of tiles is all within the subregion, len is tilelength otherwise it is shorter
            }

            page++;

        }while ( page < static_cast<int>(last-first+1) && TIFFReadDirectory(input));

        return;
    }

    //
    if (!TIFFGetField(input, TIFFTAG_ROWSPERSTRIP, &rps))
    {
        TIFFClose(input);
        dataStreamInMemory->clear();
        dataStreamInMemory->str(string());
        return;
    }

    StripsPerImage =  (img_height + rps - 1) / rps;
    LastStripSize = img_height % rps;
    if (LastStripSize==0)
        LastStripSize=rps;

    unsigned char *buf = img;
    int page=0;

    if ( starti < 0 || endi >= img_height || startj < 0 || endj >= img_width || starti >= endi || startj >= endj )
    {
        cout<<"Wrong substack indices."<<endl;
        return;
    }

    if ( starti == 0 && endi == (img_height-1) && startj == 0 && endj == (img_width-1) ) { // read whole images from files

        if (!TIFFSetDirectory(input, first))
        {
            TIFFClose(input);
            dataStreamInMemory->clear();
            dataStreamInMemory->str(string());
            return;
        }

        do{

            for (int i=0; i < StripsPerImage-1; i++){
                if (comp==1) {
                    TIFFReadRawStrip(input, i, buf, spp * rps * img_width * (bpp/8));
                    buf = buf + spp * rps * img_width * (bpp/8);
                }
                else{
                    TIFFReadEncodedStrip(input, i, buf, spp * rps * img_width * (bpp/8));
                    buf = buf + spp * rps * img_width * (bpp/8);
                }
            }

            if (comp==1) {
                TIFFReadRawStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
            }
            else{
                TIFFReadEncodedStrip(input, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
            }
            buf = buf + spp * LastStripSize * img_width * (bpp/8);

            page++;

        }while ( page < static_cast<int>(last-first+1) && TIFFReadDirectory(input));// while (TIFFReadDirectory(input));

    }
    else { // read only a subregion of images from files

        unsigned int XSIZE = img_width;

        unsigned char *rowbuf = new unsigned char[spp * rps * XSIZE * (bpp/8)];
        unsigned char *bufptr;

        do{
            if (!TIFFSetDirectory(input, first + page))
            {
                TIFFClose(input);
                ssclear(dataStreamInMemory);
                return;
            }

            int stripIndex = (starti / rps) - 1; // the strip preceeding the first one
            for (int i=starti; i <= endi; i++) {
                if ( floor((double)i / rps) > stripIndex ) { // read a new strip
                    stripIndex = (int)floor((double)i / rps);
                    if (comp==1) {
                        TIFFReadRawStrip(input, stripIndex, rowbuf, spp * ((stripIndex < StripsPerImage) ? rps :LastStripSize) * XSIZE * (bpp/8));
                    }
                    else{
                        TIFFReadEncodedStrip(input, stripIndex, rowbuf, spp * ((stripIndex < StripsPerImage) ? rps :LastStripSize) * XSIZE * (bpp/8));
                    }
                }
                bufptr = rowbuf + (i % rps) * (spp * XSIZE * (bpp/8));
                if ( bpp == 8 )
                    for (int j=0, j1=startj; j<=(endj-startj); j++, j1++) {
                        for (int c=0; c<spp; c++) {
                            buf[j * spp + c] = bufptr[j1 * spp + c];
                        }
                    }
                else
                    for (int j=0 , j1=startj; j<=(endj-startj); j++, j1++) {
                        for (int c=0; c<spp; c++) {
                            ((uint16 *)buf)[j * spp + c] = ((uint16 *)bufptr)[j1 * spp + c];
                        }
                    }
                buf = buf + spp * (endj-startj+1) * (bpp/8);
            }

            page++;

        }while ( page < static_cast<int>(last-first+1) );

        delete []rowbuf;

    }

    if ( page < static_cast<int>(last-first+1) ){
        return;
    }

    //
    TIFFClose(input);
    ssclear(dataStreamInMemory);

    // swap the data bytes if necessary
    if (b_swap)
    {
        int i;
        size_t total = img_width * img_height * spp * (last-first+1);
        if (bpp/8 == 2)
        {
            for (i=0;i<total; i++)
            {
                swap2bytes((void *)(img+2*i));
            }
        }
        else if (bpp/8 == 4)
        {
            for (i=0;i<total; i++)
            {
                swap4bytes((void *)(img+4*i));
            }
        }
    }

    //
    return;
}

// save a 3D chunk tif image with all zeros
char *initTiff3DFile(char *filename, int sz0, int sz1, int sz2, int sz3, int datatype)
{   
    //
    uint32 XSIZE  = sz0;
    uint32 YSIZE  = sz1;
    uint16 Npages = sz2;
    uint16 spp    = sz3;

    uint16 bpp=8 * datatype;

    int rowsPerStrip = -1;

    int check;

    if ( sz3 == 1 )
        spp = sz3;
    else if ( sz3 < 4 )
        spp = 3;
    else
        return ((char *) "More than 3 channels in Tiff files.");

    //
    long szSlice = (long)XSIZE * (long)YSIZE * (long)spp * (long)datatype;
    unsigned char *fakeData=NULL;
    try
    {
        fakeData = new unsigned char[ szSlice ];
        memset(fakeData,0,szSlice);
    }
    catch(...)
    {
        return ((char *)"Fail to alloc memory\n");
    }

    //disable warning and error handlers to avoid messages on unrecognized tags
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    TIFF *output;

    long expectedSize = ((long) sz0) * ((long) sz1) * ((long) sz2) * ((long) sz3) * ((long) datatype);
    long fourGBSize = 4;
    fourGBSize *= 1024;
    fourGBSize *= 1024;
    fourGBSize *= 1024;

    if ( expectedSize > (fourGBSize) )
    {
        if ( (rowsPerStrip == -1 && (((long) sz0) * ((long) sz1)) > (fourGBSize)) || ((rowsPerStrip * ((long) sz0)) > (fourGBSize)) )
            return ((char *) "Too many rows per strip for this image width.");
        else
            output = TIFFOpen(filename,"w8");
    }
    else
    {
        output = TIFFOpen(filename,"w");
    }

    if (!output)
    {
        return ((char *) "Cannot open the file.");
    }

    //
    if ( rowsPerStrip == -1 )
    {
        for(long slice=0; slice<Npages; slice++)
        {
            //
            TIFFSetDirectory(output, slice);

            //
            check = TIFFSetField(output, TIFFTAG_IMAGEWIDTH, XSIZE);
            if (!check) {
                return ((char *) "Cannot set the image width.");
            }

            check = TIFFSetField(output, TIFFTAG_IMAGELENGTH, YSIZE);
            if (!check) {
                return ((char *) "Cannot set the image height.");
            }

            check = TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, bpp);
            if (!check) {
                return ((char *) "Cannot set the image bit per sample.");
            }

            check = TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, spp);
            if (!check) {
                return ((char *) "Cannot set the image sample per pixel.");
            }

            check = TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? YSIZE : (uint32)rowsPerStrip);
            if (!check) {
                return ((char *) "Cannot set the image rows per strip.");
            }

            check = TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            if (!check) {
                return ((char *) "Cannot set the image orientation.");
            }

            check = TIFFSetField(output, TIFFTAG_COMPRESSION, COMPPRESSION_METHOD);
            if (!check) {
                return ((char *) "Cannot set the compression tag.");
            }

            check = TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
            if (!check) {
                return ((char *) "Cannot set the planarconfig tag.");
            }

            if ( spp == 1 )
                check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
            else // spp == 3
                check = TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
            if (!check) {
                return ((char *) "Cannot set the photometric tag.");
            }

            /* We are writing single page of the multipage file */
            check = TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
            if (!check) {
                return ((char *) "Cannot set the subfiletype tag.");
            }

            check = TIFFSetField(output, TIFFTAG_PAGENUMBER, slice, Npages);
            if (!check) {
                return ((char *) "Cannot set the page number.");
            }

            if(!TIFFWriteEncodedStrip(output, 0, fakeData, szSlice))
            {
                return ((char *) "Cannot write encoded strip to file.");
            }

            //
            if (!TIFFWriteDirectory(output))
            {
                return ((char *) "Cannot write a new directory.");
            }
        }
    }
    else
    {
        // TODO: modify codes to save 3D image stack later

        //
        // save one slice
        //
        int check,StripsPerImage,LastStripSize;
        uint32 rps = (uint32)rowsPerStrip;
        unsigned char *buf = fakeData;

        StripsPerImage =  (YSIZE + rps - 1) / rps;
        LastStripSize = YSIZE % rps;
        if (LastStripSize==0)
            LastStripSize=rps;

        for (int i=0; i < StripsPerImage-1; i++){
            check = TIFFWriteEncodedStrip(output, i, buf, spp * rps * XSIZE * (bpp/8));
            if (!check) {
                return ((char *) "Cannot write encoded strip to file.");
            }
            buf = buf + spp * rps * XSIZE * (bpp/8);
        }

        check = TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * XSIZE * (bpp/8));
        if (!check) {
            return ((char *) "Cannot write encoded strip to file.");
        }
        buf = buf + spp * LastStripSize * XSIZE * (bpp/8);
    }

    //
    if(fakeData)
    {
        delete[] fakeData;
    }

    //
    TIFFClose(output);

    //
    return (char *) 0;
}

//
int openTiff3DFile(char *filename, char *mode, void *&fhandle, bool reopen)
{
    //
    char *completeMode = NULL;

    //
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    // if mode is 'w' and reopen is true check if the file esists
    if ( mode[0] == 'w' && reopen )
    {
        fhandle = TIFFOpen(filename,"r");
        if ( fhandle )
        {
            TIFFClose((TIFF *) fhandle);
        }
    }

    bool mybigtiff = false;
    if ( mybigtiff && !strstr(mode,"8") )
    {
        completeMode = new char[strlen(mode)+2];
        strcpy(completeMode,mode);
        strcat(completeMode,"8");
    }
    else
    {
        completeMode = new char[strlen(mode)+2];
        strcpy(completeMode,mode);
    }

    fhandle = TIFFOpen(filename,completeMode);

    delete completeMode;

    if (!fhandle)
    {
        cout<<"Cannot open the file.\n";
        return -1;
    }

    //
    return 0;
}

//
char *appendSlice2Tiff3DFile(void *fhandler, int slice, unsigned char *img, unsigned int img_width, unsigned int img_height, int spp, int bpp, int NPages)
{
    //
    int rowsPerStrip = -1;

    //
    TIFF *output = (TIFF *) fhandler;

    // WARNING: slice must be the first page after the last, otherwise the file can be corrupted
    TIFFSetDirectory(output,slice);

    TIFFSetField(output, TIFFTAG_IMAGEWIDTH, img_width);
    TIFFSetField(output, TIFFTAG_IMAGELENGTH, img_height);
    TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, (uint16)bpp);
    TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, (uint16)spp);
    TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, (rowsPerStrip == -1) ? img_height : (uint32)rowsPerStrip);
    TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
    TIFFSetField(output, TIFFTAG_COMPRESSION, COMPPRESSION_METHOD);
    TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
    if ( spp == 1 )
        TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);
    else // spp == 3
        TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
    // We are writing single page of the multipage file
    TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
    TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, (uint16)NPages);

    // the file has been already opened: rowsPerStrip it is not too large for this image width
    if ( rowsPerStrip == -1 )
    {
        TIFFWriteEncodedStrip(output, 0, img, img_width * img_height * spp * (bpp/8));
    }
    else
    {
        int StripsPerImage,LastStripSize;
        uint32 rps = (uint32)rowsPerStrip;
        unsigned char *buf = img;

        StripsPerImage =  (img_height + rps - 1) / rps;
        LastStripSize = img_height % rps;
        if (LastStripSize==0)
            LastStripSize=rps;

        for (int i=0; i < StripsPerImage-1; i++){
            TIFFWriteEncodedStrip(output, i, buf, spp * rps * img_width * (bpp/8));
            buf = buf + spp * rps * img_width * (bpp/8);
        }

        TIFFWriteEncodedStrip(output, StripsPerImage-1, buf, spp * LastStripSize * img_width * (bpp/8));
        buf = buf + spp * LastStripSize * img_width * (bpp/8);
    }

    TIFFWriteDirectory(output);

    //
    return (char *) 0;
}

//
int writeTiff3DFile(char* filename, uint8 *img, int x, int y, int z, int c, int datatype)
{
    //
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    //
    void *fhandle = 0;
    openTiff3DFile(filename, (char *)("w"), fhandle, false);

    for(int i=0; i<z; i++)
    {
        appendSlice2Tiff3DFile(fhandle, i, img+i*x*y*c*datatype, x, y, c, datatype*8, z);
    }

    //
    del1dp(img);

    //
    TIFFClose((TIFF *) fhandle);

    //
    return 0;
}

//
int tiffIOTest(char* inputFileName, char *outputFileName, int compressionMethod)
{
    //
    TIFFSetWarningHandler(0);
    TIFFSetErrorHandler(0);

    //
    return 0;
}

//
int compareString(const char *ptr0, const char *ptr1, int len)
{
    int fast = len/sizeof(size_t) + 1;
    int offset = (fast-1)*sizeof(size_t);
    int current_block = 0;

    if( len <= sizeof(size_t)){ fast = 0; }

    size_t *lptr0 = (size_t*)ptr0;
    size_t *lptr1 = (size_t*)ptr1;

    while( current_block < fast )
    {
        if( (lptr0[current_block] ^ lptr1[current_block] ))
        {
            int pos;
            for(pos = current_block*sizeof(size_t); pos < len ; ++pos )
            {
                if( (ptr0[pos] ^ ptr1[pos]) || (ptr0[pos] == 0) || (ptr1[pos] == 0) )
                {
                    return  (int)((unsigned char)ptr0[pos] - (unsigned char)ptr1[pos]);
                }
            }
        }

        ++current_block;
    }

    while( len > offset )
    {
        if( (ptr0[offset] ^ ptr1[offset] ))
        {
            return (int)((unsigned char)ptr0[offset] - (unsigned char)ptr1[offset]);
        }
        ++offset;
    }

    //
    return 0;
}
