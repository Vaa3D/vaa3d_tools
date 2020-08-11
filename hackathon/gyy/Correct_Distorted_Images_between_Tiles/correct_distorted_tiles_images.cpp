#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <math.h>
#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

bool Correct_Distorted_Images(V3DPluginCallback2 &callback, QWidget *parent);
bool calculate_overlap_images(V3DPluginCallback2 &callback, QString imagePath1, QString imagePath2, QString orientation, vector<int> &XY_overlap);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);

bool Correct_Distorted_Images(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString imagePath1 = QFileDialog::getOpenFileName(parent, "Select Image1", "D:\\");
    qDebug()<<"ImagePath1: "<<imagePath1.toStdString().c_str();
    QString imagePath2 = QFileDialog::getOpenFileName(parent, "Select Image2", imagePath1);
    qDebug()<<"ImagePath2: "<<imagePath2.toStdString().c_str();
    QString saveImageDir = QFileDialog::getExistingDirectory(parent, "Save Images", "D:\\");
    qDebug()<<"saveImageDir: "<<saveImageDir.toStdString().c_str();

    QFileInfo imageName1(imagePath1);
    QFileInfo imageName2(imagePath2);
    string sname1 = imageName1.baseName().toStdString();
    string s1 = sname1.substr(10, 2);
    string sname2 = imageName2.baseName().toStdString();
    string s2 = sname2.substr(10, 2);

    QString orientation1, orientation2;
    if(!setOrientation(orientation1, orientation2, s1, s2))
    {
        qDebug()<<__LINE__;
        return 0;
    }
    qDebug()<<__LINE__;
    cout<<"Orientation1 = "<<orientation1.toStdString().c_str()<<endl;
    cout<<"Orientation2 = "<<orientation2.toStdString().c_str()<<endl;

    const int segNum = 50;
    vector<int> XY_overlap(segNum, 0);
    if(!calculate_overlap_images(callback, imagePath1, imagePath2, orientation1, XY_overlap))
    {
        qDebug()<<"Calculating images overlap is wrong !";
    }

    vector<int> extraImg(segNum-1, 0);
    vector<double> extraPhy(segNum-1, 0);
    vector<double> pressScale(segNum-1, 0);
    for(int i = 0; i < segNum-1; i ++)
    {
        extraImg[i] = XY_overlap[i+1] - XY_overlap[0];
        extraPhy[i] = (1920*extraImg[i]+0.0)/(3840-extraImg[i]-XY_overlap[0]);
        pressScale[i] = (1920+extraPhy[i]+0.0)/1920;
    }

    string sname = "T"+s1+"-T"+s2;
    QString qname1 = QString :: fromStdString(sname);
    QString paraFileName = saveImageDir + "\\2image_overlap_shift_" + qname1 + ".txt";
    qDebug()<<"paraFileName: "<<paraFileName.toStdString().c_str();

    ofstream outfile;
    outfile.open(paraFileName.toStdString().c_str(), ios::trunc);
    outfile<<"XY_overlap[0] = "<< XY_overlap[0]<<"\n";
    for(int i = 0; i < segNum-1; i ++)
    {
        outfile<<"XY_overlap["<<i+1<<"] = "<<XY_overlap[i+1]<<"  \t"<<"extraImg["<<i<<"] = "<<extraImg[i]<<"  \t"
              <<"extraPhy["<<i<<"] = "<<setiosflags(ios::left)<<setw(10)<<setprecision(6)<<extraPhy[i]<<"  \t"<<"compressionRatio["<<i<<"] = "<<setiosflags(ios::left)<<setw(10)<<setprecision(6)<<pressScale[i]<<"\n";
    }
    outfile.close();
}

