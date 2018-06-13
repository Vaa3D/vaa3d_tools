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
    unsigned char * pImg1, *pImg2, *p;
    long sx1, sx2, sy1, sy2, sz1, sz2, sc1, sc2;
    long imgsz, outsz;
    long datatype1, datatype2;

    // load image1
    if(fn_image1.toUpper().endsWith(".V3DRAW") || fn_image1.toUpper().endsWith(".TIF"))
    {
        Image4DSimple * p4dImage1 = callback.loadImage( const_cast<char *>(fn_image1.toStdString().c_str()) );
        if (!p4dImage1 || !p4dImage1->valid())
        {
            cout<<"fail to load image1!\n";
            return false;
        }

        datatype1 = p4dImage1->getDatatype();

        sx1 = p4dImage1->getXDim();
        sy1 = p4dImage1->getYDim();
        sz1 = p4dImage1->getZDim();

        pImg1 = p4dImage1->getRawData();

        // load image2
        if(fn_image2.toUpper().endsWith(".V3DRAW") || fn_image2.toUpper().endsWith(".TIF"))
        {
            Image4DSimple * p4dImage2 = callback.loadImage( const_cast<char *>(fn_image1.toStdString().c_str()) );
            if (!p4dImage2 || !p4dImage2->valid())
            {
                cout<<"fail to load image2!\n";
                return false;
            }

            datatype2 = p4dImage2->getDatatype();

            if(datatype1 != datatype2)
            {
                cout<<"datatypes are not matched"<<endl;
                return false;
            }

            sx2 = p4dImage2->getXDim();
            sy2 = p4dImage2->getYDim();
            sz2 = p4dImage2->getZDim();

            if(sz1 != sz2 || sy1 != sy2 || sx1 != sx2)
            {
                cout<<"image dimensions are not matched"<<endl;
                return false;
            }

            //
            imgsz = sx1*sy1*sz1;

            //
            pImg2 = p4dImage2->getRawData();

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
            if(datatype1 == V3D_UINT16)
            {
                unsigned short *p1 = (unsigned short *) (pImg1);
                unsigned short *p2 = (unsigned short *) (pImg2);

                //
                unsigned char *p=NULL;

                long sx = sx1 / 2;
                long sy = sy1 / 2;

                try
                {
                    p = new unsigned char [sx*sy];
                }
                catch(...)
                {
                    cout<<"fail to alloc memory for out image\n";
                    return false;
                }

                // computing max of 8-neighbours
                for(long y=0; y<sy; y++)
                {
                    long ofy = y*sx;

                    long ofy1 = 2*y*sx1;
                    long ofy2 = (2*y+1)*sx1;
                    for(long x=0; x<sx; x++)
                    {
                        //
                        A = p1[ofy1 + 2*x];
                        B = p1[ofy1 + 2*x+1];
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
                        p[ofy + x] = (unsigned char) round(A);
                    }
                }
            }
            else if(datatype1 == V3D_UINT8)
            {

            }
            else
            {
                cout<<"datatype is not supported"<<endl;
                return false;
            }

            // save result
            QString outFileName = QString(outlist->at(0));

            //
            Image4DSimple p4dimg;

            p4dimg.setDatatype(V3D_UINT8);
            p4dimg.setXDim(sx);
            p4dimg.setYDim(sy);
            p4dimg.setZDim(1); // 2D image as developing
            p4dimg.setCDim(1);
            p4dimg.setTDim(1);

            p4dimg.setNewRawDataPointer(p);
            p4dimg.saveImage(const_cast<char *>(outFileName.toStdString().c_str()));

            //
            if(p)
            {
                delete []p;
            }

        } // image2
    } // image1
       
    //
    return true;
}
