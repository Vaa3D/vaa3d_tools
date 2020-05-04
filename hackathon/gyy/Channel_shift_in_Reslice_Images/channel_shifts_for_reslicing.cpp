#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <iostream>
#include <QAxObject>
using namespace std;

bool calculate_center_MIP(V3DPluginCallback2 &callback, QFileInfoList imageList, vector<unsigned short*> &pdataPro, int c);
bool channel_shifts_for_reslicing_image(V3DPluginCallback2 &callback, QWidget *parent);
bool save_projection_files(V3DPluginCallback2 &callback, vector<vector<unsigned short*>> pdataPro, QFileInfoList imageList, QString SaveFolder, string snum);
void calculate_2C_shift(unsigned short * &pdata1, unsigned short * &pdata2, int &xyShift, int &zShift, int XY_shiftRange, int Z_shiftRange);
void save_shift_to_excel(QWidget *parent, QString SaveFolder, string snum, vector<vector<int>>  shiftsXY, vector<vector<int>>  shiftsZ);

bool channel_shifts_for_reslicing_image(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString tileFolderFiles = QFileDialog::getExistingDirectory(NULL, "Select tile folder","D:\\");
    qDebug()<<"Tile1: "<<tileFolderFiles.toStdString().c_str();
    QString SaveFolder = QFileDialog :: getExistingDirectory(0, "Select folder to save", "D:\\");
    qDebug()<<"SaveFolder: "<<SaveFolder.toStdString().c_str();

    QDir tileDir(tileFolderFiles);
    if(!tileDir.exists())
        return false;
    tileDir.setFilter(QDir::Dirs);
    QFileInfoList channelList = tileDir.entryInfoList();
    qDebug()<<"tileDir.size = "<<channelList.size();
    string sname = tileDir.dirName().toStdString();
    string snum = sname.substr(10, 2);

    vector<vector<unsigned short*>> pdataPro(3, vector<unsigned short*>(2, 0));
    vector<vector<int>> shiftsXY(2, vector<int>(2, 0));
    vector<vector<int>> shiftsZ(2, vector<int>(2, 0));
    const int XY_shiftRange = 5, Z_shiftRange = 20;

    QDir dir[3];
    QFileInfoList imageList[3];
    for(int i = 0; i < channelList.size()-2; i ++)
    {
        dir[i] = channelList[i+2].filePath();

        QStringList filters;
        filters << "*.tif" << "*.v3draw";
        imageList[i] = dir[i].entryInfoList(filters, QDir::Files);

        if(!calculate_center_MIP(callback, imageList[i], pdataPro[i], i))
            return false;
    }

    if(!save_projection_files(callback, pdataPro, imageList[0], SaveFolder, snum))
        return false;

    for(int i = 0; i < 2; i ++)
    {
        calculate_2C_shift(pdataPro[0][i], pdataPro[1][i], shiftsXY[0][i], shiftsZ[0][i], XY_shiftRange, Z_shiftRange);
        calculate_2C_shift(pdataPro[0][i], pdataPro[2][i], shiftsXY[1][i], shiftsZ[1][i], XY_shiftRange, Z_shiftRange);
    }

    save_shift_to_excel(parent, SaveFolder, snum, shiftsXY, shiftsZ);

    qDebug()<<"XY_shift13 = "<<shiftsXY[0][0];
    qDebug()<<"Z_shift13 = "<<shiftsZ[0][0];//2
    qDebug()<<"XY_shift15 = "<<shiftsXY[1][0];
    qDebug()<<"Z_shift15 = "<<shiftsZ[1][0];//12
    qDebug()<<"XY_shift24 = "<<shiftsXY[0][1];
    qDebug()<<"Z_shift24 = "<<shiftsZ[0][1];//2
    qDebug()<<"XY_shift26 = "<<shiftsXY[1][1];
    qDebug()<<"Z_shift26 = "<<shiftsZ[1][1];//12

    return 1;
}

