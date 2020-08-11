#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <iostream>
#include <QAxObject>
#include "math.h"

bool calculate_overlap_region_projection(V3DPluginCallback2 &callback, QWidget *parent);
bool calculate_project(V3DPluginCallback2 &callback, QFileInfoList imageList, vector<unsigned short *> &pdataCPlane, unsigned short *&pdataPro, QString orientation, int XY_length, int XY_overlapIni, int count);
bool save_projection_files(V3DPluginCallback2 &callback, vector<unsigned short *> pdataPro1, vector<unsigned short *> pdataPro2, QFileInfoList imageList, QString saveTile1[3], QString saveTile2[3]);
bool calculate_overlap_shift(V3DPluginCallback2 &callback, vector<vector<unsigned short *>> &pdataAllPlane1, vector<vector<unsigned short *>> &pdata_CPlane1, vector<vector<unsigned short *>> &pdataAllPlane2,
                        vector<vector<unsigned short *>> &pdata_CPlane2, vector<vector<int>> & XY_overlap, vector<vector<int>> & XY_shift, int XY_length, QString orientation, int Z_shift);
bool calculate_2Img_overlap_shift(unsigned short * pdata1, unsigned short * pdata2, QString orientation, int XY_length, int & XY_ocerlap_I, int & XY_shift_I, int num, int c);
void cut_planes_edge(unsigned short *pdata, unsigned short * &p_cut_data, int XY_length);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);
void calculate_tiles_shift(vector<unsigned short *> pdataPro1, vector<unsigned short *> pdataPro2, QString orientation, int XY_length, int shift_XY[3], int shift_Z[3]);
void calculate_expansion_ratio(vector<vector<int>> XY_overlap, vector<vector<int>> XY_shift, vector<vector<double>> &overlap_shift_Ratio, int XY_length, int Z_shift);
void save_tiles_shift(QWidget *parent, QString saveFolder, string s1, string s2, QString orientation, int shift_XY[3], int shift_Z[3], vector<vector<int> > XY_overlap, vector<vector<int> > XY_shift, vector<vector<double>> overlap_shift_Ratio, int Z_shift);
QAxObject * appendSheet(QAxObject * &worksheets, const QString sheetName);
QString setTitleName(QString saveFolder, string s, QDir dir, QString orientation);
QString setExcelName(QString saveFolder, string s1, string s2, QString orientation);

