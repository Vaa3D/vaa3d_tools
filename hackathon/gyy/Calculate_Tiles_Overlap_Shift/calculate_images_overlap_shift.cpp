#include <v3d_interface.h>
#include <QDebug>
#include <QAxObject>
#include <iostream>
#include <fstream>

bool calculate_tiles_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);
bool calculate_overlap_shift(V3DPluginCallback2 &callback, QFileInfoList imageList1, vector< unsigned short * >& pdata_Plane1, QFileInfoList imageList2, vector< unsigned short * >& pdata_Plane2, vector<int> & XY_overlap_C, vector<int> & XY_shift_C, QString orientation, string s1, string s2, int Z_shift, int c);
bool calculate_2Img_overlap_shift(unsigned short *pdata1, unsigned short *pdata2, QString orientation, int &XY_ocerlap_I, int &XY_shift_I, int num, int c);
void save_tiles_shift(QWidget *parent, QString saveFolder, string s1, string s2, QString orientation, vector<vector<int>> XY_overlap, vector<vector<int>> XY_shift, int Z_shift);
void cut_planes_edge(unsigned short *pdata, unsigned short *&p_cut_data);

bool calculate_tiles_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent)
{
    cout<<"Calculate tiles overlap shift!"<<endl;
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

    QString orientation1, orientation2;

    if(!setOrientation(orientation1, orientation2, s1, s2))
    {
        qDebug()<<__LINE__;
        return 0;
    }

    cout<<"Orientation1 = "<<orientation1.toStdString().c_str()<<endl;
    cout<<"Orientation2 = "<<orientation2.toStdString().c_str()<<endl;

    int Z_shift = 1;

    tileDir1.setFilter(QDir::Dirs);
    QFileInfoList channelList1 = tileDir1.entryInfoList();
    tileDir2.setFilter(QDir::Dirs);
    QFileInfoList channelList2 = tileDir2.entryInfoList();

    vector<vector<unsigned short *>> pdata_CPlane1(3, vector<unsigned short *>(1150, 0));
    vector<vector<unsigned short *>> pdata_CPlane2(3, vector<unsigned short *>(1150, 0));
    vector<vector<int>> XY_overlap(3, vector<int>(1150, 0));
    vector<vector<int>> XY_shift(3, vector<int>(1150, 0));

    QDir dir1[3], dir2[3];
    QFileInfoList imageList1[3], imageList2[3];
    for(int i = 0; i < channelList1.size()-2; i ++)
    {
        dir1[i] = channelList1[i+2].filePath();
        dir2[i] = channelList2[i+2].filePath();

        QStringList filters;
        filters << "*.tif" << "*.v3draw";
        imageList1[i] = dir1[i].entryInfoList(filters, QDir::Files);
        imageList2[i] = dir2[i].entryInfoList(filters, QDir::Files);

        if(!calculate_overlap_shift(callback, imageList1[i], pdata_CPlane1[i], imageList2[i], pdata_CPlane2[i], XY_overlap[i], XY_shift[i], orientation1, s1, s2, Z_shift, i))
            return false;
    }

    save_tiles_shift(parent, SaveFolder, s1, s2, orientation1, XY_overlap, XY_shift, Z_shift);

    return 1;
}

