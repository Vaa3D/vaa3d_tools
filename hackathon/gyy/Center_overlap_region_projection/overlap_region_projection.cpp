#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <iostream>
#include <QAxObject>

bool calculate_overlap_region_projection(V3DPluginCallback2 &callback, QWidget *parent);
bool calculate_project(V3DPluginCallback2 &callback, QFileInfoList imageList, unsigned short * & pdataPro, QString orientation, int XY_overlap, int count);
bool save_projection_files(V3DPluginCallback2 &callback, vector<unsigned short *> pdataPro1, vector<unsigned short *> pdataPro2, QFileInfoList imageList, QString saveTile1[3], QString saveTile2[3]);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);
bool calculate_tiles_shift(vector<unsigned short *> pdataPro1, vector<unsigned short *> pdataPro2, QString orientation, int shift_XY[3], int shift_Z[3]);
void save_tiles_shift(QWidget *parent, QString saveFolder, string s1, string s2, QString orientation, int shift_XY[3], int shift_Z[3]);
QString setTitleName(QString saveFolder, string s, QDir dir, QString orientation);
QString setExcelName(QString saveFolder, string s1, string s2, QString orientation);

bool calculate_overlap_region_projection(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString tileFolderFiles1 = QFileDialog::getExistingDirectory(NULL, "Select tile1 folder","D:\\");
    QString tileFolderFiles2 = QFileDialog :: getExistingDirectory(0, "Select tile2 folder", tileFolderFiles1);
    QString SaveFolder = QFileDialog :: getExistingDirectory(0, "Select folder to save", "D:\\");

    QDir tileDir1(tileFolderFiles1);
    QDir tileDir2(tileFolderFiles2);
    if(!tileDir1.exists() || !tileDir2.exists())
        return false;

    string sname1 = tileDir1.dirName().toStdString();
    string s1 = sname1.substr(10, 2);
    string sname2 = tileDir2.dirName().toStdString();
    string s2 = sname2.substr(10, 2);

    int XY_overlap = 294;
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

    vector<unsigned short *> pdataPro1(3, 0);
    vector<unsigned short *> pdataPro2(3, 0);

    QDir dir1[3], dir2[3];
    QFileInfoList imageList1[3], imageList2[3];
    QString tileSave1[3], tileSave2[3];
    int count = 0;
    for(int i = 0; i < channelList1.size()-2; i ++)
    {
        dir1[i] = channelList1[i+2].filePath();
        dir2[i] = channelList2[i+2].filePath();

        QStringList filters;
        filters << "*.tif" << "*.v3draw";
        imageList1[i] = dir1[i].entryInfoList(filters, QDir::Files);
        imageList2[i] = dir2[i].entryInfoList(filters, QDir::Files);

        count ++;
        if(!calculate_project(callback, imageList1[i], pdataPro1[i], orientation1, XY_overlap, count))
            return false;
        count ++;
        if(!calculate_project(callback, imageList2[i], pdataPro2[i], orientation2, XY_overlap, count))
            return false;

        qDebug()<<__LINE__<<": pdataPro[0][0] = "<<pdataPro1[0][0];
        tileSave1[i] = setTitleName(SaveFolder, s1, dir1[i], orientation1);
        tileSave2[i] = setTitleName(SaveFolder, s2, dir2[i], orientation2);
    }

    qDebug()<<__LINE__<<": pdataPro[0][0] = "<<pdataPro2[0][0];
    if(!save_projection_files(callback, pdataPro1, pdataPro2, imageList1[0], tileSave1, tileSave2))
        return false;

    int shift_XY[3] = {0}, shift_Z[3] = {0};
    if(calculate_tiles_shift(pdataPro1, pdataPro2, orientation1, shift_XY, shift_Z))
    {
        save_tiles_shift(parent, SaveFolder, s1, s2, orientation1, shift_XY, shift_Z);
    }
}

bool calculate_project(V3DPluginCallback2 &callback, QFileInfoList imageList, unsigned short * & pdataPro, QString orientation, int XY_overlap, int count)
{
    vector<unsigned char *> data1d_vec(imageList.size());
    vector<V3DLONG *> in_sz_vec(imageList.size());
    vector<int> datatype_vec(imageList.size());
    vector<unsigned short *> data1d_copy(imageList.size());
    for(V3DLONG i = 0; i < imageList.size(); i++)
    {
        cout<<"This is "<<i+1<<" image for "<<count<<"."<<endl;
        in_sz_vec[i] = new V3DLONG[4];
        if(!simple_loadimage_wrapper(callback, imageList[i].filePath().toStdString().c_str(), data1d_vec[i], in_sz_vec[i], datatype_vec[i]))
        {
            qDebug()<< "Error happens in reading the subject file.";
            return false;
        }

        data1d_copy[i] = new unsigned short[in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3] *datatype_vec[i]/2];
        memcpy(data1d_copy[i], data1d_vec[i], in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3]*datatype_vec[i]);
    }

    if(pdataPro){delete []pdataPro; pdataPro = 0;}
    pdataPro = new unsigned short[in_sz_vec[0][0]*imageList.size()*datatype_vec[0]/2];

    V3DLONG centerOverlap = 0;
    if(orientation == "left" || orientation == "right")
    {
        if(orientation == "left")
            centerOverlap = 1920-XY_overlap/2;
        else
            centerOverlap = XY_overlap/2;

        for(int i = 0; i < imageList.size(); i ++)
        {
            for(int j = 0; j < 1920; j ++)
            {
                int maxValue = data1d_copy[i][j*1920+centerOverlap-10];
                for(int k = centerOverlap-10; k < centerOverlap+10; k++)
                {
                    maxValue = ((maxValue >= data1d_copy[i][j*1920+k]) ? maxValue : data1d_copy[i][j*1920+k]);
                }
                pdataPro[i*1920+j] = maxValue;
            }
        }

    }

    else if(orientation == "up" || orientation == "down")
    {
        if(orientation == "up")
            centerOverlap = 1920 - XY_overlap/2;
        else
            centerOverlap = XY_overlap/2;

        for(int i = 0; i < imageList.size(); i ++)
        {
            for(int j = 0; j < 1920; j ++)
            {
                int maxValue = data1d_copy[i][(centerOverlap-10)*1920+j];
                for(int k = centerOverlap-10; k < centerOverlap+10; k ++)
                {
                    maxValue = ((maxValue >= data1d_copy[i][k*1920+j]) ? maxValue : data1d_copy[i][k*1920+j]);
                }
                pdataPro[i*1920+j] = maxValue;
            }
        }
    }
    qDebug()<<__LINE__<<": pdataPro[0][0] = "<<pdataPro[0];
}