bool calculate_overlap_region_projection(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString tileFolderFiles1 = QFileDialog::getExistingDirectory(NULL, "Select tile1 folder","D:\\");
    qDebug()<<"Tile1: "<<tileFolderFiles1.toStdString().c_str();
    QString tileFolderFiles2 = QFileDialog :: getExistingDirectory(0, "Select tile2 folder", tileFolderFiles1);
    qDebug()<<"Tile2: "<<tileFolderFiles2.toStdString().c_str();
    QString SaveFolder = QFileDialog :: getExistingDirectory(0, "Select folder to save", "D:\\");
    qDebug()<<"SaveFolder: "<<SaveFolder.toStdString().c_str();

    QDir tileDir1(tileFolderFiles1);
    QDir tileDir2(tileFolderFiles2);
    if(!tileDir1.exists() || !tileDir2.exists())
        return false;

    string sname1 = tileDir1.dirName().toStdString();
    string s1 = sname1.substr(10, 2);
    string sname2 = tileDir2.dirName().toStdString();
    string s2 = sname2.substr(10, 2);

    int XY_length = 1880, XY_overlapIni = 254;
    QString orientation1, orientation2;

    if(!setOrientation(orientation1, orientation2, s1, s2))
    {
        qDebug()<<__LINE__;
        return 0;
    }
    qDebug()<<__LINE__;
    cout<<"Orientation1 = "<<orientation1.toStdString().c_str()<<endl;
    cout<<"Orientation2 = "<<orientation2.toStdString().c_str()<<endl;

    tileDir1.setFilter(QDir::Dirs);
    QFileInfoList channelList1 = tileDir1.entryInfoList();
    tileDir2.setFilter(QDir::Dirs);
    QFileInfoList channelList2 = tileDir2.entryInfoList();

    vector<vector<unsigned short *>> pdataAllPlane1(3, vector<unsigned short *>(1150, 0));
    vector<vector<unsigned short *>> pdataAllPlane2(3, vector<unsigned short *>(1150, 0));
    vector<unsigned short *> pdataPro1(3, 0);
    vector<unsigned short *> pdataPro2(3, 0);

    QDir dir1[3], dir2[3];
    QFileInfoList imageList1[3], imageList2[3];
    QString tileSave1[3], tileSave2[3];
    int count = 0;
    for(int i = 0; i < channelList1.size()-2; i ++)
    {
        qDebug()<<": i = "<<i;
        dir1[i] = channelList1[i+2].filePath();
        dir2[i] = channelList2[i+2].filePath();

        QStringList filters;
        filters << "*.tif" << "*.v3draw";
        imageList1[i] = dir1[i].entryInfoList(filters, QDir::Files);
        imageList2[i] = dir2[i].entryInfoList(filters, QDir::Files);

        count ++;
        if(!calculate_project(callback, imageList1[i], pdataAllPlane1[i], pdataPro1[i], orientation1, XY_length, XY_overlapIni, count))
            return false;
        count ++;
        if(!calculate_project(callback, imageList2[i], pdataAllPlane2[i], pdataPro2[i], orientation2, XY_length, XY_overlapIni, count))
            return false;

        /*
        qDebug()<<__LINE__<<": pdataPro[0][0] = "<<pdataPro1[0][0];
        tileSave1[i] = setTitleName(SaveFolder, s1, dir1[i], orientation1);
        tileSave2[i] = setTitleName(SaveFolder, s2, dir2[i], orientation2);
        */
    }

//    if(!save_projection_files(callback, pdataPro1, pdataPro2, imageList1[0], tileSave1, tileSave2))
//        return false;

    int shift_XY[3] = {0}, shift_Z[3] = {0};
    calculate_tiles_shift(pdataPro1, pdataPro2, orientation1, XY_length, shift_XY, shift_Z);

    int Z_shift = (int)((shift_Z[0]+shift_Z[1]+shift_Z[2]+0.0)/3+0.5);
    vector<vector<unsigned short *>> pdata_CPlane1(3, vector<unsigned short *>(1150 - Z_shift, 0));
    vector<vector<unsigned short *>> pdata_CPlane2(3, vector<unsigned short *>(1150 - Z_shift, 0));
    vector<vector<int>> XY_overlap(3, vector<int>(1150, 0));
    vector<vector<int>> XY_shift(3, vector<int>(1150, 0));
    vector<vector<double>> overlap_shift_Ratio(3, vector<double>(1150, 0));

    calculate_overlap_shift(callback, pdataAllPlane1, pdata_CPlane1, pdataAllPlane2, pdata_CPlane2, XY_overlap, XY_shift, XY_length, orientation1, Z_shift);
    qDebug()<<__LINE__<<": Z_shift = "<<Z_shift;
    calculate_expansion_ratio(XY_overlap, XY_shift, overlap_shift_Ratio, XY_length, Z_shift);
    save_tiles_shift(parent, SaveFolder, s1, s2, orientation1, shift_XY, shift_Z, XY_overlap, XY_shift, overlap_shift_Ratio, Z_shift);

}