bool calculate_overlap_shift(V3DPluginCallback2 &callback, QFileInfoList imageList1, vector< unsigned short * >& pdata_Plane1, QFileInfoList imageList2, vector< unsigned short * >& pdata_Plane2,
                            vector<int> & XY_overlap_C, vector<int> & XY_shift_C, QString orientation, string s1, string s2, int Z_shift, int c)
{
    if(orientation == "left")
    {
        vector<unsigned char *> data1d_vec1(1150-Z_shift);
        vector<V3DLONG *> in_sz_vec1(1150-Z_shift);
        vector<int> datatype_vec1(1150-Z_shift);
        for(V3DLONG i = Z_shift; i < 1150; i++)
        {
            cout<<"This is "<<i<<" image for " <<2*c+1<<"."<<endl;
            in_sz_vec1[i-Z_shift] = new V3DLONG[4];
            if(c == 0)
            {
                if(!simple_loadimage_wrapper(callback, imageList1[i].filePath().toStdString().c_str(), data1d_vec1[i-Z_shift], in_sz_vec1[i-Z_shift], datatype_vec1[i-Z_shift]))
                {
                    qDebug()<< "Error happens in reading the subject file.";
                    return false;
                }
            }
            else if(c == 1)
            {
                if(!simple_loadimage_wrapper(callback, imageList1[i+2].filePath().toStdString().c_str(), data1d_vec1[i-Z_shift], in_sz_vec1[i-Z_shift], datatype_vec1[i-Z_shift]))
                {
                    qDebug()<< "Error happens in reading the subject file.";
                    return false;
                }
            }
            else
            {
                if(!simple_loadimage_wrapper(callback, imageList1[i+12].filePath().toStdString().c_str(), data1d_vec1[i-Z_shift], in_sz_vec1[i-Z_shift], datatype_vec1[i-Z_shift]))
                {
                    qDebug()<< "Error happens in reading the subject file.";
                    return false;
                }
            }

            if(pdata_Plane1[i-Z_shift]){delete []pdata_Plane1[i-Z_shift]; pdata_Plane1[i-Z_shift] = 0;}
            pdata_Plane1[i-Z_shift] = new unsigned short[in_sz_vec1[0][0]*in_sz_vec1[0][1]*in_sz_vec1[0][2]*in_sz_vec1[0][3] *datatype_vec1[0]/2];
            memcpy(pdata_Plane1[i-Z_shift], data1d_vec1[i-Z_shift], in_sz_vec1[0][0]*in_sz_vec1[0][1]*in_sz_vec1[0][2]*in_sz_vec1[0][3]*datatype_vec1[0]);
        }

        vector<unsigned char *> data1d_vec2(1150-Z_shift);
        vector<V3DLONG *> in_sz_vec2(1150-Z_shift);
        vector<int> datatype_vec2(1150-Z_shift);
        for(V3DLONG i = 0; i < 1150 - Z_shift; i++)
        {
            cout<<"This is "<<i<<" image for "<<2*(c+1)<<"."<<endl;
            in_sz_vec2[i] = new V3DLONG[4];
            if(c == 0)
            {
                if(!simple_loadimage_wrapper(callback, imageList2[i].filePath().toStdString().c_str(), data1d_vec2[i], in_sz_vec2[i], datatype_vec2[i]))
                {
                    qDebug()<< "Error happens in reading the subject file.";
                    return false;
                }
            }
            else if(c == 1)
            {
                if(!simple_loadimage_wrapper(callback, imageList2[i+2].filePath().toStdString().c_str(), data1d_vec2[i], in_sz_vec2[i], datatype_vec2[i]))
                {
                    qDebug()<< "Error happens in reading the subject file.";
                    return false;
                }
            }
            else
            {
                if(!simple_loadimage_wrapper(callback, imageList2[i+12].filePath().toStdString().c_str(), data1d_vec2[i], in_sz_vec2[i], datatype_vec2[i]))
                {
                    qDebug()<< "Error happens in reading the subject file.";
                    return false;
                }
            }

            if(pdata_Plane2[i]){delete []pdata_Plane2[i]; pdata_Plane2[i] = 0;}
            pdata_Plane2[i] = new unsigned short[in_sz_vec2[0][0]*in_sz_vec2[0][1]*in_sz_vec2[0][2]*in_sz_vec2[0][3] *datatype_vec2[0]/2];
            memcpy(pdata_Plane2[i], data1d_vec2[i], in_sz_vec2[0][0]*in_sz_vec2[0][1]*in_sz_vec2[0][2]*in_sz_vec2[0][3]*datatype_vec2[0]);
        }

        for(int i = 0; i < 1150-Z_shift; i ++)
        {
            if(!calculate_2Img_overlap_shift(pdata_Plane1[i], pdata_Plane2[i], orientation, XY_overlap_C[i], XY_shift_C[i], i, c))
                return false;
        }
    }

    else if(orientation == "right" || orientation == "up")
    {
        vector<unsigned char *> data1d_vec1(1150-Z_shift);
        vector<V3DLONG *> in_sz_vec1(1150-Z_shift);
        vector<int> datatype_vec1(1150-Z_shift);
        for(V3DLONG i = 0; i < 1150 - Z_shift; i++)
        {
            cout<<"This is "<<i<<" image for " <<2*c+1<<"."<<endl;
            in_sz_vec1[i] = new V3DLONG[4];
            if(!simple_loadimage_wrapper(callback, imageList1[i].filePath().toStdString().c_str(), data1d_vec1[i], in_sz_vec1[i], datatype_vec1[i]))
            {
                qDebug()<< "Error happens in reading the subject file.";
                return false;
            }

            if(pdata_Plane1[i]){delete []pdata_Plane1[i]; pdata_Plane1[i] = 0;}
            pdata_Plane1[i] = new unsigned short[in_sz_vec1[0][0]*in_sz_vec1[0][1]*in_sz_vec1[0][2]*in_sz_vec1[0][3] *datatype_vec1[0]/2];
            memcpy(pdata_Plane1[i], data1d_vec1[i], in_sz_vec1[0][0]*in_sz_vec1[0][1]*in_sz_vec1[0][2]*in_sz_vec1[0][3]*datatype_vec1[0]);
        }

        vector<unsigned char *> data1d_vec2(1150-Z_shift);
        vector<V3DLONG *> in_sz_vec2(1150-Z_shift);
        vector<int> datatype_vec2(1150-Z_shift);
        for(V3DLONG i = Z_shift; i < 1150; i++)
        {
            cout<<"This is "<<i<<" image for "<<2*(c+1)<<"."<<endl;
            in_sz_vec2[i - Z_shift] = new V3DLONG[4];
            if(!simple_loadimage_wrapper(callback, imageList2[i].filePath().toStdString().c_str(), data1d_vec2[i - Z_shift], in_sz_vec2[i - Z_shift], datatype_vec2[i - Z_shift]))
            {
                qDebug()<< "Error happens in reading the subject file.";
                return false;
            }

            if(pdata_Plane2[i - Z_shift]){delete []pdata_Plane2[i - Z_shift]; pdata_Plane2[i - Z_shift] = 0;}
            pdata_Plane2[i - Z_shift] = new unsigned short[in_sz_vec2[0][0]*in_sz_vec2[0][1]*in_sz_vec2[0][2]*in_sz_vec2[0][3] *datatype_vec2[0]/2];
            memcpy(pdata_Plane2[i - Z_shift], data1d_vec2[i - Z_shift], in_sz_vec2[0][0]*in_sz_vec2[0][1]*in_sz_vec2[0][2]*in_sz_vec2[0][3]*datatype_vec2[0]);
        }

        for(int i = 0; i < 1150-Z_shift; i ++)
        {
            if(!calculate_2Img_overlap_shift(pdata_Plane1[i], pdata_Plane2[i], orientation, XY_overlap_C[i], XY_shift_C[i], i, c))
                return false;
        }
    }
    return 1;
}

