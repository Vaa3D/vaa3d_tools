#ifndef VOI_FUNC_H
#define VOI_FUNC_H

#include <v3d_interface.h>

bool SpecDialog(V3DPluginCallback2 &callback, QWidget *parent);
void Align2Dimages(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
                                    int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int z_slice,
                                        int imgX, int imgY, int imgZ);

void Crop3D(unsigned short InputImagePtr[], unsigned short OutputImagePtr[],
                    int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int z_slice,
                    int imgX, int imgY, int imgZ);

NeuronTree cropSWCfile3D(NeuronTree nt, int xb, int xe, int yb, int ye, int zb, int ze, int type);
QStringList importSeriesFileList_addnumbersort(const QString & curFilePath);

inline bool simple_loadimage_wrapper(V3DPluginCallback& cb, const char* filename, unsigned short*& pdata, V3DLONG sz[4], int& datatype)
{
    if (!filename || !sz)
        return false;

    Image4DSimple* inimg = 0;
    inimg = cb.loadImage((char *)filename);
    if (!inimg || !inimg->valid())
        return false;

    if (pdata) {delete []pdata; pdata=0;}

    V3DLONG totalbytes = inimg->getTotalBytes();
    try
    {
        pdata = new unsigned short [totalbytes];
        if (!pdata)
            goto Label_error_simple_loadimage_wrapper;

        memcpy(pdata, inimg->getRawData(), totalbytes);
        datatype = inimg->getUnitBytes(); //1,2,or 4
        sz[0] = inimg->getXDim();
        sz[1] = inimg->getYDim();
        sz[2] = inimg->getZDim();
        sz[3] = inimg->getCDim();
        if (inimg) {delete inimg; inimg=0;} //delete "outimg" pointer, added by Z.ZHOU 06122014

    }
    catch (...)
    {
        goto Label_error_simple_loadimage_wrapper;
    }

    return true;

Label_error_simple_loadimage_wrapper:
    if (inimg) {delete inimg; inimg=0;}
    return false;
}


/*
inline bool simple_saveimage_wrapper(V3DPluginCallback& cb, const char* filename, v3d_uint16* pdata, V3DLONG sz[4], int datatype)
{
    if (!filename || !sz || !pdata)
    {
        v3d_msg("some of the parameters for simple_saveimage_wrapper() are not valid.", 0);
        return false;
    }

    ImagePixelType dt;
    if (datatype==1)
        dt = V3D_UINT8;
    else if (datatype==2)
        dt = V3D_UINT16;
    else if (datatype==4)
        dt = V3D_FLOAT32;
    else
    {
        v3d_msg(QString("the specified save data type in simple_saveimage_wrapper() is not valid, dt=[%1].").arg(datatype), 0);
        return false;
    }

    Image4DSimple* outimg = new Image4DSimple;
    if (outimg)
        outimg->setData(pdata, sz[0], sz[1], sz[2], sz[3], dt);
    else
    {
        v3d_msg("Fail to new Image4DSimple for outimg.");
        return false;
    }

    return cb.saveImage(outimg, (char *)filename);
    //in this case no need to delete "outimg" pointer as it is just a container and will not use too much memory
}
*/
#endif // VOI_FUNC_H