bool calculate_project(V3DPluginCallback2 &callback, QFileInfoList imageList, vector<unsigned short *> &pdataCPlane, unsigned short * &pdataPro, QString orientation, int XY_length, int XY_overlapIni, int count)
{
    vector<unsigned char *> data1d_vec(1150);
    vector<V3DLONG *> in_sz_vec(1150);
    vector<int> datatype_vec(1150);
    for(V3DLONG i = 0; i < 1150; i++)
    {
        cout<<"This is "<<i+1<<" image for "<<count<<"."<<endl;
        in_sz_vec[i] = new V3DLONG[4];
        if(count == 1 || count == 2)
        {
            if(!simple_loadimage_wrapper(callback, imageList[i].filePath().toStdString().c_str(), data1d_vec[i], in_sz_vec[i], datatype_vec[i]))
            {
                qDebug()<< "Error happens in reading the subject file.";
                return false;
            }
        }
        else if(count == 3 || count == 4)
        {
            if(!simple_loadimage_wrapper(callback, imageList[i+2].filePath().toStdString().c_str(), data1d_vec[i], in_sz_vec[i], datatype_vec[i]))
            {
                qDebug()<< "Error happens in reading the subject file.";
                return false;
            }
        }
        else
            if(!simple_loadimage_wrapper(callback, imageList[i+12].filePath().toStdString().c_str(), data1d_vec[i], in_sz_vec[i], datatype_vec[i]))
            {
                qDebug()<< "Error happens in reading the subject file.";
                return false;
            }

        unsigned short* pdata_temp= new unsigned short[in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3] *datatype_vec[i]/2];
        memcpy(pdata_temp, data1d_vec[i], in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3]*datatype_vec[i]);
        cut_planes_edge(pdata_temp, pdataCPlane[i], XY_length);
    }

    if(pdataPro){delete []pdataPro; pdataPro = 0;}
    pdataPro = new unsigned short[in_sz_vec[0][0]*1150*datatype_vec[0]/2];

    V3DLONG centerOverlap = 0;
    if(orientation == "left" || orientation == "right")
    {
        if(orientation == "left")
            centerOverlap = XY_length-XY_overlapIni/2;
        else
            centerOverlap = XY_overlapIni/2;

        for(int i = 0; i < 1150; i ++)
        {
            for(int j = 0; j < XY_length; j ++)
            {
                int maxValue = pdataCPlane[i][j*XY_length+centerOverlap-10];
                for(int k = centerOverlap-10; k < centerOverlap+10; k++)
                {
                    maxValue = ((maxValue >= pdataCPlane[i][j*XY_length+k]) ? maxValue : pdataCPlane[i][j*XY_length+k]);
                }
                pdataPro[i*XY_length+j] = maxValue;
            }
        }

    }

    else if(orientation == "up" || orientation == "down")
    {
        if(orientation == "up")
            centerOverlap = XY_length - XY_overlapIni/2;
        else
            centerOverlap = XY_overlapIni/2;

        for(int i = 0; i < 1150; i ++)
        {
            for(int j = 0; j < XY_length; j ++)
            {
                int maxValue = pdataCPlane[i][(centerOverlap-10)*XY_length+j];
                for(int k = centerOverlap-10; k < centerOverlap+10; k ++)
                {
                    maxValue = ((maxValue >= pdataCPlane[i][k*XY_length+j]) ? maxValue : pdataCPlane[i][k*XY_length+j]);
                }
                pdataPro[i*XY_length+j] = maxValue;
            }
        }
    }
    qDebug()<<__LINE__<<": pdataPro[0][0] = "<<pdataPro[0];
}

