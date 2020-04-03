#include <vector>
#include <v3d_interface.h>
#include <QAxObject>
#include "extractMean_gui.h"
#include <QDebug>

using namespace std;
bool calculate_mean(V3DPluginCallback2 &callback, vector<unsigned short *> & pdataMean, QFileInfoList imageList, V3DLONG rangeNum);
void saveMean_grayValue(QWidget *parent, vector<unsigned short *> & pdataMean, QDir dir, QString saveFileName, int z_size);

bool export_mean_range(V3DPluginCallback2 &callback, QWidget *parent)
{
    ExtractMeanCenterDialog dialog(parent);
    if(dialog.exec()!=QDialog::Accepted)return false;
    dialog.update();

    V3DLONG rangeNum = dialog.rangeBox -> value();

    QDir dir(dialog.openFolderFiles);

    QStringList filters;
    filters << "*.tif";
    QFileInfoList imageList = dir.entryInfoList(filters, QDir::Files);
    vector<unsigned short *> pdataMean(imageList.size());
    qDebug()<<"imageList.size = "<<imageList.size();

    if(!calculate_mean(callback, pdataMean, imageList, rangeNum))
    {
        return false;
    }
    cout<<"Calculate Mean successful !"<<endl;


    saveMean_grayValue(parent, pdataMean, dir, dialog.saveFolderFiles, imageList.size());
    cout<<"Save gray value successful !"<<endl;
    return 1;
}



void saveMean_grayValue(QWidget *parent, vector<unsigned short *> & pdataMean, QDir dir, QString saveFileName, int z_size)
{
    qDebug()<<__LINE__;
    cout<<"dir: "<<dir.dirName().toStdString().c_str()<<endl;
    QString excelName = saveFileName +"\\"+ dir.dirName() + "_Value.xlsx";
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
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet1 -> setProperty("Name", "MeanValue");

    QList <QList<QVariant>> datas_sheet;
    for(V3DLONG i = 0; i <= z_size; i ++)
    {
        QList<QVariant> rows;
        if(i ==0)
        {
            rows.append("");
            for(V3DLONG j = 0; j < pdataMean.size(); j ++)
            {
                QString name = "region_" + QString :: fromStdString(to_string((j+1)));
//                cout<<"Range name: "<<name.toStdString().c_str()<<endl;
                rows.append(name);
            }
        }
        else
        {
            rows.append(i);
            for(V3DLONG j = 0; j < pdataMean.size(); j ++)
            {
                rows.append(pdataMean[j][i-1]);
            }
        }
        datas_sheet.append(rows);
    }

    qDebug()<<__LINE__;
    QList<QVariant> vars;
    for(auto v:datas_sheet)
        vars.append(QVariant(v));
    QVariant var = QVariant(vars);

    char a = (char)(pdataMean.size()+48+1+16);
    cout<<"a_char = "<<a<<endl;
    string s(1, a);
    QString qs = QString ::fromStdString(s);
    QString num = QString::fromStdString(to_string(z_size+1));
    QString range = "A1:"+qs+num;
    cout<<"excel range: "<<range.toStdString()<<endl;

    QAxObject *excel_property = worksheet1 -> querySubObject("Range(const QString&)", range);
    excel_property -> setProperty("Value", var);
    excel_property -> setProperty("HorizontalAlignment", -4108);

    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel->dynamicCall("Quit(void)");
    delete excel;
}





bool calculate_mean(V3DPluginCallback2 &callback, vector<unsigned short *> & pdataMean, QFileInfoList imageList, V3DLONG rangeNum)
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

    V3DLONG region_center_X[5] = {10, 485, 960, 1435, 1910};
    V3DLONG region_center_Y[5] = {10, 485, 960, 1435, 1910};

    for(int i = 0; i < 5; i ++)
    {
        for(int j = 0; j < 5; j ++)
        {
            if(pdataMean[i*5+j]){delete []pdataMean[i*5+j]; pdataMean[i*5+j] = 0;}
            pdataMean[i*5+j] = new unsigned short[imageList.size()*1];
            V3DLONG start_X = region_center_X[i]-rangeNum/2;
            V3DLONG start_Y = region_center_Y[j]-rangeNum/2;
            V3DLONG end_X = region_center_X[i]+rangeNum/2-1;
            V3DLONG end_Y = region_center_Y[j]+rangeNum/2-1;

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
                pdataMean[5*i+j][image_i] = int(values+0.5);
            }

        }
    }
}
    
    
    
















    
