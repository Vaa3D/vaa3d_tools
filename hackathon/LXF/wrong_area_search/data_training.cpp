
/* histogram_func.cpp
 * Display histogram of the image
 * 2012-03-01 : by Jianlong Zhou
 * 2012-03-22 : change histogram comp method, by Yinan Wan
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "data_training.h"
#include <vector>
#include <iostream>
//#include "histogram_gui.h"
#include "volimg_proc.h"
#include <math.h>
#include<cmath>
//#include "histogram_gui.h"
//#include "stackutil.h"
#include <math.h>
#define INF 1.0e300
#define DIS(a,b) (sqrt(((a)-(b))*((a)-(b))))
using namespace std;

const QString title = QObject::tr("Histogram");

/*******************************************************
 * Split a string into string array
 * 1. args should be 0
 * 2. release args if not used any more
 *******************************************************/
int split(const char *paras, char ** &args)
{
    if(paras == 0) return 0;
    int argc = 0;
    int len = strlen(paras);
    int posb[2048];
    char * myparas = new char[len];
    strcpy(myparas, paras);
    for(int i = 0; i < len; i++)
    {
        if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
        {
            posb[argc++]=i;
        }
        else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') &&
                (myparas[i] != ' ' && myparas[i] != '\t'))
        {
            posb[argc++] = i;
        }
    }

    args = new char*[argc];
    for(int i = 0; i < argc; i++)
    {
        args[i] = myparas + posb[i];
    }

    for(int i = 0; i < len; i++)
    {
        if(myparas[i]==' ' || myparas[i]=='\t')myparas[i]='\0';
    }
    return argc;
}


bool each_class(V3DPluginCallback2 & callback,char * folder_path,char* outfile,QVector<QVector<int> > &hist_vec,vector<double> &entrople);


int data_training(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin;
    curwin=callback.currentImageWindow();
    if(!curwin)
    {
        v3d_msg("No V3D window is available for returning data ... Do nothing.", 0);
        return -1;
    }

    Image4DSimple *p4DImage = callback.getImage(curwin);
    if (p4DImage->getDatatype()!=V3D_UINT8)
    {
        v3d_msg("Now we only support 8 bit image.\n");
        return -1;
    }

    //TODO add datatype judgment
    double max_value = 256;
    V3DLONG histscale = 256;
    QVector<QVector<int> > hist_vec;
    QStringList labelsLT;

    int nChannel = p4DImage->getCDim();
    V3DLONG sz[3];
    sz[0] = p4DImage->getXDim();
    sz[1] = p4DImage->getYDim();
    sz[2] = p4DImage->getZDim();

    for (int c=0;c<nChannel;c++)
    {
        unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);
        QVector<int> tmp;
        getHistogram(inimg1d, sz[0]*sz[1]*sz[2], max_value, histscale, tmp);
        hist_vec.append(tmp);
        labelsLT.append(QString("channel %1").arg(c+1));
    }
    QString labelRB = QString("%1").arg(max_value);


    histogramDialog * dlg = new histogramDialog(hist_vec, labelsLT, labelRB, parent, QSize(500,150), QColor(50,50,50));
    dlg->setWindowTitle(QObject::tr("Histogram"));
    dlg->show();

    return 1;
}