void calculate_tiles_shift(vector<unsigned short *> pdataPro1, vector<unsigned short *> pdataPro2, QString orientation, int XY_length, int shift_XY[3], int shift_Z[3])
{
    const int XY_shift_range = 5, Z_shift_range = 20;
    if(orientation == "left" || orientation == "right")
    {
        for(int i = 0; i < 3; i ++)
        {
            double sum[XY_shift_range*Z_shift_range] = {0};
            int difference[XY_shift_range*Z_shift_range] = {0};
            int count[XY_shift_range*Z_shift_range] = {0};
            double minError = INT_MAX;
            for(int sz = 0; sz < XY_shift_range; sz ++)
            {
                for(int ss = 0; ss < Z_shift_range; ss ++)
                {
                    for(int row = 0; row < 1150-sz; row ++)
                    {
                        for(int col = 0; col < XY_length-ss; col ++)
                        {
                            int diff;
                            if(orientation == "left")
                                diff = pdataPro1[i][(row+sz)*XY_length+col]-pdataPro2[i][row*XY_length+col+ss];
                            else
                                diff = pdataPro2[i][(row+sz)*XY_length+col]-pdataPro1[i][row*XY_length+col+ss];
                            sum[sz*Z_shift_range+ss] += diff * diff;
                            count[sz*Z_shift_range+ss] +=1;
                        }
                    }
                    difference[sz*Z_shift_range+ss] = sum[sz*Z_shift_range+ss]/count[sz*Z_shift_range+ss];
                    qDebug()<<__LINE__<<": count["<<sz<<"* Z_shift_range +"<<ss<<"] = "<<count[sz*Z_shift_range+ss];
                    qDebug()<<__LINE__<<": difference["<<sz<<"* Z_shift_range +"<<ss<<"] = "<<difference[sz*Z_shift_range+ss];

                    if(minError>difference[sz*Z_shift_range+ss])
                    {
                        minError = difference[sz*Z_shift_range+ss];
                        shift_Z[i] = sz;
                        shift_XY[i] = ss;
                    }
                }
                cout<<endl;
            }
            qDebug()<<"Z shift = "<<shift_Z[i];
            qDebug()<<"XY shift = "<<shift_XY[i];
            qDebug()<<" Calculate Z shift in MIP is successful !";
        }
    }

    else if(orientation == "up" || orientation == "down")
    {
        for(int i = 0; i < 3; i ++)
        {
            double sum[XY_shift_range*Z_shift_range] = {0};
            int difference[XY_shift_range*Z_shift_range] = {0};
            int count[XY_shift_range*Z_shift_range] = {0};
            double minError = INT_MAX;
            for(int sz = 0; sz < XY_shift_range; sz ++)
            {
                for(int ss = 0; ss < Z_shift_range; ss ++)
                {
                    for(int row = 0; row < 1150-sz; row ++)
                    {
                        for(int col = 0; col < XY_length-ss; col ++)
                        {
                            int diff = pdataPro1[i][row*XY_length+col+ss]-pdataPro2[i][(row+sz)*XY_length+col];
                            sum[sz*Z_shift_range+ss] += diff * diff;
                            count[sz*Z_shift_range+ss] ++;
                        }
                    }
                    difference[sz*Z_shift_range+ss] = sum[sz*Z_shift_range+ss]/count[sz*Z_shift_range+ss];
                    qDebug()<<__LINE__<<": count["<<sz<<"* Z_shift_range +"<<ss<<"] = "<<count[sz*Z_shift_range+ss];
                    qDebug()<<__LINE__<<": difference["<<sz<<"* Z_shift_range +"<<ss<<"] = "<<difference[sz*Z_shift_range+ss];

                    if(minError>difference[sz*Z_shift_range+ss])
                    {
                        minError = difference[sz*Z_shift_range+ss];
                        shift_Z[i] = sz;
                        shift_XY[i] = ss;
                    }
                }
                cout<<endl;
            }
            qDebug()<<"Z shift = "<<shift_Z[i];
            qDebug()<<"XY shift = "<<shift_XY[i];
            qDebug()<<" Calculate Z shift in MIP is successful !";
        }
    }
}

