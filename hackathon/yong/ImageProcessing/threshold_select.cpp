#include "threshold_select.h"

void threshold_select(V3DPluginCallback2 &callback, QWidget *parent, input_PARA &PARA)
{

    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    //get image gray
    //v3d_msg(QString("datatype=%1").arg(int(p4DImage->getDatatype())), 1); //get data type
    unsigned char* data1d = p4DImage->getRawData();

    V3DLONG c=1;
    V3DLONG in_sz[4];
    in_sz[0]=p4DImage->getXDim();
    in_sz[1]=p4DImage->getYDim();
    in_sz[2]=p4DImage->getZDim();
    in_sz[3]=p4DImage->getCDim();
    V3DLONG pagesz=in_sz[0]*in_sz[1];
    V3DLONG channelsz=in_sz[2]*pagesz; //all size

    //printf gray value on the terminal
//    for(V3DLONG z=0;z<in_sz[2];z++)
//    {
//        for(V3DLONG y=0;y<in_sz[1];y++)
//        {
//            for(V3DLONG x=0;x<in_sz[0];x++)
//            {
//                printf("%d",data1d[z*pagesz+y*in_sz[0]+x]);
//            }
//        }
//    }
//    qDebug()<<endl;

    //create .txt file
    PARA.inimg_file = p4DImage->getFileName();
    QString filename = PARA.inimg_file + "_imagevalue.txt";
    QFile data(filename);

    if(!data.open(QFile::WriteOnly | QIODevice::Text))
    {
        //return -1;
        QMessageBox::information(0, "", "create .txt file failed");
        return;
    }

    QTextStream out(&data);

    V3DLONG N = in_sz[0];
    V3DLONG M = in_sz[1];
    V3DLONG P = in_sz[2];
    V3DLONG sc = in_sz[3];
    //V3DLONG pagesz = N*M;
    //V3DLONG channelsz = pagesz*P;

    float min=100000000;
    float max=-1;

    for(V3DLONG i=0;i<N*M*P;i++)
    {
        if(data1d[i]>max) max=data1d[i];
        if(data1d[i]<min) min=data1d[i];
        out<<data1d[i]<<"\n";
    }
    data.close();

    qDebug()<<"min = "<<min<<"   "<<"max = "<<max<<endl;
    //v3d_msg(QString("min=%1 max=%2").arg(min).arg(max), 1);

    //method1:select threshold by base on gary level and counts
    statisticalProcess(data1d, in_sz, c-1);

    //method2:select threshold by base on imgave and imgstd
    double imgAve, imgStd, bkg_thresh;
    //mean_and_std(p4DImage->getRawDataAtChannel(0),p4DImage->getTotalUnitNumberPerChannel(), imgAve, imgStd);
    mean_and_std(p4DImage->getRawData(), p4DImage->getTotalUnitNumberPerChannel(), imgAve, imgStd);

    double td= (imgStd<10)? 10: imgStd;
    bkg_thresh = imgAve +0.7*td ;

    QString avgAndstd=QString("avg:%1  std:%2").arg(imgAve).arg(imgStd);
    QString threshold=QString("threshold:%1").arg(bkg_thresh);

    qDebug() << "avgAndstd = "<<avgAndstd<<endl;
    qDebug() << "bkg_thresh = "<<bkg_thresh<<endl;

    //v3d_msg(avgAndstd);
    v3d_msg(threshold);


    return;
}

bool threshold_select(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent,input_PARA &PARA)
{
    return true;
}

template <class T1, class T2> bool mean_and_std(T1 *data, V3DLONG n, T2 & ave, T2 & sdev)
{
    if (!data || n<=0)
      return false;

    int j;
    double ep=0.0,s,p;

    if (n <= 1)
    {
      //printf("len must be at least 2 in mean_and_std\n");
      ave = data[0];
      sdev = (T2)0;
      return true; //do nothing
    }

    s=0.0;
    for (j=0;j<n;j++) s += data[j];
    double ave_double=(T2)(s/n); //use ave_double for the best accuracy

    double var=0.0;
    for (j=0;j<n;j++) {
        s=data[j]-(ave_double);
        var += (p=s*s);
    }
    var=(var-ep*ep/n)/(n-1);
    sdev=(T2)(sqrt(var));
    ave=(T2)ave_double; //use ave_double for the best accuracy

    return true;
}


template <class T> bool statisticalProcess(T* data1d,V3DLONG *in_sz,V3DLONG c)
{
    if(!data1d||!in_sz||c<0)
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

    typename std::map<T,V3DLONG> grayCounts;
    for(V3DLONG i=0;i<N*M*P;i++)
    {
        grayCounts[data1d[i]]++;
    }

    QString strTmp=QString("%1 grayLevel in all\n").arg(grayCounts.size());

    for(typename std::map<T,V3DLONG>::iterator iter=grayCounts.begin();iter!=grayCounts.end();iter++)
    {

        strTmp+=QString("%1      %2\n").arg(iter->first).arg(iter->second);

    }

    v3d_msg(strTmp);
    return true;
}
