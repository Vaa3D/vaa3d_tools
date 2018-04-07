#ifndef DATA_TRAINING_H
#define DATA_TRAINING_H

#include <QtGui>
#include <vector>
#include <v3d_interface.h>
#include <iostream>
using namespace std;

class histogramDialog : public QDialog
{
    Q_OBJECT

public:
    histogramDialog(QVector< QVector<int> >& vvec, QStringList labelsOfLeftTop, QString labelOfRightBottom,
            QWidget *parent=0, QSize figSize=QSize(500, 150), QColor barColor=QColor(100,100,100));
//	virtual ~histogramDialog(){};
//	virtual void closeEvent( QCloseEvent* e);
    int nChannel;
    QVector<QVector<int> > data;
    QLabel** labelPicture;
    QColor barColor;
    QSize figSize;
    QPushButton* normalButton;
    QPushButton* logButton;

public slots:
    void updateBar();
};
template <class T> bool getHistogram(const T * pdata1d, V3DLONG datalen, double max_value, V3DLONG & histscale, QVector<int> &hist)
{
    // init hist
   // cout<<"datalen = "<<datalen<<endl;
    hist = QVector<int>(histscale, 0);
    //cout<<"max_valve = "<<max_value<<endl;
    //cout<<"histscale = "<<histscale<<endl;
    for (V3DLONG i=0;i<datalen;i++)
    {
        //std::cout<<"pdata1d[i] = "<<int(pdata1d[i])<<endl;
        V3DLONG ind = pdata1d[i]/max_value * histscale;
        //V3DLONG ind = pdata1d[i];
        //std::cout<<"ind = "<<ind<<endl;
        hist[ind] ++;
    }
    //std::cout<<"hist = "<<hist[0]<<endl;
    return true;

}

QImage drawBarFigure(QVector<int>& vec, QColor barColor=QColor(100,100,100));
int data_training(V3DPluginCallback2 &callback, QWidget *parent);
bool data_training(V3DPluginCallback2 &callback, const V3DPluginArgList & input, V3DPluginArgList & output);
void bubble_sort(vector<double> &dis_v);
void bubble_sort(double* dis_v,int v_size);
QMap<double,int> KNN(vector<double> &entrople_1,vector<double> &entrople_2,vector<double> &entrople_sample,int N);

QStringList importFileList_addnumbersort(const QString & curFilePath);

#endif // DATA_TRAINING_H