bool save_projection_files(V3DPluginCallback2 &callback, vector<unsigned short *> pdataPro1, vector<unsigned short *> pdataPro2, QFileInfoList imageList, QString saveTile1[3], QString saveTile2[3])
{
    unsigned char * data1d = 0;
    V3DLONG in_sz[4]={0};
    int datatype;

    QString file = imageList[0].filePath();
    if(!simple_loadimage_wrapper(callback, file.toStdString().c_str(), data1d, in_sz, datatype))
    {
        qDebug()<< "Error happens in reading the subject file.";
        return false;
    }
    qDebug()<<__LINE__<<": datatype = "<<datatype;

    qDebug()<<__LINE__<<": pdataPro[0][0] = "<<pdataPro1[0][0];
    for(int i = 0; i < 3; i ++)
    {
        // Save tile1
        unsigned char * data1dTile1 = new unsigned char[in_sz[0]*1150*datatype];
        memcpy(data1dTile1, pdataPro1[i], in_sz[0]*1150*datatype);
        V3DLONG in_sz_pro1[4] = {in_sz[0], 1150, 1, 1};
        qDebug()<<__LINE__<<": data1dTile1[0] = "<<data1dTile1[0];
        if(simple_saveimage_wrapper(callback, saveTile1[i].toStdString().c_str(), data1dTile1, in_sz_pro1, datatype))
        {
            qDebug()<<"Save Tile1 successfully!";
        }

        // Save tile2
        unsigned char * data1dTile2 = new unsigned char[in_sz[0]*1150*datatype];
        memcpy(data1dTile2, pdataPro2[i], in_sz[0]*1150*datatype);
        V3DLONG in_sz_pro2[4] = {in_sz[0], 1150, 1, 1};
        qDebug()<<__LINE__<<": data1dTile2[0] = "<<data1dTile2[0];
        if(simple_saveimage_wrapper(callback, saveTile2[i].toStdString().c_str(), data1dTile2, in_sz_pro2, datatype))
        {
            qDebug()<<"Save Tile2 successfully!";
        }
    }

}

bool calculate_overlap_shift(V3DPluginCallback2 &callback, vector<vector<unsigned short *>> &pdataAllPlane1, vector<vector<unsigned short *>> &pdata_CPlane1, vector<vector<unsigned short *>> &pdataAllPlane2,
                        vector<vector<unsigned short *>> &pdata_CPlane2, vector<vector<int> > &XY_overlap, vector<vector<int> > &XY_shift, int XY_length, QString orientation, int Z_shift)
{
    qDebug()<<__LINE__<<": Calculate overlap and shift!";
    for(int c = 0; c < 3; c ++)
    {
        if(orientation == "left")
        {
            qDebug()<<__LINE__<<": orientation = "<<orientation.toStdString().c_str();
            for(int i = 0; i < 1150 - Z_shift; i ++)
            {
                if(pdata_CPlane1[c][i]){delete []pdata_CPlane1[c][i]; pdata_CPlane1[c][i] = 0;}
                pdata_CPlane1[c][i] = new unsigned short[XY_length*XY_length*1*1*2/2];
                memcpy(pdata_CPlane1[c][i], pdataAllPlane1[c][i+Z_shift], XY_length*XY_length*1*1*2);

                if(pdata_CPlane2[c][i]){delete []pdata_CPlane2[c][i]; pdata_CPlane2[c][i] = 0;}
                pdata_CPlane2[c][i] = new unsigned short[XY_length*XY_length*1*1*2/2];
                memcpy(pdata_CPlane2[c][i], pdataAllPlane2[c][i], XY_length*XY_length*1*1*2);
            }
            for(int i = 0; i < 1150-Z_shift; i ++)
            {
                if(!calculate_2Img_overlap_shift(pdata_CPlane1[c][i], pdata_CPlane2[c][i], orientation, XY_length, XY_overlap[c][i], XY_shift[c][i], i, c))
                    return false;
            }
        }

        else if(orientation == "right" || orientation == "up")
        {
            qDebug()<<__LINE__<<": orientation = "<<orientation.toStdString().c_str();
            for(int i = 0; i < 1150 - Z_shift; i ++)
            {
                if(pdata_CPlane1[c][i]){delete []pdata_CPlane1[c][i]; pdata_CPlane1[c][i] = 0;}
                pdata_CPlane1[c][i] = new unsigned short[XY_length*XY_length*1*1*2/2];
                memcpy(pdata_CPlane1[c][i], pdataAllPlane1[c][i], XY_length*XY_length*1*1*2);

                if(pdata_CPlane2[c][i]){delete []pdata_CPlane2[c][i]; pdata_CPlane2[c][i] = 0;}
                pdata_CPlane2[c][i] = new unsigned short[XY_length*XY_length*1*1*2/2];
                memcpy(pdata_CPlane2[c][i], pdataAllPlane2[c][i+Z_shift], XY_length*XY_length*1*1*2);
            }
            qDebug()<<__LINE__<<": pdata_CPlane1[0][0] = "<<pdata_CPlane1[0][0][0];
            qDebug()<<__LINE__<<": pdata_CPlane2[0][0] = "<<pdata_CPlane2[0][0][0];
            for(int i = 0; i < 1150-Z_shift; i ++)
            {
                if(!calculate_2Img_overlap_shift(pdata_CPlane1[c][i], pdata_CPlane2[c][i], orientation, XY_length, XY_overlap[c][i], XY_shift[c][i], i, c))
                    return false;
            }
        }
    }
    return 1;
}


