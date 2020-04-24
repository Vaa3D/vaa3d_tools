#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <iostream>
#include <QAxObject>

using namespace std;

bool Calculate_Z_Shift_tiles(V3DPluginCallback2 &callback, QWidget *parent);
bool calculate_region_mean(V3DPluginCallback2 &callback, vector<unsigned short *> & pdataMean, QFileInfoList imageList, QString orientation, int XY_shift, int XY_overlap, int count);
void saveMean_grayValue(QWidget *parent, vector<vector<unsigned short *>> & pdataMean, QDir tileDir1, QString saveFileName, int z_size);
QString setTitleName(QString qname);
bool setOrientation(QString & orientation1, QString & orientation2, string s1, string s2);

bool Calculate_Z_Shift_tiles(V3DPluginCallback2 &callback, QWidget *parent)
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

    int XY_shift = 0, XY_overlap = 294;
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

    vector<vector<unsigned short *>> pdataMean1(3, vector<unsigned short *>(21, 0));
    vector<vector<unsigned short *>> pdataMean2(3, vector<unsigned short *>(21, 0));

    int imageSize = 0;
    int count = 0;
    for(int i = 2; i < channelList1.size(); i ++)
    {
        QDir dir1(channelList1[i].filePath());
        QDir dir2(channelList2[i].filePath());

        QStringList filters;
        filters << "*.tif";
        QFileInfoList imageList1 = dir1.entryInfoList(filters, QDir::Files);
        QFileInfoList imageList2 = dir2.entryInfoList(filters, QDir::Files);
        imageSize = imageList1.size();

        count ++;
        if(!calculate_region_mean(callback, pdataMean1[i-2], imageList1, orientation1, XY_shift, XY_overlap, count))
        {
            return false;
        }

        count ++;
        if(!calculate_region_mean(callback, pdataMean2[i-2], imageList2, orientation2, XY_shift, XY_overlap, count))
        {
            return false;
        }
    }
    cout<<"Calculate Mean successful !"<<endl;

    saveMean_grayValue(parent, pdataMean1, tileDir1, SaveFolder, imageSize);
    saveMean_grayValue(parent, pdataMean2, tileDir2, SaveFolder, imageSize);
    cout<<"Save gray value successful !"<<endl;
    return 1;
}


bool calculate_region_mean(V3DPluginCallback2 &callback, vector<unsigned short *> &pdataMean, QFileInfoList imageList, QString orientation, int XY_shift, int XY_overlap, int count)
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

    V3DLONG region_center_X[21];
    V3DLONG region_center_Y[21];
    if(orientation == "left")
    {

        for(int i = 0; i < 7; i ++)
        {
            region_center_X[i*3+0] = 1920-XY_overlap+10;
            region_center_X[i*3+1] = 1920-XY_overlap/2;
            region_center_X[i*3+2] = 1920-10;
        }

        for(int i = 0; i < 3; i ++)
        {
            region_center_Y[0*3+i] = 10;
            region_center_Y[3*3+i] = (1920-XY_shift)/2;
            region_center_Y[6*3+i] = 1910-XY_shift;
            region_center_Y[1*3+i] = (region_center_Y[3*3+i]-region_center_Y[0*3+i])/3+region_center_Y[0*3+i];
            region_center_Y[2*3+i] = (region_center_Y[3*3+i]-region_center_Y[0*3+i])/3*2+region_center_Y[0*3+i];
            region_center_Y[4*3+i] = (region_center_Y[6*3+i]-region_center_Y[3*3+i])/3+region_center_Y[3*3+i];
            region_center_Y[5*3+i] = (region_center_Y[6*3+i]-region_center_Y[3*3+i])/3*2+region_center_Y[3*3+i];
        }

    }

    else if(orientation == "right")
    {

        for(int i = 0; i < 7; i ++)
        {
            region_center_X[i*3+0] = 10;
            region_center_X[i*3+1] = XY_overlap/2;
            region_center_X[i*3+2] = XY_overlap-10;
        }
        for(int i = 0; i < 3; i ++)
        {
            region_center_Y[0*3+i] = 10+XY_shift;
            region_center_Y[3*3+i] = (1920+XY_shift)/2;
            region_center_Y[6*3+i] = 1910;
            region_center_Y[1*3+i] = (region_center_Y[3*3+i]-region_center_Y[0*3+i])/3+region_center_Y[0*3+i];
            region_center_Y[2*3+i] = (region_center_Y[3*3+i]-region_center_Y[0*3+i])/3*2+region_center_Y[0*3+i];
            region_center_Y[4*3+i] = (region_center_Y[6*3+i]-region_center_Y[3*3+i])/3+region_center_Y[3*3+i];
            region_center_Y[5*3+i] = (region_center_Y[6*3+i]-region_center_Y[3*3+i])/3*2+region_center_Y[3*3+i];
        }
    }

    else if(orientation == "down")
    {

        for(int i = 0; i < 7; i ++)
        {
            region_center_Y[0*7+i] = 10;
            region_center_Y[1*7+i] = XY_overlap/2;
            region_center_Y[2*7+i] = XY_overlap-10;
        }
        for(int i = 0; i < 3; i ++)
        {
            region_center_X[i*7+0] = 10;
            region_center_X[i*7+3] = (1920-XY_shift)/2;
            region_center_X[i*7+6] = 1910-XY_shift;
            region_center_X[i*7+1] = (region_center_X[i*7+3]-region_center_X[i*7+0])/3+region_center_X[i*7+0];
            region_center_X[i*7+2] = (region_center_X[i*7+3]-region_center_X[i*7+0])/3*2+region_center_X[i*7+0];
            region_center_X[i*7+4] = (region_center_X[i*7+6]-region_center_X[i*7+3])/3+region_center_X[i*7+3];
            region_center_X[i*7+5] = (region_center_X[i*7+6]-region_center_X[i*7+3])/3*2+region_center_X[i*7+3];
        }
    }

    else if(orientation == "up")
    {

        for(int i = 0; i < 7; i ++)
        {
            region_center_Y[0*7+i] = 1920-XY_overlap+10;
            region_center_Y[1*7+i] = 1920-XY_overlap/2;
            region_center_Y[2*7+i] = 1920-10;
        }
        for(int i = 0; i < 3; i ++)
        {
            region_center_X[i*7+0] = 10+XY_shift;
            region_center_X[i*7+3] = (1920+XY_shift)/2;
            region_center_X[i*7+6] = 1910;
            region_center_X[i*7+1] = (region_center_X[i*7+3]-region_center_X[i*7+0])/3+region_center_X[i*7+0];
            region_center_X[i*7+2] = (region_center_X[i*7+3]-region_center_X[i*7+0])/3*2+region_center_X[i*7+0];
            region_center_X[i*7+4] = (region_center_X[i*7+6]-region_center_X[i*7+3])/3+region_center_X[i*7+3];
            region_center_X[i*7+5] = (region_center_X[i*7+6]-region_center_X[i*7+3])/3*2+region_center_X[i*7+3];
        }
    }

    qDebug()<<__LINE__<<" : 2-"<<region_center_X[1]<<"; 4-"<<region_center_X[3];
    for(int i = 0; i < 21; i ++)
    {
        if(pdataMean[i]){delete []pdataMean[i]; pdataMean[i] = 0;}
        pdataMean[i] = new unsigned short[imageList.size()*1];

        V3DLONG start_X = region_center_X[i]-10;
        V3DLONG start_Y = region_center_Y[i]-10;
        V3DLONG end_X = region_center_X[i]+10-1;
        V3DLONG end_Y = region_center_Y[i]+10-1;

        for(V3DLONG image_i = 0; image_i < imageList.size(); image_i ++)
        {
            V3DLONG record = 0;
            double values = 0, sum = 0;
            for(V3DLONG mi = start_X; mi <= end_X; mi ++)
                for(V3DLONG ni = start_Y; ni <= end_Y; ni ++)
                {
                    sum += data1d_copy[image_i][ni*in_sz_vec[image_i][0] + mi];
                    record ++;
                }
            values = sum/record;
            pdataMean[i][image_i] = int(values+0.5);
        }
    }
}


