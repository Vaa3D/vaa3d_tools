/* sample2images_func.cpp
 * a plugin to sample 2 images
 * 6/13/2018 : by Yang Yu
 */

//
#include "sample2images_func.h"

#if  defined(Q_OS_LINUX)
#include <omp.h>
#endif

//
const QString title = QObject::tr("sample 2 images");

//
char *tiffread(char* filename, unsigned char *&p, uint32 &sz0, uint32  &sz1, uint32  &sz2, uint16 &datatype, uint16 &comp)
{
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

    //cout<<"test "<<sz0<<" "<<sz1<<" "<<sz2<<" "<<datatype<<endl;

    long imgsz = (long)sz0*(long)sz1*(long)sz2*(long)datatype;

    //
    try
    {
        p = new unsigned char [imgsz];
    }
    catch(...)
    {
        return ((char*) "fail to alloc memory for loading a tiff image.");
    }

    //
    uint32 rps;
    int StripsPerImage,LastStripSize;

    //
    if (!TIFFGetField(input, TIFFTAG_ROWSPERSTRIP, &rps))
    {
        TIFFClose(input);
        return ((char *) "Undefined rowsperstrip.");
    }

    if (!TIFFGetField(input, TIFFTAG_COMPRESSION, &comp))
    {
        TIFFClose(input);
        return ((char *) "Undefined compression.");
    }

    StripsPerImage =  (sz1 + rps - 1) / rps;
    LastStripSize = sz1 % rps;
    if (LastStripSize==0)
        LastStripSize=rps;

    if (!TIFFSetDirectory(input, 0)) // init
    {
        TIFFClose(input);
        return ((char *) "fail to setdir.");
    }

    unsigned char *buf = p;

    do{
        for (int i=0; i < StripsPerImage-1; i++)
        {
            if (comp==1)
            {
                TIFFReadRawStrip(input, i, buf,  rps * sz0 * datatype);
                buf = buf + rps * sz0 * datatype;
            }
            else
            {
                TIFFReadEncodedStrip(input, i, buf, rps * sz0 * datatype);
                buf = buf + rps * sz0 * datatype;
            }
        }

        if (comp==1)
        {
            TIFFReadRawStrip(input, StripsPerImage-1, buf, LastStripSize * sz0 * datatype);
        }
        else
        {
            TIFFReadEncodedStrip(input, StripsPerImage-1, buf, LastStripSize * sz0 * datatype);
        }
        buf = buf + LastStripSize * sz0 * datatype;

    }
    while (TIFFReadDirectory(input)); // while (TIFFReadDirectory(input));

    //
    TIFFClose(input);

    //
    return ((char *) 0);
}

//
char *tiffwrite(char* filename, unsigned char *p, uint32 sz0, uint32  sz1, uint32  sz2, uint16 datatype, uint16 comp)
{

    TIFF *output = TIFFOpen(filename,"w");

    //
    if(sz2>1)
    {
        // 3D TIFF
        for(long slice=0; slice<sz2; slice++)
        {
            TIFFSetDirectory(output,slice);

            TIFFSetField(output, TIFFTAG_IMAGEWIDTH, sz0);
            TIFFSetField(output, TIFFTAG_IMAGELENGTH, sz1);
            TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, (uint16)(datatype*8));
            TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, 1);
            TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, sz1);
            TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
            TIFFSetField(output, TIFFTAG_COMPRESSION, comp);
            TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
            //TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
            TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

            // We are writing single page of the multipage file
            TIFFSetField(output, TIFFTAG_SUBFILETYPE, FILETYPE_PAGE);
            TIFFSetField(output, TIFFTAG_PAGENUMBER, (uint16)slice, (uint16)sz2);

            // the file has been already opened: rowsPerStrip it is not too large for this image width
            TIFFWriteEncodedStrip(output, 0, p, sz0 * sz1 * datatype);

            TIFFWriteDirectory(output);
        }
    }
    else
    {
        // 2D TIFF
        TIFFSetField(output, TIFFTAG_IMAGEWIDTH, sz0);
        TIFFSetField(output, TIFFTAG_IMAGELENGTH, sz1);
        TIFFSetField(output, TIFFTAG_BITSPERSAMPLE, (uint16)(datatype*8));
        TIFFSetField(output, TIFFTAG_SAMPLESPERPIXEL, 1);
        TIFFSetField(output, TIFFTAG_ROWSPERSTRIP, sz1);
        TIFFSetField(output, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
        TIFFSetField(output, TIFFTAG_COMPRESSION, comp);
        TIFFSetField(output, TIFFTAG_PLANARCONFIG,PLANARCONFIG_CONTIG);
        TIFFSetField(output, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_MINISBLACK);

        // the file has been already opened: rowsPerStrip it is not too large for this image width
        TIFFWriteEncodedStrip(output, 0, p, sz0 * sz1 * datatype);

        //
        TIFFWriteDirectory(output);
    }

    //
    TIFFClose(output);

    //
    return ((char *) 0);
}