bool calculate_center_MIP(V3DPluginCallback2 &callback, QFileInfoList imageList, vector<unsigned short *> &pdataPro, int c)
{
    vector<unsigned short *> data1d_copy(1162);
    vector<unsigned char *> data1d_vec(1162);
    vector<V3DLONG *> in_sz_vec(1162);
    vector<int> datatype_vec(1162);
    for(V3DLONG i = 0; i < 1162; i++)
    {
        cout<<"This is "<<i+1<<" image for channel " <<c+1<<"."<<endl;
        in_sz_vec[i] = new V3DLONG[4];
        if(!simple_loadimage_wrapper(callback, imageList[i].filePath().toStdString().c_str(), data1d_vec[i], in_sz_vec[i], datatype_vec[i]))
        {
            qDebug()<< "Error happens in reading the subject file.";
            return false;
        }

        if(data1d_copy[i]){delete []data1d_copy[i]; data1d_copy[i] = 0;}
        data1d_copy[i] = new unsigned short[in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3] *datatype_vec[i]/2];
        memcpy(data1d_copy[i], data1d_vec[i], in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3]*datatype_vec[i]);
    }

    if(pdataPro[0]) {delete []pdataPro[0]; pdataPro[0] = 0;}
    pdataPro[0] = new unsigned short[in_sz_vec[0][0]*imageList.size()*datatype_vec[0]/2];
    V3DLONG centerY = 960; // XZ plane
    for(int i = 0; i < imageList.size(); i ++)
    {
        for(int j = 0; j < 1920; j ++)
        {
            int maxValue = data1d_copy[i][j*1920+centerY-10];
            for(int k = centerY - 10; k < centerY + 10; k ++)
                maxValue = ((maxValue >= data1d_copy[i][k*1920+j]) ? maxValue : data1d_copy[i][k*1920+j]);
            pdataPro[0][i*1920+j] = maxValue;
        }
    }

    if(pdataPro[1]) {delete []pdataPro[1]; pdataPro[1] = 0;}
    pdataPro[1] = new unsigned short[in_sz_vec[0][0]*imageList.size()*datatype_vec[0]/2];
    V3DLONG centerX = 960; // YZ plane
    for(int i = 0; i < imageList.size(); i ++)
    {
        for(int j = 0; j < 1920; j ++)
        {
            int maxValue = data1d_copy[i][j*1920+centerX-10];
            for(int k = centerX - 10; k < centerX + 10; k ++)
                maxValue = ((maxValue >= data1d_copy[i][j*1920+k]) ? maxValue : data1d_copy[i][j*1920+k]);
            pdataPro[1][i*1920+j] = maxValue;
        }
    }

}

bool save_projection_files(V3DPluginCallback2 &callback, vector<vector<unsigned short*>> pdataPro, QFileInfoList imageList, QString SaveFolder, string snum)
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

    for(int i = 0; i < 3; i ++)
    {
        qDebug()<<__LINE__<<"i = "<<i;
        // Save XZ plane
        string saveXZ = "Tile"+snum+"_C"+to_string(i+1)+"_XZ.v3draw";
        QString saveNameXZ = SaveFolder + "\\" + QString::fromStdString(saveXZ);

        unsigned char * data1dXZ = new unsigned char[in_sz[0]*imageList.size()*datatype];
        memcpy(data1dXZ, pdataPro[i][0], in_sz[0]*imageList.size()*datatype);
        V3DLONG in_sz_XZ[4] = {in_sz[0], imageList.size(), 1, 1};
        qDebug()<<__LINE__<<": data1dXZ[0] = "<<data1dXZ[0];
        if(simple_saveimage_wrapper(callback, saveNameXZ.toStdString().c_str(), data1dXZ, in_sz_XZ, datatype))
        {
            qDebug()<<"Save XZ plane successfully!";
        }

        // Save YZ plane
        string saveYZ = "Tile"+snum+"_C"+to_string(i+1)+"_YZ.v3draw";
        QString saveNameYZ = SaveFolder + "\\" + QString::fromStdString(saveYZ);

        unsigned char * data1dYZ = new unsigned char[in_sz[0]*imageList.size()*datatype];
        memcpy(data1dYZ, pdataPro[i][1], in_sz[0]*imageList.size()*datatype);
        V3DLONG in_sz_YZ[4] = {in_sz[0], imageList.size(), 1, 1};
        qDebug()<<__LINE__<<": data1dYZ[0] = "<<data1dYZ[0];
        if(simple_saveimage_wrapper(callback, saveNameYZ.toStdString().c_str(), data1dYZ, in_sz_YZ, datatype))
        {
            qDebug()<<"Save YZ plane successfully!";
        }
    }
}


