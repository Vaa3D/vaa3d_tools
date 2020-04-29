#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <iostream>
#include <QAxObject>

bool calculate_tiles_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);
bool calculate_overlap_shift(V3DPluginCallback2 &callback, QFileInfoList imageList1, vector< unsigned short * >& pdata_Plane1, QFileInfoList imageList2, vector< unsigned short * >& pdata_Plane2, vector<int> & XY_overlap_C, vector<int> & XY_shift_C, QString orientation, string s1, string s2, int Z_shift);
bool calculate_2Img_overlap_shift(unsigned short *pdata1, unsigned short *pdata2, QString orientation, int &XY_ocerlap_I, int &XY_shift_I);
void save_tiles_shift(QWidget *parent, QString saveFolder, string s1, string s2, QString orientation, vector<vector<int>> XY_overlap, vector<vector<int>> XY_shift, int Z_shift);


bool calculate_tiles_overlap_shift(V3DPluginCallback2 &callback, QWidget *parent)
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

    QString orientation1, orientation2;

    if(!setOrientation(orientation1, orientation2, s1, s2))
    {
        qDebug()<<__LINE__;
        return 0;
    }

    cout<<"Orientation1 = "<<orientation1.toStdString().c_str()<<endl;
    cout<<"Orientation2 = "<<orientation2.toStdString().c_str()<<endl;

    int Z_shift = 4;

    tileDir1.setFilter(QDir::Dirs);
    QFileInfoList channelList1 = tileDir1.entryInfoList();
    tileDir2.setFilter(QDir::Dirs);
    QFileInfoList channelList2 = tileDir2.entryInfoList();

    vector<vector<unsigned short *>> pdata_CPlane1(3, vector<unsigned short *>(12, 0));
    vector<vector<unsigned short *>> pdata_CPlane2(3, vector<unsigned short *>(12, 0));
    vector<vector<int>> XY_overlap(3, vector<int>(12, 0));
    vector<vector<int>> XY_shift(3, vector<int>(12, 0));

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

        if(!calculate_overlap_shift(callback, imageList1[i], pdata_CPlane1[i], imageList2[i], pdata_CPlane2[i], XY_overlap[i], XY_shift[i], orientation1, s1, s2, Z_shift))
            return false;
    }

    save_tiles_shift(parent, SaveFolder, s1, s2, orientation1, XY_overlap, XY_shift, Z_shift);

    return 1;
}

bool calculate_overlap_shift(V3DPluginCallback2 &callback, QFileInfoList imageList1, vector< unsigned short * >& pdata_Plane1, QFileInfoList imageList2, vector< unsigned short * >& pdata_Plane2,
                            vector<int> & XY_overlap_C, vector<int> & XY_shift_C, QString orientation, string s1, string s2, int Z_shift)
{
    vector<unsigned char *> data1d_vec1(12);
    vector<V3DLONG *> in_sz_vec1(12);
    vector<int> datatype_vec1(12);
    for(V3DLONG i = 0; i < 12; i++)
    {
        cout<<"This is "<<i*100+30<<" image for tile " <<s1.c_str()<<"."<<endl;
        in_sz_vec1[i] = new V3DLONG[4];
        if(!simple_loadimage_wrapper(callback, imageList1[i*100+30].filePath().toStdString().c_str(), data1d_vec1[i], in_sz_vec1[i], datatype_vec1[i]))
        {
            qDebug()<< "Error happens in reading the subject file.";
            return false;
        }

        if(pdata_Plane1[i]){delete []pdata_Plane1[i]; pdata_Plane1[i] = 0;}
        pdata_Plane1[i] = new unsigned short[in_sz_vec1[i][0]*in_sz_vec1[i][1]*in_sz_vec1[i][2]*in_sz_vec1[i][3] *datatype_vec1[i]/2];
        memcpy(pdata_Plane1[i], data1d_vec1[i], in_sz_vec1[i][0]*in_sz_vec1[i][1]*in_sz_vec1[i][2]*in_sz_vec1[i][3]*datatype_vec1[i]);
    }

    vector<unsigned char *> data1d_vec2(12);
    vector<V3DLONG *> in_sz_vec2(12);
    vector<int> datatype_vec2(12);
    for(V3DLONG i = 0; i < 12; i++)
    {
        int index;
        if(orientation == "left")
            index = i*100+30 - Z_shift;
        else if(orientation == "right" || orientation == "up")
            index = i*100+30 + Z_shift;
        cout<<"This is "<<index<<" image for tile"<<s2.c_str()<<"."<<endl;
        in_sz_vec2[i] = new V3DLONG[4];
        if(!simple_loadimage_wrapper(callback, imageList2[index].filePath().toStdString().c_str(), data1d_vec2[i], in_sz_vec2[i], datatype_vec2[i]))
        {
            qDebug()<< "Error happens in reading the subject file.";
            return false;
        }

        if(pdata_Plane2[i]){delete []pdata_Plane2[i]; pdata_Plane2[i] = 0;}
        pdata_Plane2[i] = new unsigned short[in_sz_vec2[i][0]*in_sz_vec2[i][1]*in_sz_vec2[i][2]*in_sz_vec2[i][3] *datatype_vec2[i]/2];
        memcpy(pdata_Plane2[i], data1d_vec2[i], in_sz_vec2[i][0]*in_sz_vec2[i][1]*in_sz_vec2[i][2]*in_sz_vec2[i][3]*datatype_vec2[i]);
    }

    for(int i = 0; i < 12; i ++)
    {
        if(!calculate_2Img_overlap_shift(pdata_Plane1[i], pdata_Plane2[i], orientation, XY_overlap_C[i], XY_shift_C[i]))
            return false;
    }
    return 1;
}