bool data_training(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output)
{
    cout<<"Welcome to histogram"<<endl;
    if(output.size() != 1) return true;

    vector<char*>* inlist = (vector<char*>*) (input.at(0).p);
    char * folder_path_1;
    char * folder_path_2;
    char * folder_path_3;
    char * folder_path_4;
    if (inlist->size() != 4)
    {
        cerr<<"You must specify 4 input file!"<<endl;
        return false;
    }
    //char * infile = inlist->at(0);
    QVector<QVector<int> > hist_vec_1;
    QVector<QVector<int> > hist_vec_2;
    QVector<QVector<int> > hist_vec_3;
    QVector<QVector<int> > hist_vec_4;

    vector<double> entrople_1;
    vector<double> entrople_2;
    vector<double> entrople_3;
    vector<double> entrople_4;




    //Read the output file
    vector<char*>* outlist = (vector<char*>*) (output.at(0).p);
    if (outlist->size()!=4)
    {
        cerr<<"You must specify 4 output file!"<<endl;
        return false;
    }
    char* outfile_1 = outlist->at(0);
    char* outfile_2 = outlist->at(1);
    char* outfile_3 = outlist->at(2);
    char* outfile_4 = outlist->at(3);
   // cout<<"output file: "<<outfile<<endl;



    folder_path_1 = inlist->at(0);
    folder_path_2 = inlist->at(1);
    folder_path_3 = inlist->at(2);
    folder_path_4 = inlist->at(3);

    each_class(callback,folder_path_1,outfile_1,hist_vec_1,entrople_1);
    each_class(callback,folder_path_2,outfile_2,hist_vec_2,entrople_2);
    each_class(callback,folder_path_3,outfile_3,hist_vec_3,entrople_3);
    each_class(callback,folder_path_4,outfile_4,hist_vec_4,entrople_4);
    double sum1=0;
    double sum2=0;
    double sum3=0;
    for(V3DLONG i=0;i<entrople_1.size();i++)
    {
        cout<<"entrople_1 = "<<entrople_1[i]<<endl;
        sum1 = sum1 + entrople_1[i];
    }
    for(V3DLONG i=0;i<entrople_2.size();i++)
    {
        //cout<<"entrople_2 = "<<entrople_2[i]<<endl;
        sum2 = sum2 + entrople_2[i];
    }
    for(V3DLONG i=0;i<entrople_3.size();i++)
    {
        sum3 = sum3 + entrople_3[i];
    }
    sum1 = sum1/entrople_1.size();
    sum2 = sum2/entrople_2.size();
    sum3 = sum3/entrople_3.size();
    cout<<"sum1 = "<<sum1<<endl;
    cout<<"sum2 = "<<sum2<<endl;
    cout<<"sum3 = "<<sum3<<endl;
    cout<<"entrople1.size = "<<entrople_1.size()<<endl;
    cout<<"entrople2.size = "<<entrople_2.size()<<endl;
    cout<<"entrople3.size = "<<entrople_3.size()<<endl;

    cout<<"entrople1 = "<<entrople_1[0]<<endl;
    cout<<"entrople2 = "<<entrople_2[0]<<endl;
    cout<<"entrople3 = "<<entrople_3[0]<<endl;

    vector<double> entrople_sample;
    int N = 1;
    int count_all1=0;
    int count_all2=0;
    double count_all;


    QMap<double,int> sample,predict;
    for(V3DLONG i=0;i<entrople_3.size();i++)
    {
        predict[entrople_3[i]]=1;
    }
    for(V3DLONG i=0;i<entrople_4.size();i++)
    {
        predict[entrople_4[i]]=0;
    }


    for(V3DLONG i=0;i<entrople_3.size();i++)
    {
        entrople_sample.push_back(entrople_3[i]);
    }
    for(V3DLONG i=0;i<entrople_4.size();i++)
    {
        entrople_sample.push_back(entrople_4[i]);
    }

    sample = KNN(entrople_1,entrople_2,entrople_sample,N);
    cout<<"111sample = "<<sample.size()<<endl;
    for(V3DLONG i=0;i<entrople_3.size();i++)
    {
        if(sample[entrople_3[i]] == 1)
        {
            count_all1++;
            cout<<"count_all1 = "<<count_all1<<endl;
        }
    }
    cout<<"222sample = "<<sample.size()<<endl;
    for(V3DLONG i=0;i<entrople_4.size();i++)
    {
        if(sample[entrople_4[i]] == 0)
        {
            count_all2++;
            //cout<<"count_all2 = "<<count_all2<<endl;
        }
    }
    cout<<"333sample = "<<sample.size()<<endl;
    count_all = count_all1+count_all2;
    cout<<"count_all2 = "<<count_all2<<endl;
    cout<<count_all<<endl;
    double per = count_all/sample.size();


    cout<<"% = "<<per<<endl;

    return true;

}
QMap<double,int> KNN(vector<double> &entrople_1,vector<double> &entrople_2,vector<double> &entrople_sample,int N)
{
    cout<<"this is knn"<<endl;
    bool label;


    vector<double> dis_v;

    double dis1,min_dis1,dis2,min_dis2;
    min_dis1 = 100000;
    min_dis2 = 100000;
    
    QMap<double,int> training,sample;
    QMap<double,double> mark;
    for(V3DLONG i=0;i<entrople_1.size();i++)
    {
        training[entrople_1[i]]=1;
    }
    for(V3DLONG i=0;i<entrople_2.size();i++)
    {
        training[entrople_2[i]]=0;
    }
    for(V3DLONG i=0;i<entrople_sample.size();i++)
    {

        dis_v.clear();
        V3DLONG count1=0;
        V3DLONG count2=0;
        QMap<double,int>::iterator it;
        for(it = training.begin();it != training.end();++it)
        {
            dis1 = DIS(entrople_sample[i],it.key());
            mark.insert(dis1,it.key());
            dis_v.push_back(dis1);

        }

       bubble_sort(dis_v);

      // cout<<dis_v[0]<<"     "<<dis_v[dis_v.size()-1]<<endl;

        for(int k=0;k<N;k++)
        {
            double this_dis = dis_v[k];
            //cout<<this_dis<<endl;
            double u = mark[this_dis];
            if(training[u] == 1)
            {
                count1++;
            }
            else if(training[u] == 0)
            {
                count2++;
            }

        }
        if(count1>count2)
        {
            sample[entrople_sample[i]]=1;
        }
        else
        {

            sample[entrople_sample[i]]=0;
        }
        dis_v.clear();
}



    cout<<"out of knn"<<endl;
    return sample;

    
    

}
void bubble_sort(double* dis_v,int v_size)
{

    int i,j;
    double t;
    for(i=0;i<v_size-1;i++)
    {

        //temp = dis_v[i];
        for(j = 0;j<v_size-1-i;j++)
        {
            if(dis_v[j]<dis_v[j+1])
            {

                t = dis_v[j];
                dis_v[j] = dis_v[j+1];
                dis_v[j+1] = t;
            }

        }
        //cout<<"out_first_for"<<endl;

    }
    //cout<<"out_bubble"<<endl;
    //return dis_v;

}


