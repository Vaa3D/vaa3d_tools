#include <vector>
#include <v3d_interface.h>
#include <QAxObject>
#include <QDebug>
#include <math.h>
#include <fstream>
#define PI 3.14159265
using namespace std;

bool restore_C3_Images_to_C2(V3DPluginCallback2 &callback, QWidget *parent);
void save_Parameter_to_Txt(double extensionCoff, double angleCentralPoint, double angleRotation, QString saveImageDir);
bool restore_C3_Image(V3DPluginCallback2 &callback, double extensionCoff, QString imagePath, QString saveImageDir);


bool restore_C3_Images_to_C2(V3DPluginCallback2 &callback, QWidget *parent)
{
    double extensionCoff = 0, angleCentralPoint_2pi = 0, angleRotation_2pi = 0;
    double angleCentralPoint_360 = 0, angleRotation_360 = 0;
    int overlap_C2 = 297, overlap_C3 = 299;
    int shift_C2 = 10, shift_C3 = 14;
//    QString imagePath1 = QFileDialog::getOpenFileName(parent, "Select Image1", "D:\\");
//    qDebug()<<"ImagePath1: "<<imagePath1.toStdString().c_str();
//    QString saveImageDir = QFileDialog::getExistingDirectory(parent, "Save Images", "D:\\");

    extensionCoff = sqrt((shift_C3*shift_C3+(1920-overlap_C3)*(1920-overlap_C3)+0.0)/(shift_C2*shift_C2+(1920-overlap_C2)*(1920-overlap_C2)+0.0));
    angleCentralPoint_2pi = (atan((shift_C2+0.0)/(1920-overlap_C2)));
    angleCentralPoint_360 = angleCentralPoint_2pi/2/PI*360;
    angleRotation_2pi = atan((shift_C3+0.0)/(1920-overlap_C3))-angleCentralPoint_2pi;
    angleRotation_360 = angleRotation_2pi/2/PI*360;
    qDebug()<<"extensionCoff = "<<extensionCoff;
    qDebug()<<"angleCentralPoint_2pi = "<<angleCentralPoint_2pi;
    qDebug()<<"angleRotation_2pi  = "<<angleRotation_2pi;
    qDebug()<<"angleCentralPoint_360  = "<<angleCentralPoint_360;
    qDebug()<<"angleRotation_360 = "<<angleRotation_360;

//    save_Parameter_to_Txt(extensionCoff, angleCentralPoint, angleRotation, saveImageDir);
//    restore_C3_Image(callback, extensionCoff, imagePath1, saveImageDir);



    return 1;
}

void save_Parameter_to_Txt(double extensionCoff, double angleCentralPoint, double angleRotation, QString saveImageDir)
{

    QString paraFileName = saveImageDir + "\\parameters.txt";
    ofstream outfile;
    outfile.open(paraFileName.toStdString().c_str(), ios::trunc);
    outfile<<"extensionCoff = "<<extensionCoff<<"\n";
    outfile<<"angleCentralPoint = "<<angleCentralPoint<<"\n";
    outfile<<"angleRotation = "<<angleRotation<<"\n";
    outfile.close();
}

