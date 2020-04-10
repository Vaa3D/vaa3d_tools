#include <vector>
#include <v3d_interface.h>
#include <QDebug>
#include <iostream>
#include <QAxObject>

using namespace std;

bool Calculate_Z_Shift_tiles(V3DPluginCallback2 &callback, QWidget *parent);
bool calculate_region_mean(V3DPluginCallback2 &callback, vector<unsigned short *> & pdataMean, QFileInfoList imageList, QString orientation, int Y_shift, int X_overlap);
void saveMean_grayValue(QWidget *parent, vector<unsigned short *> & pdataMean1, vector<unsigned short *> & pdataMean2, QDir dir1, QDir dir2, QString saveFileName, int z_size);

bool Calculate_Z_Shift_tiles(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString tileFolderFiles1 = QFileDialog::getExistingDirectory(NULL, "Select tile1 folder","D:\\");
    QString tileFolderFiles2 = QFileDialog :: getExistingDirectory(0, "Select tile2 folder", tileFolderFiles1);
    QString SaveFolder = QFileDialog :: getExistingDirectory(0, "Select folder to save", tileFolderFiles1);


    QDir dir1(tileFolderFiles1);
    QDir dir2(tileFolderFiles2);
    if(!dir1.exists() || !dir2.exists())
        return false;

    QStringList filters;
    filters << "*.tif";
    QFileInfoList imageList1 = dir1.entryInfoList(filters, QDir::Files);
    QFileInfoList imageList2 = dir2.entryInfoList(filters, QDir::Files);
    vector<unsigned short *> pdataMean1(7);
    vector<unsigned short *> pdataMean2(7);

    int Y_shift = 10, X_overlap = 294;
    qDebug()<<__LINE__<<": dirName = "<<dir1.dirName().toStdString().c_str();
    QString orientation1 = "right";
    QString orientation2 = "left";

    if(!calculate_region_mean(callback, pdataMean1, imageList1, orientation1, Y_shift, X_overlap))
    {
        return false;
    }
    cout<<"Calculate Mean2 successful !"<<endl;
    if(!calculate_region_mean(callback, pdataMean2, imageList2, orientation2, Y_shift, X_overlap))
    {
        return false;
    }
    cout<<"Calculate Mean1 successful !"<<endl;

    saveMean_grayValue(parent, pdataMean1, pdataMean2, dir1, dir2, SaveFolder, imageList1.size());
    cout<<"Save gray value successful !"<<endl;
    return 1;
}


