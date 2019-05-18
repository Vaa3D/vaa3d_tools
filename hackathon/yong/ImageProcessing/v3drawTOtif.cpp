#include "v3drawTOtif.h"

bool v3drawTOtif(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{

}


bool v3drawTOtif(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    QString inimage = infiles[0];
    cout<<"inimageName :"<<inimage.toStdString()<<endl;

    QString temp_filename = inimage.split(".").first();
    //cout<<temp_filename.toStdString()<<endl;

    QString outimage = temp_filename + ".tif";
    unsigned char * data1d = 0;
    V3DLONG in_sz[4];
    int dataType;
    if(!simple_loadimage_wrapper(callback, inimage.toStdString().c_str(), data1d, in_sz, dataType))
    {
        cerr<<"load image "<<inimage.toStdString()<<" error!"<<endl;
        return false;
    }

    if(!simple_saveimage_wrapper(callback,outimage.toStdString().c_str(),data1d,in_sz,1))
    {
        cerr<<"save image "<<outimage.toStdString()<<" error!"<<endl;
        return false;
    }

    return true;

}