bool calculate_2Img_overlap_shift(unsigned short * pdata1, unsigned short * pdata2, QString orientation, int & XY_ocerlap_I, int & XY_shift_I)
{
    const int overlap_range = 50, shift_range = 20;
    double sum[overlap_range * shift_range] = {0};
    int difference[overlap_range * shift_range] = {0};
    int count[overlap_range * shift_range] = {0};
    double minError = INT_MAX;

    if(orientation == "left" || orientation == "right")
    {
        for(int ss1 = 260; ss1 < 260+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-260)+ss2;
                for(int row = 0; row < 1920-ss2; row ++)
                {
                    for(int col = 0; col < ss1; col ++)
                    {
                        int diff;
                        if(orientation == "left")
                            diff = pdata1[row*1920+1920-ss1+col]-pdata2[(row+ss2)*1920+col];
                        else
                            diff = pdata2[row*1920+1920-ss1+col]-pdata1[(row+ss2)*1920+col];
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
                    XY_ocerlap_I = ss1;
                    XY_shift_I = ss2;
                }
            }
            cout<<endl;
        }
        qDebug()<<"XY_overlap = "<<XY_ocerlap_I;
        qDebug()<<"XY_shift = "<<XY_shift_I;
        qDebug()<<" Calculate shift in overlap area is successful !";
    }

    else if(orientation == "up" || orientation == "down")
    {
        for(int ss1 = 260; ss1 < 260+overlap_range; ss1 ++)
        {
            for(int ss2 = 0; ss2 < shift_range; ss2 ++)
            {
                int indexOL = shift_range*(ss1-260)+ss2;
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
                    XY_ocerlap_I = ss1;
                    XY_shift_I = ss2;
                }
            }
            cout<<endl;
        }
        qDebug()<<"XY_overlap = "<<XY_ocerlap_I;
        qDebug()<<"XY_shift = "<<XY_shift_I;
        qDebug()<<" Calculate shift in overlap area is successful !";
    }
    return 1;
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

    for(int i = 0; i < 5; i ++)
    {
        QList<QVariant> rows;
        if(i == 0)
        {
            rows.append(qname1);
            for(int j = 0; j < 12; j ++)
            {
                int index1 = j*100+30;
                int index2;
                if(orientation == "left")
                    index2 = j*100+30 - Z_shift;
                else if(orientation == "right" || orientation == "up")
                    index2 = j*100+30 + Z_shift;

                char a = (char)(2*j+48+2+16);
                char b = a+1;
                string s(1, a);
                string ss(1, b);
                QString qs = QString ::fromStdString(s);
                QString qss = QString :: fromStdString(ss);
                QString merge_cell1 = qs+"1:"+qss+"1";
                QAxObject *merge_range1 = worksheet->querySubObject("Range(const QString&)", merge_cell1);
                merge_range1->setProperty("MergeCells", true);

                string s1 = to_string(index1)+"_"+to_string(index2);
                QString qs1 = QString ::fromStdString(s1);
                rows.append(qs1);
                rows.append("");
            }
        }
        else if(i == 1)
        {
            rows.append("Channel");
            for(int j = 0; j < 12; j ++)
            {
                rows.append("overlap");
                rows.append("shift");
            }
        }

        else
        {
            string s2 = "C"+to_string(i-1);
            QString qs2 = QString ::fromStdString(s2);
            rows.append(qs2);
            for(int j = 0; j < 12; j ++)
            {
                rows.append(XY_overlap[i-2][j]);
                rows.append(XY_shift[i-2][j]);
            }
        }
        datas_sheet.append(rows);
    }

        QList<QVariant> vars;
        for(auto v:datas_sheet)
            vars.append(QVariant(v));
        QVariant var = QVariant(vars);

        QString range = "A1:Y5";
        cout<<"excel range: "<<range.toStdString()<<endl;

        QAxObject *excel_property = worksheet -> querySubObject("Range(const QString&)", range);
        excel_property -> setProperty("Value", var);
        excel_property -> setProperty("HorizontalAlignment", -4108);
        qDebug()<<__LINE__;

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