bool calculate_2Img_overlap_shift(unsigned short * pdata1, unsigned short * pdata2, QString orientation, int XY_length, int & XY_ocerlap_I, int & XY_shift_I, int num, int c)
{
    qDebug()<<"Calculate the overlap and shift between the "<<num+1<<" pairs for "<<c+1<<" channel.";

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
                for(int row = 0; row < XY_length-ss2; row ++)
                {
                    for(int col = 0; col < ss1; col ++)
                    {
                        int diff;
                        if(orientation == "left")
                            diff = pdata1[row*XY_length+XY_length-ss1+col]-pdata2[(row+ss2)*XY_length+col];
                        else
                            diff = pdata2[row*XY_length+XY_length-ss1+col]-pdata1[(row+ss2)*XY_length+col];
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_ocerlap_I = ss1;
                    XY_shift_I = ss2;
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
                    for(int col = 0; col < XY_length-ss2; col ++)
                    {
                        int diff = pdata1[(XY_length-ss1+row)*XY_length+ss2+col]-pdata2[row*XY_length+col];
                        sum[indexOL] += diff * diff;
                        count[indexOL] += 1;
                    }
                }
                difference[indexOL] = sum[indexOL]/count[indexOL];

                if(minError>difference[indexOL])
                {
                    minError = difference[indexOL];
                    XY_ocerlap_I = ss1;
                    XY_shift_I = ss2;
                }
            }
        }
    }
    return 1;
}


void cut_planes_edge(unsigned short *pdata, unsigned short * &p_cut_data, int XY_length)
{
    if(p_cut_data){delete []p_cut_data; p_cut_data = 0;}
    p_cut_data = new unsigned short[XY_length * XY_length];
    for(int i = 0; i < XY_length; i ++)
        for(int j = 0; j < XY_length; j ++)
        {
            p_cut_data[i*XY_length+j] = pdata[(i+20)*(XY_length+40)+(20+j)];
        }
}

void calculate_expansion_ratio(vector<vector<int>> XY_overlap, vector<vector<int>> XY_shift, vector<vector<double>> & overlap_shift_Ratio, int XY_length, int Z_shift)
{
    qDebug()<<__LINE__<<"Calculae expansion ratio!";
    for(int c = 0; c < 3; c ++)
    {
        double expansion_para_1 = sqrt(pow((XY_length-XY_overlap[c][0]), 2) + pow(XY_shift[c][0], 2) + 0.0);
        for(int i = 0; i < 1150-Z_shift; i ++)
        {
            //overlap_shift_Ratio[c][i] = sqrt((pow((1880-XY_overlap[c][i]), 2) + pow(XY_shift[c][i], 2) + 0.0)/(pow((1880-XY_overlap[c][i+1]), 2) + pow(XY_shift[c][i+1], 2) + 0.0));
            double expansion_para_i = sqrt(pow((XY_length-XY_overlap[c][i]), 2) + pow(XY_shift[c][i], 2) + 0.0);
            overlap_shift_Ratio[c][i] = expansion_para_i/expansion_para_1;
        }
    }
}