bool calculate_2Img_overlap_shift(unsigned short * pdata1, unsigned short * pdata2, QString orientation, int & XY_overlap_I, int & XY_shift_I, int num, int c)
{
    qDebug()<<"Calculate the overlap and shift between the "<<num+1<<" pairs for "<<c+1<<" channel.";
    unsigned short * p_cut_data1 = 0;
    unsigned short * p_cut_data2 = 0;
    cut_planes_edge(pdata1, p_cut_data1);
    cut_planes_edge(pdata2, p_cut_data2);
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
                    XY_overlap_I = ss1;
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
                    XY_overlap_I = ss1;
                    XY_shift_I = ss2;
                }
            }
        }
    }
    return 1;
}

void cut_planes_edge(unsigned short *pdata, unsigned short * &p_cut_data)
{
    if(p_cut_data){delete []p_cut_data; p_cut_data = 0;}
    p_cut_data = new unsigned short[1880 * 1880];
    for(int i = 0; i < 1880; i ++)
        for(int j = 0; j < 1880; j ++)
        {
            p_cut_data[i*1880+j] = pdata[(i+20)*1920+(20+j)];
        }
}

void save_tiles_shift(QWidget *parent, QString saveFolder, string s1, string s2, QString orientation, vector<vector<int>> XY_overlap, vector<vector<int>> XY_shift, int Z_shift)
{
    string ss = "T_"+s1+"_"+s2+"_XY_shift.xlsx";
    QString excelName = saveFolder + "\\" + QString :: fromStdString(ss);
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
    QAxObject *worksheet = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet -> setProperty("Name", "XY_shift");

    QList <QList<QVariant>> datas_sheet;
    string sname1 = "T"+s1+"-T"+s2;
    QString qname1 = QString :: fromStdString(sname1);

    for(int i = 0; i < 1150 - Z_shift + 2; i ++)
    {
        QList<QVariant> rows;
        if(i == 0)
        {
            rows.append(qname1);
            QString merge_cell1 = "C1:F1";
            QAxObject *merge_range1 = worksheet->querySubObject("Range(const QString&)", merge_cell1);
            merge_range1->setProperty("MergeCells", true);

            QString merge_cell2 = "H1:K1";
            QAxObject *merge_range2 = worksheet->querySubObject("Range(const QString&)", merge_cell2);
            merge_range2->setProperty("MergeCells", true);

            rows.append("Z");
            rows.append("overlap");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("shift");
            rows.append("");
            rows.append("");
            rows.append("");
        }
        else if(i == 1)
        {
            rows.append("");
            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");
            rows.append("Average");
            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");
            rows.append("Average");
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
            rows.append(qs1);

            rows.append(i-1);
            rows.append(XY_overlap[0][i-2]);
            rows.append(XY_overlap[1][i-2]);
            rows.append(XY_overlap[2][i-2]);

            double ave1 = (XY_overlap[0][i-2]+XY_overlap[1][i-2]+XY_overlap[2][i-2]+0.0)/3;
            rows.append(ave1);
            rows.append("");

            rows.append(XY_shift[0][i-2]);
            rows.append(XY_shift[1][i-2]);
            rows.append(XY_shift[2][i-2]);

            double ave2 = (XY_shift[0][i-2]+XY_shift[1][i-2]+XY_shift[2][i-2]+0.0)/3;
            rows.append(ave2);

        }
        datas_sheet.append(rows);
    }

        QList<QVariant> vars;
        for(auto v:datas_sheet)
            vars.append(QVariant(v));
        QVariant var = QVariant(vars);

        QString num = QString :: fromStdString(to_string(1150+2-Z_shift));
        QString range = "A1:K"+num;
        cout<<"excel range: "<<range.toStdString()<<endl;

        QAxObject *excel_property = worksheet -> querySubObject("Range(const QString&)", range);
        excel_property -> setProperty("Value", var);
        excel_property -> setProperty("HorizontalAlignment", -4108);
        qDebug()<<__LINE__;

        cout<<"excelName: "<<excelName.toStdString().c_str()<<endl;
        workbook->dynamicCall("Save()");
        workbook->dynamicCall("Close(Boolean)", false);
        excel->dynamicCall("Quit(void)");
        delete excel;
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
