#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <iostream>
#include <QAxObject>

bool calculate_2Img_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);
bool calculate_overlap_shift(unsigned short *pdata1, unsigned short *pdata2, QString orientation);


bool calculate_2Img_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString imagePath1 = QFileDialog::getOpenFileName(parent, "Open image1", "D:\\");
    QString imagePath2 = QFileDialog::getOpenFileName(parent, "Open image2", imagePath1);

    QFileInfo imageInfo1(imagePath1);
    QFileInfo imageInfo2(imagePath2);
    string sname1 = imageInfo1.baseName().toStdString();
    string s1 = sname1.substr(10, 2);
    string sname2 = imageInfo2.baseName().toStdString();
    string s2 = sname2.substr(10, 2);

    QString orientation1, orientation2;

    if(!setOrientation(orientation1, orientation2, s1, s2))
    {
        return 0;
    }

    unsigned char * data1d1 = 0;
    unsigned short * data1d_copy1 = 0;
    V3DLONG in_sz1[4];
    int datatype1;
    if (!simple_loadimage_wrapper(callback,imagePath1.toStdString().c_str(), data1d1, in_sz1, datatype1))
    {
        qDebug()<< "Error happens in reading the subject file.";
        return false;
    }
    data1d_copy1 = new unsigned short[in_sz1[0]*in_sz1[1]*in_sz1[2]*in_sz1[3]*datatype1/2];
    memcpy(data1d_copy1, data1d1, in_sz1[0]*in_sz1[1]*in_sz1[2]*in_sz1[3]*datatype1);

    unsigned char * data1d2 = 0;
    unsigned short * data1d_copy2 = 0;
    V3DLONG in_sz2[4];
    int datatype2;
    if (!simple_loadimage_wrapper(callback,imagePath2.toStdString().c_str(), data1d2, in_sz2, datatype2))
    {
        qDebug()<< "Error happens in reading the subject file.";
        return false;
    }
    data1d_copy2 = new unsigned short[in_sz2[0]*in_sz2[1]*in_sz2[2]*in_sz2[3]*datatype2/2];
    memcpy(data1d_copy2, data1d2, in_sz2[0]*in_sz2[1]*in_sz2[2]*in_sz2[3]*datatype2);

    if(!calculate_overlap_shift(data1d_copy1, data1d_copy2, orientation1))
    {
        return false;
    }

    return 1;
}

bool calculate_overlap_shift(unsigned short * pdata1, unsigned short * pdata2, QString orientation)
{
    int XY_overlap = 0, XY_shift = 0;
    const int overlap_range = 30, shift_range = 20;
    double sum[overlap_range * shift_range] = {0};
    int difference[overlap_range * shift_range] = {0};
    int count[overlap_range * shift_range] = {0};
    double minError = INT_MAX;

    if(orientation == "left" || orientation == "right")
    {
        for(int ss1 = 280; ss1 < 280+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-280)+ss2;
                for(int row = 0; row < 1920-ss2; row ++)
                {
                    for(int col = 0; col < ss1; col ++)
                    {
                        int diff = pdata1[row*1920+1920-ss1+col]-pdata2[(row+ss2)*1920+col];
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];
                qDebug()<<__LINE__<<": count["<<ss1<<"* shift_range +"<<ss2<<"] = "<<count[indexOL];
                qDebug()<<__LINE__<<": difference["<<ss1<<"* shift_range +"<<ss2<<"] = "<<difference[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap = ss1;
                    XY_shift = ss2;
                }
            }
            cout<<endl;
        }
        qDebug()<<"XY_overlap = "<<XY_overlap;
        qDebug()<<"XY_shift = "<<XY_shift;
        qDebug()<<" Calculate shift in overlap area is successful !";
    }

    else if(orientation == "up" || orientation == "down")
    {
        for(int ss1 = 280; ss1 < 280+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-280)+ss2;
                for(int row = 0; row < ss1; row ++)
                {
                    for(int col = 0; col < 1920-ss2; col ++)
                    {
                        int diff = pdata1[(1920-ss1+row)*1920+ss2+col]-pdata2[row*1920+col];
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];
                qDebug()<<__LINE__<<": count["<<ss1<<"* shift_range +"<<ss2<<"] = "<<count[indexOL];
                qDebug()<<__LINE__<<": difference["<<ss1<<"* shift_range +"<<ss2<<"] = "<<difference[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_overlap = ss1;
                    XY_shift = ss2;
                }
            }
            cout<<endl;
        }
        qDebug()<<"XY_overlap = "<<XY_overlap;
        qDebug()<<"XY_shift = "<<XY_shift;
        qDebug()<<" Calculate shift in overlap area is successful !";
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

    else if(((num1 == 22)&&(num2 == 28)) || ((num1 == 23)&&(num2 == 27))||((num1 == 21)&&(num2 == 26)))
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