void bubble_sort(vector<double> &dis_v)
{
    //cout<<"bubble"<<endl;
    int i,j;
    double t;
    int v_size = dis_v.size();
    for(i=0;i<v_size-1;i++)
    {
        for(j = 0;j<v_size-1-i;j++)
        {
            if(dis_v[j]>dis_v[j+1])
            {

                t = dis_v[j];
                dis_v[j] = dis_v[j+1];
                dis_v[j+1] = t;
            }

        }
        //cout<<"out_first_for"<<endl;


    }
    //cout<<"out of bubble"<<endl;
}
bool each_class(V3DPluginCallback2 & callback,char * folder_path,char* outfile,QVector<QVector<int> > &hist_vec,vector<double> &entrople)
{
    unsigned char * inimg1d = NULL;
    QVector<int> tmp;
    QStringList tifList = importFileList_addnumbersort(QString(folder_path));
    cout<<"tiflist_size = "<<tifList.size()<<endl;

    for(V3DLONG i=2;i<tifList.size();i++)
    {
        //cout<<"name = "<<tifList.at(i)<<endl;
       char *infile;
       QByteArray ba = tifList.at(i).toLatin1();
       infile = ba.data();
        //cout<<"input file: "<<infile<<endl;

        V3DLONG sz[4];
        int datatype;
        if (!simple_loadimage_wrapper(callback, infile, inimg1d, sz, datatype))
        {
            cerr<<"faile to load image"<<endl;
            return false;
        }

        if (datatype!=1)
        {
            v3d_msg("Now we only support 8 bit image.\n");
            return -1;
        }

        //TODO add datatype judgment
        double max_value = 256;
        V3DLONG histscale = 256;


        int nChannel = sz[3];
       // cout<<"nchannel = "<<nChannel<<endl;
        tmp.clear();
        for (int c=0;c<nChannel;c++)
        {

            getHistogram(inimg1d, sz[0]*sz[1]*sz[2], max_value, histscale, tmp);
            hist_vec.append(tmp);
        }
    }
    //output histogram to csv file
    FILE *fp;
    fp = fopen(outfile, "w");
 //   cout<<"hang = "<<hist_vec.size()<<endl;
  //  cout<<"lie = "<<hist_vec[0].size()<<endl;
    for (int i=0;i<hist_vec.size();i++)
    {

        for (int j=0;j<hist_vec[i].size();j++)
            fprintf(fp, "%d,", hist_vec[i][j]);
        fprintf(fp,"\n");
    }
    fclose(fp);


    cout<<"******************************calculate entropie*****************************"<<endl;

    vector<double> sum_entr;
    double sum_temp;
    //vector<vector<double> > p_sum;
    //vector<double> p_each;
    double p;
    //double entro_each;
    vector<int> count_all;

    for(V3DLONG i=0;i<hist_vec.size();i++)
    {
        sum_temp=0;
        int  count=0;
        for(V3DLONG j=0;j<hist_vec[i].size();j++)
        {
            sum_temp = hist_vec[i][j]+sum_temp;
            if(hist_vec[i][j]!=0)
            {
                count++;
            }
        }
        //cout<<"sum_temp = "<<sum_temp<<endl;
        sum_entr.push_back(sum_temp);
        count_all.push_back(count);
    }

    for(V3DLONG i=0;i<hist_vec.size();i++)
    {
        //cout<<"count_all = "<<count_all[i]<<endl;
        sum_temp=0;
        for(V3DLONG j=0;j<hist_vec[i].size();j++)
        {
            double tmp_=hist_vec[i][j];
            //cout<<"count_all = "<<count_all[i]<<endl;
            p=tmp_/sum_entr[i];

            if(p!=0)
            {
             //   cout<<"p = "<<p<<endl;
                sum_temp = sum_temp + p*log2(p);
            }
        }
        entrople.push_back(-sum_temp);
    }


        if (inimg1d) {delete []inimg1d; inimg1d=NULL;}

}

