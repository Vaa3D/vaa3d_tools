#include "get_sub_block.h"

bool get_sub_block(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString fileOpenName;
    fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
                                                "",

                                                QObject::tr("Supported file (*.swc *.SWC *.eswc)"

                                            ));
    NeuronTree nt=readSWC_file(fileOpenName);
    cout<<fileOpenName.toStdString()<<endl;
    cout<<"nt.listNeuron = "<<nt.listNeuron.size()<<endl;

    QString inimg_file = callback.getPathTeraFly();
    //cout<<"inimg = "<<inimg_file.toStdString()<<endl;

    //QString inimg_file = "/media/yong/zhang/mouseID_321237-17302/RES(54600x34412x9847)";

    V3DLONG minx = 100000000;
    V3DLONG miny = 100000000;
    V3DLONG minz = 100000000;
    V3DLONG maxx = -100000000;
    V3DLONG maxy = -100000000;
    V3DLONG maxz = -100000000;
    for(V3DLONG i = 0;i < nt.listNeuron.size();i++)
    {
        if(nt.listNeuron[i].x < minx)
            minx = nt.listNeuron[i].x;
        if(nt.listNeuron[i].y < miny)
            miny = nt.listNeuron[i].y;
        if(nt.listNeuron[i].z < minz)
            minz = nt.listNeuron[i].z;

        if(nt.listNeuron[i].x > maxx)
            maxx = nt.listNeuron[i].x;
        if(nt.listNeuron[i].y > maxy)
            maxy= nt.listNeuron[i].y;
        if(nt.listNeuron[i].z > maxz)
            maxz = nt.listNeuron[i].z;
    }
    cout<<"minx = "<<minx<<"  miny = "<<miny<<"  minz = "<<minz<<endl;
    cout<<"maxx = "<<maxx<<"  maxy = "<<maxy<<"  maxz = "<<maxz<<endl;

    double lenx = maxx - minx;
    double leny = maxy - miny;
    double lenz = maxz - minz;
    int spacex = 0.2*lenx;
    int spacey = 0.2*leny;
    int spacez = 0.2*lenz;
    if(spacex<1)spacex=15;
    if(spacey<1)spacey=15;
    if(spacez<1)spacez=15;

    V3DLONG xb = minx-spacex;
    V3DLONG xe = maxx+spacex;
    V3DLONG yb = miny-spacey;
    V3DLONG ye = maxy+spacey;
    V3DLONG zb = minz-spacez;
    V3DLONG ze = maxz+spacez;

    double block_sz[3];
    block_sz[0] = xb;
    block_sz[1] = yb;
    block_sz[2] = zb;

    V3DLONG pagesz;
    V3DLONG im_cropped_sz[4];
    unsigned char * im_cropped = 0;

    pagesz = (xe-xb+1)*(ye-yb+1)*(ze-zb+1);
    im_cropped_sz[0] = xe-xb+1;
    im_cropped_sz[1] = ye-yb+1;
    im_cropped_sz[2] = ze-zb+1;
    im_cropped_sz[3] = 1;
    cout<<"pagesz = "<<pagesz<<endl;

    cout<<"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"<<endl;
    try {im_cropped = new unsigned char [pagesz];}
    catch(...)  {v3d_msg("cannot allocate memory for image_mip."); return false;}

    cout<<"bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb"<<endl;

    im_cropped = callback.getSubVolumeTeraFly(inimg_file.toStdString(),xb,xe+1,
                                              yb,ye+1,zb,ze+1);

    cout<<"cccccccccccccccccccccccccccccccc"<<endl;

    QString outimg_file = fileOpenName +".v3draw";
    simple_saveimage_wrapper(callback, outimg_file.toStdString().c_str(),(unsigned char *)im_cropped,im_cropped_sz,1);
    if(im_cropped) {delete []im_cropped; im_cropped = 0;}

    QList<NeuronSWC> block_list;
    for(V3DLONG i = 0;i < nt.listNeuron.size();i++)
    {
        NeuronSWC cur;
        cur.n = nt.listNeuron[i].n;
        cur.r = nt.listNeuron[i].r;
        cur.parent = nt.listNeuron[i].parent;
        cur.type = nt.listNeuron[i].type;
        cur.x = nt.listNeuron[i].x - block_sz[0];
        cur.y = nt.listNeuron[i].y - block_sz[1];
        cur.z = nt.listNeuron[i].z - block_sz[2];

        block_list.push_back(cur);
    }

    QString filename = fileOpenName + "_result.swc";
    NeuronTree nt_result;
    nt_result.listNeuron = block_list;
    writeSWC_file(filename,nt_result);

    return true;

}


bool get_sub_block(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent)
{
    return true;
}