bool calculate_tiles_shift(vector<unsigned short *> pdataPro1, vector<unsigned short *> pdataPro2, QString orientation, int shift_XY[3], int shift_Z[3])
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
                    for(int row = 0; row < 1162-sz; row ++)
                    {
                        for(int col = 0; col < 1920-ss; col ++)
                        {
                            int diff = pdataPro1[i][(row+sz)*1920+col]-pdataPro2[i][row*1920+col+ss];
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
                    for(int row = 0; row < 1162-sz; row ++)
                    {
                        for(int col = 0; col < 1920-ss; col ++)
                        {
                            int diff = pdataPro1[i][row*1920+col+ss]-pdataPro2[i][(row+sz)*1920+col];
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
        return 1;
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
        unsigned char * data1dTile1 = new unsigned char[in_sz[0]*imageList.size()*datatype];
        memcpy(data1dTile1, pdataPro1[i], in_sz[0]*imageList.size()*datatype);
        V3DLONG in_sz_pro1[4] = {in_sz[0], imageList.size(), 1, 1};
        qDebug()<<__LINE__<<": data1dTile1[0] = "<<data1dTile1[0];
        if(simple_saveimage_wrapper(callback, saveTile1[i].toStdString().c_str(), data1dTile1, in_sz_pro1, datatype))
        {
            qDebug()<<"Save Tile1 successfully!";
        }

        // Save tile2
        unsigned char * data1dTile2 = new unsigned char[in_sz[0]*imageList.size()*datatype];
        memcpy(data1dTile2, pdataPro2[i], in_sz[0]*imageList.size()*datatype);
        V3DLONG in_sz_pro2[4] = {in_sz[0], imageList.size(), 1, 1};
        qDebug()<<__LINE__<<": data1dTile2[0] = "<<data1dTile2[0];
        if(simple_saveimage_wrapper(callback, saveTile2[i].toStdString().c_str(), data1dTile2, in_sz_pro2, datatype))
        {
            qDebug()<<"Save Tile2 successfully!";
        }
    }

}

void save_tiles_shift(QWidget *parent, QString saveFolder, string s1, string s2, QString orientation, int shift_XY[3], int shift_Z[3])
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
    worksheet1 -> setProperty("Name", "shift");

    QList <QList<QVariant>> datas_sheet;
    string sname1 = "T"+s1+"-T"+s2;
    QString qname1 = QString :: fromStdString(sname1);
    for(int i = 0 ; i < 4; i ++)
    {
        QList<QVariant> rows;
        if(i == 0)
        {
            rows.append(qname1);
            if(orientation == "left" || orientation == "right")
                rows.append("Y");
            else if(orientation == "up" || orientation == "down")
                rows.append("X");
            rows.append("Z");
        }
        else
        {
            QString name = "C" + QString :: fromStdString(to_string((i)));
            rows.append(name);
            rows.append(shift_XY[i-1]);
            rows.append(shift_Z[i-1]);
        }
        datas_sheet.append(rows);
    }

    qDebug()<<__LINE__;
    QList<QVariant> vars;
    for(auto v:datas_sheet)
        vars.append(QVariant(v));
    QVariant var = QVariant(vars);

    QString range = "A1:C4";
    cout<<"excel range: "<<range.toStdString()<<endl;

    QAxObject *excel_property = worksheet1 -> querySubObject("Range(const QString&)", range);
    excel_property -> setProperty("Value", var);
    excel_property -> setProperty("HorizontalAlignment", -4108);

    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel->dynamicCall("Quit(void)");
    delete excel;

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
    string ss = "T_"+s1+"_"+s2+"_MIP_shifts_";
    QString qstr;
    if(orientation == "left" || orientation == "right")
        qstr = QString :: fromStdString(ss) + "YZ.xlsx";
    else if(orientation == "up" || orientation == "down")
        qstr = QString :: fromStdString(ss) + "XZ.xlsx";
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