QStringList importFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();
    // get the iamge files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";

    QDir dir(curFilePath);
    if(!dir.exists())
    {
        cout <<"Cannot find the directory";
        return myList;
    }
    foreach(QString file, dir.entryList()) // (imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }
    //print filenames
    foreach(QString qs, myList) qDebug() << qs;
    return myList;
}

histogramDialog::histogramDialog(QVector< QVector<int> >& vvec, QStringList labelsOfLeftTop, QString labelOfRightBottom,
        QWidget *parent, QSize figSize, QColor barColor)
: QDialog(parent)

{
    setWindowFlags(Qt::Popup /*| Qt::WindowStaysOnTopHint*/ | Qt::Tool); setAttribute(Qt::WA_MacAlwaysShowToolWindow);

    QGridLayout *layout = new QGridLayout;

    nChannel = vvec.size();
    data = vvec;
    barColor = barColor;
    figSize = figSize;
    labelPicture = new QLabel * [nChannel]; //revised by PHC, 2010-05-20
    int row = 1;
    for (int i=0; i<nChannel; i++)
    {
        labelPicture[i] = new QLabel;
        labelPicture[i]->setFrameStyle(QFrame::Box | QFrame::Plain);	labelPicture[i]->setLineWidth(1);

        labelPicture[i]->resize(figSize);

        QVector<int> & vec = data[i];

        V3DLONG imin, imax;
        int vmin, vmax;
        minMaxInVector(vec.data(), vec.size(), imin, vmin, imax, vmax);
        QString desc = QString("max=%1").arg(vmax);

        layout->addWidget(new QLabel(labelsOfLeftTop[i]), row, 0);
        layout->addWidget(new QLabel(desc),               row++, 0, Qt::AlignRight);

        layout->addWidget( labelPicture[i],  row++, 0);

    }
    layout->addWidget(new QLabel(QString("%1").arg(0)),  row, 0);
    layout->addWidget(new QLabel(labelOfRightBottom),    row++, 0, Qt::AlignRight);
    //add option button to display histogram normal or log10
    normalButton = new QPushButton("Display normal histogram");
    logButton = new QPushButton("Display log 10 histogram");
    connect(normalButton, SIGNAL(clicked()), this, SLOT(updateBar()));
    connect(logButton, SIGNAL(clicked()), this, SLOT(updateBar()));
    layout->addWidget(normalButton, row, 0, Qt::AlignLeft);
    layout->addWidget(logButton, row++, 0, Qt::AlignRight);

    layout->setSizeConstraint(QLayout::SetFixedSize); //same as dlg->setFixedSize(dlg->size()); // fixed layout after show

    setLayout(layout);
    //if (labelPicture) {delete []labelPicture; labelPicture=0;} //added by PHC, 2010-05-20

    updateBar();
}