bool restore_C3_Image(V3DPluginCallback2 &callback, double extensionCoff, QString imagePath, QString saveImageDir)
{
    V3DLONG in_sz[4] = {0};
    int datatype = 0;
    unsigned char *pdata = 0;
    unsigned short *data1d_copy = 0;
    if(!simple_loadimage_wrapper(callback, imagePath.toStdString().c_str(), pdata, in_sz, datatype))
    {
        qDebug()<<"Error Image! Input Again!";
    }

    qDebug()<<"sz[0] = "<<in_sz[0];
    qDebug()<<"sz[1] = "<<in_sz[1];
    qDebug()<<"sz[2] = "<<in_sz[2];
    qDebug()<<"sz[3] = "<<in_sz[3];
    qDebug()<<"datatype = "<<datatype;

    if(data1d_copy){delete []data1d_copy; data1d_copy = 0;}
    data1d_copy = new unsigned short[in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3]*datatype/2];
    memcpy(data1d_copy, pdata, in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3]*datatype);

    unsigned int outWidth = (unsigned int)(in_sz[0]*extensionCoff+0.5);
    unsigned int outHeight = (unsigned int)(in_sz[1]*extensionCoff+0.5);
    qDebug()<<__LINE__<<": outWidth = "<<outWidth;
    qDebug()<<__LINE__<<": outHeight = "<<outHeight;
    unsigned short *data1dOut = new unsigned short[outWidth*outHeight];

    double x = 0, y = 0;
    for(unsigned int j = 0; j < outHeight-extensionCoff; j ++)
    {
        y = j/extensionCoff;
        for(unsigned int i = 0; i < outWidth; i ++)
        {
            int grayValue = 0;
            x = i/extensionCoff;
            int x1, x2, y1, y2;
            x1 = (int)x;
            x2 = x1 + 1;
            y1 = (int)y;
            y2 = y1 + 1;

            double u, v;
            u = x - x1;
            v = y - y1;

            if((x >= (in_sz[0] - 1)) && (y >= (in_sz[1] - 1)))
            {
                grayValue = data1d_copy[y1*in_sz[0]+x1];
            }
            else if(x >= (in_sz[0] - 1))
            {
                int value1 = data1d_copy[y1*in_sz[0]+x1];
                int value2 = data1d_copy[y1*in_sz[0]+x2];
                grayValue = value1 * (1-u) + value2 * u;
            }
            else if(x >= (in_sz[1] - 1))
            {
                int value1 = data1d_copy[y1*in_sz[0]+x1];
                int value2 = data1d_copy[y2*in_sz[0]+x1];
                grayValue = value1 * (1-u) + value2 * u;
            }
            else
            {
                int value1 = data1d_copy[y1*in_sz[0]+x1];
                int value2 = data1d_copy[y2*in_sz[0]+x1];
                int value3 = data1d_copy[y1*in_sz[0]+x2];
                int value4 = data1d_copy[y2*in_sz[0]+x2];
                int grayValue1, grayValue2;
                grayValue1 = value1 * (1-u) + value2 * u;
                grayValue2 = value3 * (1-u) + value4 * u;
                grayValue = grayValue1 * (1-v) + grayValue2 * v;
            }
            data1dOut[j*in_sz[0]+i] = grayValue;
        }
    }

    QString imageOriName = saveImageDir + "\\originalImage.v3draw";
    V3DLONG in_sz1[4] = {outWidth, outHeight, in_sz[2], in_sz[3]};
    unsigned char *pdataOri = new unsigned char[in_sz1[0]*in_sz1[1]*in_sz1[2]*in_sz1[3]*datatype];
    memcpy(pdataOri, data1dOut, in_sz1[0]*in_sz1[1]*in_sz1[2]*in_sz1[3]*datatype);

    if(!simple_saveimage_wrapper(callback, imageOriName.toStdString().c_str(), pdataOri, in_sz1, datatype))
    {
        qDebug()<<"Failed to save original image !";
    }

    int diff1 = outWidth-in_sz[0];
    int diff2 = outHeight-in_sz[1];
    unsigned short *data1dOut_Cut = new unsigned short[in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3]*datatype/2];
    for(V3DLONG i = 0; i < in_sz[0]; i ++)
    {
        for(V3DLONG j = 0; j < in_sz[1]; j ++)
        {
            data1dOut_Cut[j*in_sz[0]+i] = data1dOut[(j+diff2/2)*in_sz1[0]+(i+diff1/2)];
        }
    }

    QString imageCutName = saveImageDir + "\\cutImage.v3draw";
    unsigned char *pdataCut = new unsigned char[in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3]*datatype];
    memcpy(pdataCut, data1dOut_Cut, in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3]*datatype);
    if(!simple_saveimage_wrapper(callback, imageCutName.toStdString().c_str(), pdataCut, in_sz, datatype))
    {
        qDebug()<<"Failed to save cutted image !";
    }






    return 1;
}

