bool calculate_region_mean(V3DPluginCallback2 &callback, vector<unsigned short *> & pdataMean, QFileInfoList imageList, QString orientation, int Y_shift, int X_overlap)
{
    vector<unsigned char *> data1d_vec(imageList.size());
    vector<V3DLONG *> in_sz_vec(imageList.size());
    vector<int> datatype_vec(imageList.size());
    vector<unsigned short *> data1d_copy(imageList.size());
    for(V3DLONG i = 0; i < imageList.size(); i++)
    {
        cout<<"This is "<<i+1<<" image"<<endl;
        in_sz_vec[i] = new V3DLONG[4];
        if(!simple_loadimage_wrapper(callback, imageList[i].filePath().toStdString().c_str(), data1d_vec[i], in_sz_vec[i], datatype_vec[i]))
        {
            qDebug()<< "Error happens in reading the subject file.";
            return false;
        }

        data1d_copy[i] = new unsigned short[in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3] *datatype_vec[i]/2];
        memcpy(data1d_copy[i], data1d_vec[i], in_sz_vec[i][0]*in_sz_vec[i][1]*in_sz_vec[i][2]*in_sz_vec[i][3]*datatype_vec[i]);
    }

    qDebug()<<__LINE__;

    V3DLONG region_center_X = 0;
    V3DLONG region_center_Y[7];
    if(orientation == "right")
    {

        region_center_X = 1920-X_overlap/2;
        region_center_Y[0] = 10;
        region_center_Y[3] = (1920-Y_shift)/2;
        region_center_Y[6] = 1910-Y_shift;
        region_center_Y[1] = (region_center_Y[3]-region_center_Y[0])/3+region_center_Y[0];
        region_center_Y[2] = (region_center_Y[3]-region_center_Y[0])/3*2+region_center_Y[0];
        region_center_Y[4] = (region_center_Y[6]-region_center_Y[3])/3+region_center_Y[3];
        region_center_Y[5] = (region_center_Y[6]-region_center_Y[3])/3*2+region_center_Y[3];
    }

    if(orientation == "left")
    {

        region_center_X = X_overlap/2;
        region_center_Y[0] = 10+Y_shift;
        region_center_Y[3] = (1920+Y_shift)/2;
        region_center_Y[6] = 1910;
        region_center_Y[1] = (region_center_Y[3]-region_center_Y[0])/3+region_center_Y[0];
        region_center_Y[2] = (region_center_Y[3]-region_center_Y[0])/3*2+region_center_Y[0];
        region_center_Y[4] = (region_center_Y[6]-region_center_Y[3])/3+region_center_Y[3];
        region_center_Y[5] = (region_center_Y[6]-region_center_Y[3])/3*2+region_center_Y[3];
    }

    qDebug()<<__LINE__<<" : 2-"<<region_center_Y[1]<<"; 4-"<<region_center_Y[3];
    for(int i = 0; i < 7; i ++)
    {
        if(pdataMean[i]){delete []pdataMean[i]; pdataMean[i] = 0;}
        pdataMean[i] = new unsigned short[imageList.size()*1];
        V3DLONG start_X = region_center_X-10;
        V3DLONG start_Y = region_center_Y[i]-10;
        V3DLONG end_X = region_center_X+10-1;
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


void saveMean_grayValue(QWidget *parent, vector<unsigned short *> & pdataMean1, vector<unsigned short *> & pdataMean2, QDir dir1, QDir dir2, QString saveFileName, int z_size)
{
        QString dirNames(dir1.dirName());
        QString excelName = saveFileName +"\\Tiles_"+dirNames+"_Value.xlsx";
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
                QString merge_cell1 = "B1:H1";
                QAxObject *merge_range1 = worksheet->querySubObject("Range(const QString&)", merge_cell1);
                merge_range1->setProperty("MergeCells", true);

                QString merge_cell2 = "J1:P1";
                QAxObject *merge_range2 = worksheet->querySubObject("Range(const QString&)", merge_cell2);
                merge_range2->setProperty("MergeCells", true);

                rows.append("");
                rows.append("Tile1");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("Tile2");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("");
                rows.append("");
            }
            else if(i == 1)
            {
                rows.append("");
                rows.append("Region_1");
                rows.append("Region_2");
                rows.append("Region_3");
                rows.append("Region_4");
                rows.append("Region_5");
                rows.append("Region_6");
                rows.append("Region_7");
                rows.append("");
                rows.append("Region_1");
                rows.append("Region_2");
                rows.append("Region_3");
                rows.append("Region_4");
                rows.append("Region_5");
                rows.append("Region_6");
                rows.append("Region_7");
            }
            else
            {
                rows.append(i-1);
                for(V3DLONG j = 0; j < pdataMean1.size(); j ++)
                {
                    rows.append(pdataMean1[j][i-2]);
                }
                rows.append("");
                for(V3DLONG j = 0; j < pdataMean2.size(); j ++)
                {
                    rows.append(pdataMean2[j][i-2]);
                }
            }
                datas_sheet.append(rows);
        }


            QList<QVariant> vars;
            for(auto v:datas_sheet)
                vars.append(QVariant(v));
            QVariant var = QVariant(vars);

            char a = (char)(pdataMean1.size()*2+48+2+16);
            cout<<"a_char = "<<a<<endl;
            string s(1, a);
            QString qs = QString ::fromStdString(s);
            QString num = QString::fromStdString(to_string(z_size+2));
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