/*histogramDialog::~histogramDialog()
{
    // TODO Auto-generated destructor stub
}
void histogramDialog::closeEvent(QCloseEvent* e)
{
    this->deleteLater();
}
*/

QImage drawBarFigure(QVector<int>& vec, QColor barColor)
{
    //find bound
    int x1,x2, y1,y2;
    x1=x2= 0;
    y1=y2= 0;
    for (int i=0; i<vec.size(); i++)
    {
        int value = vec[i];
        if (value > y2) y2 = value;
    }
    x2 = vec.size()-1;

    QSize size(x2-x1+1, y2-y1+1);
    QImage img(size, QImage::Format_RGB32);

    //paint bar figure
    QPainter p(&img);
    QRect rect(0, 0, size.width(), size.height());
    QBrush shade(QColor(255,255,255));
    p.fillRect(rect, shade);
    for (int i=0; i<vec.size(); i++)
    {
        int value = vec[i];
        QRect rect(i, 0, 1, value);
        QBrush shade( barColor );
        p.fillRect(rect, shade);
    }

    return img.mirrored(); // flip y-direction
}

void histogramDialog::updateBar()
{
    QPushButton* button = (QPushButton*)sender();
    for (int i=0;i<nChannel;i++)
    {
        V3DLONG imin, imax;
        int vmin, vmax;
        QVector<int> vec_rescaled = data[i];
        if (button==logButton)
        {
            for (int j=0;j<vec_rescaled.size();j++)
            {
                if (vec_rescaled[j]!=0)
                    vec_rescaled[j] = log10(double(vec_rescaled[j]))* 1000.0;
            }
        }
        minMaxInVector(vec_rescaled.data(), vec_rescaled.size(), imin, vmin, imax, vmax);
        for (V3DLONG j=0;j<vec_rescaled.size();j++)
            vec_rescaled[j] = vec_rescaled[j]*500.0/vmax;

        QImage fig = drawBarFigure( vec_rescaled, barColor);
        QPicture pic;
        QPainter p;
        p.begin( &pic );
            QRect rectBar = labelPicture[i]->rect();
            p.drawImage( QRect(labelPicture[i]->rect().topLeft(),QSize(rectBar.width()-2, rectBar.height()-2) ), fig );
        p.end();
        labelPicture[i]->setPicture( pic );
    }

}


#undef INF
