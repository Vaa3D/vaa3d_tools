#include "GMM_Algorithm.h"

const QString title = QObject::tr("ImageProcessing");

void gmm(V3DPluginCallback2 &callback,QWidget *parent,input_PARA &PARA)
{
    v3dhandle curwin=callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    Image4DSimple *p4DImage=callback.getImage(curwin);
    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }
    unsigned char* data1d = p4DImage->getRawData();
    QString fileName=QString(QLatin1String(p4DImage->getFileName()));
    v3d_msg(fileName,0);
    v3d_msg(QString("datatype=%1").arg(int(p4DImage->getDatatype())), 0);

    //input
    V3DLONG c=1;
    V3DLONG in_sz[4];
    in_sz[0] = p4DImage->getXDim();
    in_sz[1] = p4DImage->getYDim();
    in_sz[2] = p4DImage->getZDim();
    in_sz[3] = p4DImage->getCDim();

    float * outimg = 0;
    bool isOk;

    switch(p4DImage->getDatatype())
    {
    case V3D_UINT8:
        isOk = gmmFilter(fileName,data1d, in_sz, c-1,outimg,parent);
        break;
    case V3D_UINT16:
        isOk = gmmFilter(fileName,(unsigned short int *)data1d, in_sz, c-1, outimg,parent);
        break;
    case V3D_FLOAT32:
        isOk = gmmFilter(fileName,(float *)data1d, in_sz, c-1, outimg,parent);
        break;
    default:
        isOk = false;
        v3d_msg("Unsupported data type. Do nothing.");
        return;
    }

    if(!isOk){
        v3d_msg("Fail to gmm. Do nothing.");
        return;
    }

    // display
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData((unsigned char *)outimg, in_sz[0], in_sz[1], in_sz[2], 1, V3D_FLOAT32);
    //new4DImage->setFileName("C:/Users/admin/Desktop/wp_data..v3draw.v3dpbd");
    new4DImage->setFileName("/home/yong/Desktop/false_connection_examples/041..v3draw.v3dpbd");
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, new4DImage);
    callback.setImageName(newwin, title);
    callback.updateImageWindow(newwin);
}

bool gmm(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
{
    return true;
}


template <class T> bool gmmFilter(QString fileName,T* data1d,V3DLONG *in_sz,V3DLONG c,float* &outimg,QWidget *parent)
{
    if(!data1d||!in_sz||c<0||outimg)
    {
        return false;
    }
    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    V3DLONG pagesz = N*M;
    V3DLONG channelsz = pagesz*P;

    V3DLONG offset_init = c*channelsz;

    try
    {
        outimg = new float [channelsz];
    }
    catch (...)
    {
        printf("Fail to allocate memory.\n");
        return false;
    }

    T *curdata1d = data1d + offset_init;

    for(V3DLONG i=0;i<N;i++)
    {
        for(V3DLONG j=0;j<M;j++)
        {
            for(V3DLONG k=0;k<P;k++)
            {
                outimg[i*pagesz+j*N+k]=data1d[i*pagesz+j*N+k]/2;
            }
        }
    }

    //exportGrayValueToTxt
    QString rawStr=fileName.split(".")[0];
    QFile data(rawStr+"_gray.txt");
    if(!data.open(QFile::WriteOnly | QIODevice::Text))
    {
        return -1;
    }
    QTextStream out(&data);
    float _min=100000000;
    float _max=-1;
    for(V3DLONG i=0;i<N*M*P;i++)
    {
        if(data1d[i]>_max) _max=data1d[i];
        if(data1d[i]<_min) _min=data1d[i];
        out<<data1d[i]<<"\n";
    }
    data.close();
    v3d_msg(QString("_min=%1 _max=%2").arg(_min).arg(_max), 0);

    QFile classByGmmFile("C:/Users/admin/Desktop/ImageBgDetect_gmm.txt");

    QStringList args;
    args.append("D:/wpkenan/Project/Pycharm/20180924/gmmSklearn.py");
    args.append(rawStr+"_gray.txt");
    QProcess::execute(QString("Python.exe"), args);

    V3DLONG threshold = 0;
    threshold=QInputDialog::getInteger(parent, "Channel",
        "Enter threshold # (starts from 0):",
        0, 0, 100, 1);

    QFile inputFile(rawStr+"_gray_gmm.txt");
    if (inputFile.open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QTextStream in(&inputFile);
        int i=0;
        while (!in.atEnd())
        {
            QString line = in.readLine();

            if(line.toInt()>=threshold)
            {
                outimg[i++]=line.toInt()*20;
            }
            else
            {
                outimg[i++]=0;
            }
        }
        inputFile.close();
    }

    return true;
}