bool calculate_overlap_images(V3DPluginCallback2 &callback, QString imagePath1, QString imagePath2, QString orientation, vector<int> &XY_overlap)
{
    V3DLONG in_sz1[4] = {0};
    int datatype1 = 0;
    unsigned char *pdata1 = 0;
    unsigned short *data1d_copy1 = 0;
    if(!simple_loadimage_wrapper(callback, imagePath1.toStdString().c_str(), pdata1, in_sz1, datatype1))
    {
        qDebug()<<"Error Image1! Input Again!";
    }

    if(data1d_copy1){delete []data1d_copy1; data1d_copy1 = 0;}
    data1d_copy1 = new unsigned short[in_sz1[0]*in_sz1[1]*in_sz1[2]*in_sz1[3]*datatype1/2];
    memcpy(data1d_copy1, pdata1, in_sz1[0]*in_sz1[1]*in_sz1[2]*in_sz1[3]*datatype1);

    V3DLONG in_sz2[4] = {0};
    int datatype2 = 0;
    unsigned char *pdata2 = 0;
    unsigned short *data1d_copy2 = 0;
    if(!simple_loadimage_wrapper(callback, imagePath2.toStdString().c_str(), pdata2, in_sz2, datatype2))
    {
        qDebug()<<"Error Image2! Input Again!";
    }

    if(data1d_copy2){delete []data1d_copy2; data1d_copy2 = 0;}
    data1d_copy2 = new unsigned short[in_sz2[0]*in_sz2[1]*in_sz2[2]*in_sz2[3]*datatype2/2];
    memcpy(data1d_copy2, pdata2, in_sz2[0]*in_sz2[1]*in_sz2[2]*in_sz2[3]*datatype2);

    for(int i = 0; i < XY_overlap.size(); i ++)
    {
        int shift = 13;
        const int overlap_range = 50;
        double sum[overlap_range] = {0};
        int difference[overlap_range] = {0};
        int count[overlap_range] = {0};
        double minError = INT_MAX;

        if(orientation == "left" || orientation == "right")
        {
            for(int ss1 = 260; ss1 < 260+overlap_range; ss1 ++)
            {
                int indexOL = ss1 - 260;
                for(int row = i*1920/XY_overlap.size(); row < (i+1)*1920/XY_overlap.size(); row ++)
                {
                    for(int col = 0; col < ss1; col ++)
                    {
                        int diff;
                        if(((row+shift)*1920+col)<1920*1920)
                        {
                            if(orientation == "left")
                                diff = data1d_copy1[row*1920+1920-ss1+col] - data1d_copy2[(row+shift)*1920+col];
                            else
                                diff = data1d_copy2[row*1920+1920-ss1+col] - data1d_copy1[(row+shift)*1920+col];
                            sum[indexOL] += diff * diff;
                            count[indexOL] += 1;
                        }
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap[i] = ss1;
                }
            }
        }

        else if(orientation == "up" || orientation == "down")
        {
            for(int ss1 = 260; ss1 < 260+overlap_range; ss1 ++)
            {
                int indexOL = ss1 - 260;
                for(int row = 0; row < ss1; row ++)
                {
                    for(int col = i*1920/XY_overlap.size(); col < (i+1)*1920/XY_overlap.size(); col ++)
                    {
                        int diff;
                        if(((1920-ss1+row)*1920+shift+col)<1920*1920)
                            diff = data1d_copy1[(1920-ss1+row)*1920+shift+col] - data1d_copy2[row*1920+col];

                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap[i] = ss1;
                }
            }
        }
    }
    return 1;
}

bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2)
{
    cout<<"s1 = "<<s1.c_str()<<endl;
    cout<<"s2 = "<<s2.c_str()<<endl;
    int num11 = std::stoi(s1);
    int num22 = std::stoi(s2);
    cout<<"num1 = "<<num11<<endl;
    cout<<"num2 = "<<num22<<endl;
    if(num11<0 || num11>28 ||num22<0 || num22>28 || num11 == num22)
        return 0;
    int flag = (num11>num22)?1:0;
    int num1 = min(num11, num22);
    int num2 = max(num11, num22);

    if(((num1-1)/5 == (num2-1)/5) && ((num1-1)/5%2 == 0) && (num1+1 == num2))
    {
        orientation1 = "left";
        orientation2 = "right";
    }

    else if(((num1-1)/5 == (num2-1)/5) && ((num1-1)/5%2 == 1) && (num1+1 == num2))
    {
        orientation1 = "right";
        orientation2 = "left";
    }

    else if(((num1-1)/5+1 == (num2-1)/5) && (num1-1)/5<4)
    {
        if(((num1%5 == 1)&&(num2 = num1+9)) || ((num1%5 == 2)&&(num2 = num1+7)) || ((num1%5 == 3)&&(num2 = num1+5))
                || ((num1%5 == 4)&&(num2 = num1+3)) || ((num1%5 == 0)&&(num2 = num1+1)))
        {
            orientation1 = "up";
            orientation2 = "down";
        }
    }

    else if(((num1 == 22)&&(num2 == 28)) || ((num1 == 23)&&(num2 == 27))||((num1 == 24)&&(num2 == 26)))
    {
        orientation1 = "up";
        orientation2 = "down";
    }

    else
        return 0;

    if(flag == 1)
    {
        QString temp = orientation1;
        orientation1 = orientation2;
        orientation2 = temp;
    }
    return 1;
}