void saveMean_grayValue(QWidget *parent, vector<vector<unsigned short *>> & pdataMean1, QDir tileDir1, QString saveFileName, int z_size)
{
    for(int j = 0; j < 21; j ++)
    {
        QString dirNames(tileDir1.dirName());
        QString numName = QString::fromStdString(to_string(j+1));
        QString excelName = saveFileName +"\\"+setTitleName(dirNames)+numName+"_Value.xlsx";
        cout<<"excelName: "<<excelName.toStdString()<<endl;

        QAxObject *excel = new QAxObject(parent);
        excel -> setControl("Excel.Application"); // connect EXCEL control
        excel -> setProperty("DisplayAlerts", true); // display window
        QAxObject *workbooks = excel -> querySubObject("WorkBooks"); // obtain excel set
        workbooks -> dynamicCall("Add"); // create a new excel
        QAxObject *workbook = excel -> querySubObject("ActiveWorkBook"); // obtain current excel
        workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                                excelName, 51, QString(""), QString(""), false, false);//51xlsx, 56xls

        QAxObject *worksheets = workbook->querySubObject("WorkSheets"); // obtain sheets set
        QAxObject *worksheet = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
        worksheet -> setProperty("Name", "MeanValue");

        QList <QList<QVariant>> datas_sheet;
        for(V3DLONG i = 0; i <= z_size+1; i ++)
        {
            QList<QVariant> rows;
            if(i == 0)
            {
//                QString merge_cell1 = "B1:H1";
//                QAxObject *merge_range1 = worksheet->querySubObject("Range(const QString&)", merge_cell1);
//                merge_range1->setProperty("MergeCells", true);

                rows.append("Z");
                rows.append("C1");
                rows.append("C2");
                rows.append("C3");
            }
            else
            {
                rows.append(i);
                for(V3DLONG K = 0; K < 3; K ++)
                {
                    rows.append(pdataMean1[K][j][i-1]);
                }
            }
                datas_sheet.append(rows);
        }

        QList<QVariant> vars;
        for(auto v:datas_sheet)
            vars.append(QVariant(v));
        QVariant var = QVariant(vars);

        char a = (char)(3+48+1+16);
        cout<<"a_char = "<<a<<endl;
        string s(1, a);
        QString qs = QString ::fromStdString(s);
        QString num = QString::fromStdString(to_string(z_size+1));
        QString range = "A1:"+qs+num;
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

}

QString setTitleName(QString qname)
{
    string sname = qname.toStdString();
    string s = sname.substr(10, 2);
    string s1 = "Tile_"+s+"_Region_";
    QString qstr = QString :: fromStdString(s1);
    return qstr;

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