void save_tiles_shift(QWidget *parent, QString saveFolder, string s1, string s2, QString orientation, int shift_XY[3], int shift_Z[3], vector<vector<int>> XY_overlap, vector<vector<int>> XY_shift, vector<vector<double>> overlap_shift_Ratio, int Z_shift)
{
    QString excelName = setExcelName(saveFolder, s1, s2, orientation);
    QAxObject *excel = new QAxObject(parent);
    excel -> setControl("Excel.Application"); // connect EXCEL control
    excel -> setProperty("DisplayAlerts", true); // display window
    QAxObject *workbooks = excel -> querySubObject("WorkBooks"); // obtain excel set
    workbooks -> dynamicCall("Add"); // create a new excel
    QAxObject *workbook = excel -> querySubObject("ActiveWorkBook"); // obtain current excel
    workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                            excelName, 51, QString(""), QString(""), false, false);
    //51xlsx, 56xls
    QAxObject *worksheets = workbook->querySubObject("WorkSheets"); // obtain sheets set
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet1 -> setProperty("Name", "MIP_shift");
    QAxObject *worksheet2 = appendSheet(worksheets, "XY_shift");

    QList <QList<QVariant>> datas_sheet1;
    QList <QList<QVariant>> datas_sheet2;

    string sname1 = "T"+s1+"-T"+s2;
    QString qname1 = QString :: fromStdString(sname1);
    for(int i = 0 ; i < 4; i ++)
    {
        QList<QVariant> rows1;
        if(i == 0)
        {
            rows1.append(qname1);
            if(orientation == "left" || orientation == "right")
                rows1.append("Y");
            else if(orientation == "up" || orientation == "down")
                rows1.append("X");
            rows1.append("Z");
        }
        else
        {
            QString name = "C" + QString :: fromStdString(to_string((i)));
            rows1.append(name);
            rows1.append(shift_XY[i-1]);
            rows1.append(shift_Z[i-1]);
        }
        datas_sheet1.append(rows1);
    }

    for(int i = 0; i < 1150 - Z_shift + 2; i ++)
    {
        QList<QVariant> rows2;
        if(i == 0)
        {
            rows2.append(qname1);
            QString merge_cell1 = "C1:F1";
            QAxObject *merge_range1 = worksheet2->querySubObject("Range(const QString&)", merge_cell1);
            merge_range1->setProperty("MergeCells", true);

            QString merge_cell2 = "H1:K1";
            QAxObject *merge_range2 = worksheet2->querySubObject("Range(const QString&)", merge_cell2);
            merge_range2->setProperty("MergeCells", true);

            QString merge_cell3 = "M1:P1";
            QAxObject *merge_range3 = worksheet2->querySubObject("Range(const QString&)", merge_cell3);
            merge_range3->setProperty("MergeCells", true);

            rows2.append("Z");
            rows2.append("Overlap");
            rows2.append("");
            rows2.append("");
            rows2.append("");
            rows2.append("");
            rows2.append("Shift");
            rows2.append("");
            rows2.append("");
            rows2.append("");
            rows2.append("");
            rows2.append("Expansion_Ratio");
            rows2.append("");
            rows2.append("");
            rows2.append("");
        }
        else if(i == 1)
        {
            rows2.append("");
            rows2.append("");
            rows2.append("C1");
            rows2.append("C2");
            rows2.append("C3");
            rows2.append("Average");
            rows2.append("");
            rows2.append("C1");
            rows2.append("C2");
            rows2.append("C3");
            rows2.append("Average");
            rows2.append("");
            rows2.append("C1");
            rows2.append("C2");
            rows2.append("C3");
            rows2.append("Average");
        }

        else
        {
            int index1;
            int index2;
            if(orientation == "left")
            {
                index1 = i+Z_shift-2;
                index2 = i-2;
            }
            else if(orientation == "right" || orientation == "up")
            {
                index1 = i-2;
                index2 = i+Z_shift-2;
            }

            string s1 = to_string(index1)+"_"+to_string(index2);
            QString qs1 = QString ::fromStdString(s1);
            rows2.append(qs1);

            rows2.append(i-1);
            rows2.append(XY_overlap[0][i-2]);
            rows2.append(XY_overlap[1][i-2]);
            rows2.append(XY_overlap[2][i-2]);
            double ave1 = (XY_overlap[0][i-2]+XY_overlap[1][i-2]+XY_overlap[2][i-2]+0.0)/3;
            rows2.append(ave1);
            rows2.append("");

            rows2.append(XY_shift[0][i-2]);
            rows2.append(XY_shift[1][i-2]);
            rows2.append(XY_shift[2][i-2]);
            double ave2 = (XY_shift[0][i-2]+XY_shift[1][i-2]+XY_shift[2][i-2]+0.0)/3;
            rows2.append(ave2);

            rows2.append("");
            rows2.append(overlap_shift_Ratio[0][i-2]);
            rows2.append(overlap_shift_Ratio[1][i-2]);
            rows2.append(overlap_shift_Ratio[2][i-2]);
            double ave3 = (overlap_shift_Ratio[0][i-2]+overlap_shift_Ratio[1][i-2]+overlap_shift_Ratio[2][i-2]+0.0)/3;
            rows2.append(ave3);
        }
        datas_sheet2.append(rows2);
    }

    QList<QVariant> vars1, vars2;
    for(auto v:datas_sheet1)
    {
        vars1.append(QVariant(v));
    }
    for(auto v:datas_sheet2)
    {
        vars2.append(QVariant(v));
    }
    QVariant var1 = QVariant(vars1);
    QVariant var2 = QVariant(vars2);

    QString range1 = "A1:C4";
    cout<<"excel range: "<<range1.toStdString()<<endl;
    QAxObject *excel_property1 = worksheet1 -> querySubObject("Range(const QString&)", range1);
    excel_property1 -> setProperty("Value", var1);
    excel_property1 -> setProperty("HorizontalAlignment", -4108);

    QString num = QString :: fromStdString(to_string(1150+2-Z_shift));
    QString range2 = "A1:P"+num;
    cout<<"excel range: "<<range2.toStdString()<<endl;
    QAxObject *excel_property2 = worksheet2 -> querySubObject("Range(const QString&)", range2);
    excel_property2 -> setProperty("Value", var2);
    excel_property2 -> setProperty("HorizontalAlignment", -4108);

    cout<<"excelName: "<<excelName.toStdString().c_str()<<endl;
    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel->dynamicCall("Quit(void)");
    delete excel;
}