//
bool sample2images_func(const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 &callback)
{
    //
    if(input.size()<1 || output.size() != 1)
    {
        cout<<"vaa3d -x sample2images -f sample2images -i image1.tif image2.tif -o output.tif"<<endl;
        return false;
    }

    // parsing input
    if (input.size()>1)
    {
        vector<char*> * paras = (vector<char*> *)(input.at(1).p);
        if (paras->size() >= 1)
        {
            // parameters
        }
        else
        {
            cerr<<"unknow parameters"<<endl;
            return false;
        }
    }

    vector<char *> * outlist = (vector<char*> *)(output.at(0).p);
    if (outlist->size()>1)
    {
        cerr << "You cannot specify more than 1 output files"<<endl;
        return false;
    }

    //
    vector<char *> * inlist =  (vector<char*> *)(input.at(0).p);
    if (inlist->size()<1)
    {
        cerr<<"You must specify input files"<<endl;
        return false;
    }

    //
    QString fn_image1 = QString(inlist->at(0));
    QString fn_image2 = QString(inlist->at(1));

    // load images
    unsigned char *pImg1, *pImg2, *p;
    uint32 sx1, sx2, sy1, sy2, sz1, sz2, sx, sy;
    long imgsz, outsz;
    uint16 datatype1, datatype2;
    uint16 comp;

//    // load image1
//    if(fn_image1.toUpper().endsWith(".V3DRAW"))
//    {
//        Image4DSimple * p4dImage1 = callback.loadImage( const_cast<char *>(fn_image1.toStdString().c_str()) );
//        if (!p4dImage1 || !p4dImage1->valid())
//        {
//            cout<<"fail to load image1!\n";
//            return false;
//        }

//        datatype1 = p4dImage1->getDatatype();

//        sx1 = p4dImage1->getXDim();
//        sy1 = p4dImage1->getYDim();
//        sz1 = p4dImage1->getZDim();

//        pImg1 = p4dImage1->getRawData();

//        // load image2
//        if(fn_image2.toUpper().endsWith(".V3DRAW"))
//        {
//            Image4DSimple * p4dImage2 = callback.loadImage( const_cast<char *>(fn_image1.toStdString().c_str()) );
//            if (!p4dImage2 || !p4dImage2->valid())
//            {
//                cout<<"fail to load image2!\n";
//                return false;
//            }

//            datatype2 = p4dImage2->getDatatype();

//            if(datatype1 != datatype2)
//            {
//                cout<<"datatypes are not matched"<<endl;
//                return false;
//            }

//            sx2 = p4dImage2->getXDim();
//            sy2 = p4dImage2->getYDim();
//            sz2 = p4dImage2->getZDim();

//            if(sz1 != sz2 || sy1 != sy2 || sx1 != sx2)
//            {
//                cout<<"image dimensions are not matched"<<endl;
//                return false;
//            }

//            //
//            imgsz = sx1*sy1*sz1;

//            //
//            pImg2 = p4dImage2->getRawData();

//            // bit shift
//            if(datatype1 == V3D_UINT16)
//            {
//                unsigned short *p1 = (unsigned short *) (pImg1);
//                unsigned short *p2 = (unsigned short *) (pImg2);
//                for(long i=0; i<imgsz; i++ )
//                {
//                    p1[i] = p1[i] >> 4;
//                    p2[i] = p2[i] >> 4;
//                }
//            }

//            // downsample
//            if(datatype1 == V3D_UINT16)
//            {
//                unsigned short *p1 = (unsigned short *) (pImg1);
//                unsigned short *p2 = (unsigned short *) (pImg2);

//                //
//                unsigned char *p=NULL;

//                sx = sx1 / 2;
//                sy = sy1 / 2;

//                try
//                {
//                    p = new unsigned char [sx*sy];
//                }
//                catch(...)
//                {
//                    cout<<"fail to alloc memory for out image\n";
//                    return false;
//                }

//                // computing max of 8-neighbours
//                for(long y=0; y<sy; y++)
//                {
//                    long ofy = y*sx;

//                    long ofy1 = 2*y*sx1;
//                    long ofy2 = (2*y+1)*sx1;
//                    for(long x=0; x<sx; x++)
//                    {
//                        //
//                        unsigned short A = p1[ofy1 + 2*x];
//                        unsigned short B = p1[ofy1 + 2*x+1];
//                        if ( B > A ) A = B;

//                        B = p1[ofy2 + 2*x];
//                        if ( B > A ) A = B;

//                        B = p1[ofy2 + 2*x+1];
//                        if ( B > A ) A = B;

//                        B = p2[ofy1 + 2*x];
//                        if ( B > A ) A = B;

//                        B = p2[ofy1 + 2*x+1];
//                        if ( B > A ) A = B;

//                        B = p2[ofy2 + 2*x];
//                        if ( B > A ) A = B;

//                        B = p2[ofy2 + 2*x+1];
//                        if ( B > A ) A = B;

//                        // computing max
//                        p[ofy + x] = (unsigned char) round(A);
//                    }
//                }
//            }
//            else if(datatype1 == V3D_UINT8)
//            {

//            }
//            else
//            {
//                cout<<"datatype is not supported"<<endl;
//                return false;
//            }

//            // save result
//            QString outFileName = QString(outlist->at(0));

//            //
//            Image4DSimple p4dimg;

//            p4dimg.setDatatype(V3D_UINT8);
//            p4dimg.setXDim(sx);
//            p4dimg.setYDim(sy);
//            p4dimg.setZDim(1); // 2D image as developing
//            p4dimg.setCDim(1);
//            p4dimg.setTDim(1);

//            p4dimg.setNewRawDataPointer(p);
//            p4dimg.saveImage(const_cast<char *>(outFileName.toStdString().c_str()));

//            //
//            if(p)
//            {
//                delete []p;
//            }

//        } // image2
//    } // image1


    // load image1
    if(fn_image1.toUpper().endsWith(".TIF"))
    {
        //
        TIFFSetWarningHandler(0);
        TIFFSetErrorHandler(0);

        //
        char *error_check1 = tiffread(const_cast<char *>(fn_image1.toStdString().c_str()), pImg1, sx1, sy1, sz1, datatype1, comp);
        if(error_check1)
        {
            cout<<error_check1<<endl;
            return false;
        }

        // load image2
        if(fn_image2.toUpper().endsWith(".TIF"))
        {
            char *error_check2 = tiffread(const_cast<char *>(fn_image2.toStdString().c_str()), pImg2, sx2, sy2, sz2, datatype2, comp);
            if(error_check2)
            {
                cout<<error_check2<<endl;
                return false;
            }

            //
            if(datatype1 != datatype2)
            {
                cout<<"datatypes are not matched"<<endl;
                return false;
            }

            //
            if(sz1 != sz2 || sy1 != sy2 || sx1 != sx2)
            {
                cout<<"image dimensions are not matched"<<endl;
                return false;
            }

            //
            imgsz = sx1*sy1*sz1;

            // bit shift
            if(datatype1 == V3D_UINT16)
            {
                unsigned short *p1 = (unsigned short *) (pImg1);
                unsigned short *p2 = (unsigned short *) (pImg2);
                for(long i=0; i<imgsz; i++ )
                {
                    p1[i] = p1[i] >> 4;
                    p2[i] = p2[i] >> 4;
                }
            }

            // downsample

            //
            sx = sx1 / 2;
            sy = sy1 / 2;

            try
            {
                outsz = sx*sy;
                p = new unsigned char [outsz];
                memset(p, 0, outsz);
            }
            catch(...)
            {
                cout<<"fail to alloc memory for out image\n";
                return false;
            }

            //
            if(datatype1 == V3D_UINT16)
            {
                unsigned short *p1 = (unsigned short *) (pImg1);
                unsigned short *p2 = (unsigned short *) (pImg2);

                // computing max of 8-neighbours
                for(long y=0; y<sy; y++)
                {
                    long ofy = y*sx;

                    long ofy1 = 2*y*sx1;
                    long ofy2 = (2*y+1)*sx1;

                    for(long x=0; x<sx; x++)
                    {
                        //
                        int A = p1[ofy1 + 2*x];
                        int B = p1[ofy1 + 2*x+1];
                        if ( B > A ) A = B;

                        B = p1[ofy2 + 2*x];
                        if ( B > A ) A = B;

                        B = p1[ofy2 + 2*x+1];
                        if ( B > A ) A = B;

                        B = p2[ofy1 + 2*x];
                        if ( B > A ) A = B;

                        B = p2[ofy1 + 2*x+1];
                        if ( B > A ) A = B;

                        B = p2[ofy2 + 2*x];
                        if ( B > A ) A = B;

                        B = p2[ofy2 + 2*x+1];
                        if ( B > A ) A = B;

                        // computing max
                        p[ofy + x] = (unsigned char)(A);
                    }
                }
            }
            else if(datatype1 == V3D_UINT8)
            {
                unsigned char *p1 = (unsigned char *) (pImg1);
                unsigned char *p2 = (unsigned char *) (pImg2);

                // computing max of 8-neighbours
                for(long y=0; y<sy; y++)
                {
                    long ofy = y*sx;

                    long ofy1 = 2*y*sx1;
                    long ofy2 = (2*y+1)*sx1;

                    for(long x=0; x<sx; x++)
                    {
                        //
                        int A = p1[ofy1 + 2*x];
                        int B = p1[ofy1 + 2*x+1];
                        if ( B > A ) A = B;

                        B = p1[ofy2 + 2*x];
                        if ( B > A ) A = B;

                        B = p1[ofy2 + 2*x+1];
                        if ( B > A ) A = B;

                        B = p2[ofy1 + 2*x];
                        if ( B > A ) A = B;

                        B = p2[ofy1 + 2*x+1];
                        if ( B > A ) A = B;

                        B = p2[ofy2 + 2*x];
                        if ( B > A ) A = B;

                        B = p2[ofy2 + 2*x+1];
                        if ( B > A ) A = B;

                        // computing max
                        p[ofy + x] = (unsigned char)(A);
                    }
                }
            }
            else
            {
                cout<<"datatype is not supported"<<endl;
                return false;
            }

            // save result
            QString outFileName = QString(outlist->at(0));

            //
            char *error_check = tiffwrite(const_cast<char *>(outFileName.toStdString().c_str()), p, sx, sy, 1, 1, comp);
            if(error_check)
            {
                cout<<error_check<<endl;
                return false;
            }

            //
            if(pImg1)
            {
                delete []pImg1;
            }

            if(pImg2)
            {
                delete []pImg2;
            }

            if(p)
            {
                delete []p;
            }
        }
    }

    //
    return true;
}