void calculate_2C_shift(unsigned short * &pdata1, unsigned short * &pdata2, int &xyShift, int &zShift, int XY_shiftRange, int Z_shiftRange)
{
    vector<V3DLONG> sumShift(2*XY_shiftRange*Z_shiftRange, 0);
    vector<double> difference(2*XY_shiftRange*Z_shiftRange, 0);
    vector<V3DLONG> count(2*XY_shiftRange*Z_shiftRange, 0);
    double minError = INT_MAX;

    for(int rz = 0; rz < Z_shiftRange; rz ++)
    {
        for(int rxy = -XY_shiftRange; rxy < XY_shiftRange; rxy ++)
        {
            int index = (2 * XY_shiftRange) * rz + rxy + XY_shiftRange;
            for(int row = 0; row < 1162 - rz; row ++)
            {
                if(rxy<0){
                for(int col = 0; col < 1920 + rxy; col ++)
                {
                    int diff = pdata1[row * 1920 + col ] - pdata2[(row + rz) * 1920 + col-rxy];
                    sumShift[index] += diff * diff;
                    count[index] ++;
                }
             }
             else{
                for(int col = 0; col < 1920 - rxy; col ++)
                {
                    int diff = pdata1[row * 1920 + col +rxy] - pdata2[(row + rz) * 1920 + col];
                    sumShift[index] += diff * diff;
                    count[index] ++;
                }
                    }
            }
            difference[index] = (sumShift[index]+0.0)/count[index];

            if(minError > difference[index])
            {
                minError = difference[index];
                zShift = rz;
                xyShift = rxy;
            }
        }
    }
}

void save_shift_to_excel(QWidget *parent, QString SaveFolder, string snum, vector<vector<int> > shiftsXY, vector<vector<int> > shiftsZ)
{
    string ss = "Tile"+snum+"_XZ_YZ_shift.xlsx";
    QString excelName = SaveFolder + "\\" + QString :: fromStdString(ss);
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
    worksheet -> setProperty("Name", "XYZ_shift");

    QList <QList<QVariant>> datas_sheet;
    string sname = "Tile" + snum;
    QString qname = QString :: fromStdString(sname);

    for(int i = 0; i < 4; i ++)
    {
        QList<QVariant> rows;
        if(i == 0)
        {
            rows.append(qname);
            QString merge_cell1 = "B1:C1";
            QAxObject *merge_range1 = worksheet->querySubObject("Range(const QString&)", merge_cell1);
            merge_range1->setProperty("MergeCells", true);

            QString merge_cell2 = "D1:E1";
            QAxObject *merge_range2 = worksheet->querySubObject("Range(const QString&)", merge_cell2);
            merge_range2->setProperty("MergeCells", true);
            rows.append("XY");
            rows.append("");
            rows.append("Z");
            rows.append("");
        }
        else if(i == 1)
        {
            rows.append("");
            rows.append("C2-C1");
            rows.append("C3-C1");
            rows.append("C2-C1");
            rows.append("C3-C1");
        }
        else if(i == 2)
        {
            rows.append("XZ");
            rows.append(shiftsXY[i-2][0]);
            rows.append(shiftsXY[i-1][0]);
            rows.append(shiftsZ[i-2][0]);
            rows.append(shiftsZ[i-1][0]);
        }
        else
        {
            rows.append("YZ");
            rows.append(shiftsXY[i-3][1]);
            rows.append(shiftsXY[i-2][1]);
            rows.append(shiftsZ[i-3][1]);
            rows.append(shiftsZ[i-2][1]);
        }
        datas_sheet.append(rows);
    }
    QList<QVariant> vars;
    for(auto v:datas_sheet)
        vars.append(QVariant(v));
    QVariant var = QVariant(vars);

    QString range = "A1:E4";
    cout<<"excel range: "<<range.toStdString()<<endl;

    QAxObject *excel_property = worksheet -> querySubObject("Range(const QString&)", range);
    excel_property -> setProperty("Value", var);
    excel_property -> setProperty("HorizontalAlignment", -4108);

    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel->dynamicCall("Quit(void)");
    delete excel;

}