QAxObject * appendSheet(QAxObject * &worksheets, const QString sheetName)
{
    int cnt = worksheets -> property("Count").toInt();
    QAxObject * pLastSheet = worksheets -> querySubObject("Item(int)", cnt);
    worksheets -> querySubObject("Add(QVariant)", pLastSheet->asVariant()); // create a new sheet
    QAxObject *pNewSheet = worksheets->querySubObject("Item(int)", cnt);
    pLastSheet -> dynamicCall("Move(QVariant)",pNewSheet->asVariant());
    pNewSheet -> setProperty("Name", sheetName);

    return pNewSheet;
}


QString setTitleName(QString saveFolder, string s, QDir dir, QString orientation)
{
    string s1 = "Tile_"+s+"_C";
    string sname = dir.dirName().toStdString();
    int i = 0;
    for(; i < sname.size(); i ++)
    {
        if(sname[i]>='0' && sname[i]<='9')
            break;
    }
    string si = string(1, sname[i]);
    QString qstr;
    if(orientation == "left" || orientation == "right")
        qstr = QString :: fromStdString(s1+si) + "_YZ.v3draw";
    else if(orientation == "up" || orientation == "down")
        qstr = QString :: fromStdString(s1+si) + "_XZ.v3draw";
    qDebug()<<__LINE__<<": fileName = "<<qstr.toStdString().c_str();
    QString fullName = saveFolder + "\\" + qstr;
    return fullName;

}

QString setExcelName(QString saveFolder, string s1, string s2, QString orientation)
{
    string ss = "T_"+s1+"_"+s2+"_";
    QString qstr;
    if(orientation == "left" || orientation == "right")
        qstr = QString :: fromStdString(ss) + "YZ_XY_shift.xlsx";
    else if(orientation == "up" || orientation == "down")
        qstr = QString :: fromStdString(ss) + "XZ_XY_shift.xlsx";
    qDebug()<<__LINE__<<": excelName = "<<qstr.toStdString().c_str();
    QString fullName = saveFolder + "\\" + qstr;
    return fullName;
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
