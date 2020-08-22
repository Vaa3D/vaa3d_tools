#include <v3d_interface.h>
#include <iostream>
#include <fstream>
#include <QDebug>
using namespace std;

bool calculate_2image_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent);
bool calculate_2Img_overlap_shift(unsigned short * pdata1, unsigned short * pdata2, unsigned short * &p_cut_data1, unsigned short * &p_cut_data2, QString orientation, int & XY_overlap, int & XY_shift);
void cut_planes_edge(unsigned short *pdata, unsigned short *&p_cut_data);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);

bool calculate_2image_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString imagePath1 = QFileDialog::getOpenFileName(parent, "Select Image1", "D:\\");
    qDebug()<<"ImagePath1: "<<imagePath1.toStdString().c_str();
    QString imagePath2 = QFileDialog::getOpenFileName(parent, "Select Image2", imagePath1);
    qDebug()<<"ImagePath2: "<<imagePath2.toStdString().c_str();
    QString saveImageDir = QFileDialog::getExistingDirectory(parent, "Save Images", imagePath1);
    qDebug()<<"saveImageDir: "<<saveImageDir.toStdString().c_str();
    int XY_overlap, XY_shift;

    unsigned short * p_cut_data1 = 0;
    unsigned short * p_cut_data2 = 0;

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

    if(!calculate_2Img_overlap_shift(data1d_copy1, data1d_copy2, p_cut_data1, p_cut_data2, orientation1, XY_overlap, XY_shift))
    {
        qDebug()<<"Error calculate shift and overlap!";
    }

    qDebug()<<"XY_overlap = "<<XY_overlap;
    qDebug()<<"shift = "<<XY_shift;

    string sname = "T"+s1+"-T"+s2;
    QString qname1 = QString :: fromStdString(sname);
    QString paraFileName = saveImageDir + "\\2image_overlap_shift_" + qname1 + ".txt";
    qDebug()<<"paraFileName: "<<paraFileName.toStdString().c_str();

    ofstream outfile;
    outfile.open(paraFileName.toStdString().c_str(), ios::trunc);

    outfile<<"XY_overlap = "<<XY_overlap<<"\n"<<"shift = "<<XY_shift<<"  \t"<<"\n";
    outfile.close();

    /*
    QString cutImagesFile1 = saveImageDir + "\\" + imageName1.baseName() + "_cut.v3draw";
    QString cutImagesFile2 = saveImageDir + "\\" + imageName2.baseName() + "_cut.v3draw";
    V3DLONG in_sz_times[4] = {(in_sz1[0]-40), (in_sz1[1]-40), in_sz1[2], in_sz1[3]};
    simple_saveimage_wrapper(callback, cutImagesFile1.toStdString().c_str(), (unsigned char *)p_cut_data1, in_sz_times, datatype1);
    simple_saveimage_wrapper(callback, cutImagesFile2.toStdString().c_str(), (unsigned char *)p_cut_data2, in_sz_times, datatype1);
    */
}

bool calculate_2Img_overlap_shift(unsigned short * pdata1, unsigned short * pdata2, unsigned short * &p_cut_data1, unsigned short * &p_cut_data2, QString orientation, int & XY_overlap, int & XY_shift)
{
    cut_planes_edge(pdata1, p_cut_data1);
    cut_planes_edge(pdata2, p_cut_data2);
    cout<<"calculate 2Img overlap shift!"<<endl;
    const int overlap_range = 30, shift_range = 20;
    double sum[overlap_range * shift_range] = {0};
    int difference[overlap_range * shift_range] = {0};
    int count[overlap_range * shift_range] = {0};
    double minError = INT_MAX;

    if(orientation == "left" || orientation == "right")
    {
        for(int ss1 = 240; ss1 < 240+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-240)+ss2;
                for(int row = 0; row < 1880-ss2; row ++)
                {
                    for(int col = 0; col < ss1; col ++)
                    {
                        int diff;
                        if(orientation == "left")
                            diff = p_cut_data1[row*1880+1880-ss1+col]-p_cut_data2[(row+ss2)*1880+col];
                        else
                            diff = p_cut_data2[row*1880+1880-ss1+col]-p_cut_data1[(row+ss2)*1880+col];
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap = ss1;
                    XY_shift = ss2;
                }
            }
        }
    }

    else if(orientation == "up" || orientation == "down")
    {
        for(int ss1 = 240; ss1 < 240+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-240)+ss2;
                for(int row = 0; row < ss1; row ++)
                {
                    for(int col = 0; col < 1880-ss2; col ++)
                    {
                        int diff = p_cut_data1[(1880-ss1+row)*1880+ss2+col]-p_cut_data2[row*1880+col];
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap = ss1;
                    XY_shift = ss2;
                }
            }
        }
    }
    return 1;
}

void cut_planes_edge(unsigned short *pdata, unsigned short * &p_cut_data)
{
    cout<<"cut planes edge!"<<endl;
    if(p_cut_data){delete []p_cut_data; p_cut_data = 0;}
    p_cut_data = new unsigned short[1880 * 1880];
    for(int i = 0; i < 1880; i ++)
        for(int j = 0; j < 1880; j ++)
        {
            p_cut_data[i*1880+j] = pdata[(i+20)*1920+(20+j)];
        }
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
    int num1 = (num11 > num22)?num22:num11;
    int num2 = (num11 > num22)?num11:num22;

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
