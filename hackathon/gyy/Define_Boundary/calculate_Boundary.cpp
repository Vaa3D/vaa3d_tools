#include <v3d_interface.h>
#include <QFileDialog>
#include <QAxObject>
#include <iostream>
#include <QDebug>
#include "point_class.h"


using namespace std;
int read_Excel_File(QString FileName, vector<vector<int> > &range_data_vec);
int write_Excel_File(QWidget *parent, QString saveName, vector<vector<Point_class>> point_3channel_vec, int pos1[3], int pos2[3]);
void set_3channel_range_20_data(vector<vector<int>> range_data_vec, vector<vector<Point_class>> &point_3channel_vec);
void set_3channel_filter_data(vector<vector<Point_class>> &point_3channel_vec);
void set_3channel_filter_thres_data(vector<vector<Point_class>> &point_3channel_vec);
void set_3channel_flag(vector<vector<Point_class>> &point_3channel_vec);
void find_boundary_position(vector<vector<Point_class>> point_3channel_vec, int pos1[3], int pos2[3]);
void write_sheet_file(QAxObject *worksheet, vector<vector<Point_class>> point_3channel_vec, int pos1[3], int pos2[3]);

bool calculate_tile_bountary(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString fileDir = QFileDialog ::getExistingDirectory(NULL, "Select range_20 data excel", "D:\\");
    QDir dir(fileDir);
    QStringList filters;
    filters << "*.xlsx"<<"*.xls";
    QFileInfoList range_20_data_list = dir.entryInfoList(filters, QDir::Files);
    qDebug()<<"excel size = "<<range_20_data_list.size();


    for(int i = 0; i < range_20_data_list.size(); i ++)
    {
        vector<vector<int> > range_data_vec;
        read_Excel_File(range_20_data_list[i].filePath(), range_data_vec);
        vector<vector<Point_class>> point_3channel_vec;
        cout<<"range_20_data_list = "<<range_20_data_list[i].fileName().toStdString()<<endl;

        qDebug()<<__LINE__;
        set_3channel_range_20_data(range_data_vec, point_3channel_vec);
        cout<<"filter_data1 = "<<point_3channel_vec[0][0].filter_data<<endl;
        qDebug()<<__LINE__;

        //error
        set_3channel_filter_data(point_3channel_vec);
        qDebug()<<__LINE__;
        set_3channel_filter_thres_data(point_3channel_vec);
        qDebug()<<__LINE__;
        set_3channel_flag(point_3channel_vec);
        int pos1[3] = {1, 1, 1}, pos2[3] = {1162,1162,1162};
        qDebug()<<__LINE__;
        find_boundary_position(point_3channel_vec,pos1, pos2);
        qDebug()<<__LINE__;


        QString saveName;
        saveName = fileDir + "\\" + range_20_data_list[i].baseName() + "_boundary.xlsx";
        cout<<"SaveName = "<<saveName.toStdString()<<endl;
        write_Excel_File(parent, saveName, point_3channel_vec,pos1, pos2);

    }
    return 1;

}


void set_3channel_range_20_data(vector<vector<int>> range_data_vec, vector<vector<Point_class>> &point_3channel_vec)
{
    for(int j = 0; j < 1162; j ++)
    {  
        vector<Point_class> point_vec;
        Point_class point1;
        point1.set_range_20_data(range_data_vec[j][0]);
        point_vec.push_back(point1);
        Point_class point2;
        point2.set_range_20_data(range_data_vec[j][1]);
        point_vec.push_back(point2);
        Point_class point3;
        point3.set_range_20_data(range_data_vec[j][2]);
        point_vec.push_back(point3);

        point_3channel_vec.push_back(point_vec);

    }
}

void set_3channel_filter_data(vector<vector<Point_class>> &point_3channel_vec)
{
    for(int m = 0; m < 1162; m ++)
    {
        for(int n = 0; n < 3; n ++)
        {
            if(m == 0)
            {
                Point_class p1, p2;
                Point_class point_arr[4]={p1, p2, point_3channel_vec[m+1][n], point_3channel_vec[m+2][n]};
                point_3channel_vec[m][n].set_filter_range(3);
                point_3channel_vec[m][n].set_filter_data(point_arr);

            }

             else if(m == 1)
            {
                Point_class p1;
                Point_class point_arr[4] = {p1, point_3channel_vec[m-1][n], point_3channel_vec[m+1][n], point_3channel_vec[m+2][n]};
                point_3channel_vec[m][n].set_filter_range(4);
                point_3channel_vec[m][n].set_filter_data(point_arr);
                point_3channel_vec[m][n].set_filter_shift_data(point_3channel_vec[m-1][n]);

            }

            else if (m == 1160)
            {
                Point_class p1;
                Point_class point_arr[4] = {p1, point_3channel_vec[m+1][n], point_3channel_vec[m-1][n], point_3channel_vec[m-2][n]};
                point_3channel_vec[m][n].set_filter_range(4);
                point_3channel_vec[m][n].set_filter_data(point_arr);
                point_3channel_vec[m][n].set_filter_shift_data(point_3channel_vec[m-1][n]);
            }

            else if(m == 1161)
            {
                Point_class p1, p2;
                Point_class point_arr[4]={p1, p2, point_3channel_vec[m-1][n], point_3channel_vec[m-2][n]};
                point_3channel_vec[m][n].set_filter_range(3);
                point_3channel_vec[m][n].set_filter_data(point_arr);
                point_3channel_vec[m][n].set_filter_shift_data(point_3channel_vec[m-1][n]);
            }

            else
            {
                Point_class point_arr[4] = {point_3channel_vec[m-2][n], point_3channel_vec[m-1][n], point_3channel_vec[m+1][n], point_3channel_vec[m+2][n]};
                point_3channel_vec[m][n].set_filter_data(point_arr);
                point_3channel_vec[m][n].set_filter_shift_data(point_3channel_vec[m-1][n]);
            }
        }
    }

}

void set_3channel_filter_thres_data(vector<vector<Point_class>> &point_3channel_vec)
{
    for(int m = 4; m < 1162; m ++)
    {
        for(int n = 0; n < 3; n ++)
        {
            Point_class point_arr[5] = {point_3channel_vec[m-4][n], point_3channel_vec[m-3][n], point_3channel_vec[m-2][n], point_3channel_vec[m-1][n], point_3channel_vec[m][n]};
            point_3channel_vec[m][n].set_filter_thres_data1(point_arr);
            point_3channel_vec[m][n].set_filter_thres_data2(point_arr);
        }
    }

/*
    for(int m = 1157; m >= 0; m --)
    {
        for(int n = 0; n < 3; n ++)
        {
            Point_class point_arr[5] = {point_3channel_vec[m+4][n], point_3channel_vec[m+3][n], point_3channel_vec[m+2][n], point_3channel_vec[m+1][n], point_3channel_vec[m][n]};
            point_3channel_vec[m][n].set_filter_thres_data2(point_arr);
        }
    }
 */

}


void set_3channel_flag(vector<vector<Point_class>> &point_3channel_vec)
{
    for(int m = 3; m < 1162; m ++)
    {
        for(int n = 0; n < 3; n ++)
        {
            point_3channel_vec[m][n].set_flag_data1(point_3channel_vec[m-3][n]);
        }
    }

    for(int m = 0; m < 1156; m ++)
    {
        for(int n = 0; n < 3; n ++)
        {
            point_3channel_vec[m][n].set_flag_data2(point_3channel_vec[m+1][n], point_3channel_vec[m+6][n]);
        }
    }
}


void find_boundary_position(vector<vector<Point_class>> point_3channel_vec, int pos1[3], int pos2[3])
{
    int count1[3] = {0};
    for(int m = 0; m < 1162; m ++)
    {
        if(point_3channel_vec[m][0].flag_data1 == 1)
        {
            count1[0] ++;
            if(count1[0] >= point_3channel_vec[0][0].flag_range)
            {
                pos1[0] = m - point_3channel_vec[0][0].flag_range+1+1;
                if(m+50<1162)
                    for(int n = m-10; n < m+20; n ++)
                    {
                        if(point_3channel_vec[n][1].flag_data1 == 1)
                        {
                            count1[1]++;
                            if(count1[1] >= point_3channel_vec[0][0].flag_range)
                            {
                                pos1[1] = n - point_3channel_vec[0][0].flag_range+1+1;
                                for(int k = n-10; k < n+20; k ++)
                                {
                                    if(point_3channel_vec[k][2].flag_data1 == 1)
                                    {
                                        count1[2]++;
                                        if(count1[2] >= point_3channel_vec[0][0].flag_range)
                                        {
                                            pos1[2] = k - point_3channel_vec[0][0].flag_range+1+1;
                                            break;
                                        }

                                    }
                                    else
                                        count1[2] = 0;
                                }
                                if(count1[2]<point_3channel_vec[0][0].flag_range)
                                {
                                    count1[0] = 0;
                                    count1[1] = 0;
                                    pos1[0] = 1;
                                    pos1[1] = 1;
                                }
                            }
                        }
                        else
                            count1[1] = 0;
                        if(pos1[2]!=1)
                            break;
                    }
                if(count1[1] < point_3channel_vec[0][0].flag_range)
                {
                        count1[0] = 0;
                        pos1[0] = 1;
                }
            }
        }
        else
            count1[0]=0;
        if(pos1[1]!=1)
            break;
    }


    int count2[3] = {0};
    for(int m = 1161; m > 0; m --)
    {
        if(point_3channel_vec[m][2].flag_data2 == 1)
        {
            count2[2] ++;
            if(count2[2] >= point_3channel_vec[0][0].flag_range)
            {
                pos2[2] = m + point_3channel_vec[0][0].flag_range;
                if(m-50>0)
                    for(int n = m+10; n > m-20; n --)
                    {
                        if(point_3channel_vec[n][1].flag_data2 == 1)
                        {
                            count2[1]++;
                            if(count2[1] >= point_3channel_vec[0][0].flag_range)
                            {
                                pos2[1] = n + point_3channel_vec[0][0].flag_range;
                                for(int k = n+10; k > n-20; k --)
                                {
                                    if(point_3channel_vec[k][0].flag_data2 == 1)
                                    {
                                        count2[0]++;
                                        if(count2[0] >= point_3channel_vec[0][0].flag_range)
                                        {
                                            pos2[0] = k + point_3channel_vec[0][0].flag_range;
                                            break;
                                        }

                                    }
                                    else
                                        count2[0] = 0;
                                }
                                if(count2[0]<point_3channel_vec[0][0].flag_range)
                                {
                                    count2[2] = 1;
                                    count2[1] = 1;
                                    pos2[2] = 1162;
                                    pos2[1] = 1162;
                                }
                            }
                        }
                        else
                            count2[1] = 0;
                        if(pos2[0]!=1162)
                            break;
                    }
                if(count2[1] < point_3channel_vec[0][0].flag_range)
                {
                        count2[2] = 0;
                        pos2[2] = 1162;
                }
            }
        }
        else
            count2[2]=0;
        if(pos2[1]!=1162)
            break;
    }


}

int read_Excel_File(QString FileName, vector<vector<int>> &range_data_vec)
{
    QAxObject *excel = NULL;
    QAxObject *workbooks = NULL;
    QAxObject *workbook = NULL;
    excel = new QAxObject("Excel.Application");
    if(!excel)
    {
        qDebug()<<"EXCEL is lost !";
    }
    qDebug()<<__LINE__;

    excel -> dynamicCall("SetVisible(bool)", false);
    workbooks = excel->querySubObject("WorkBooks"); // excel set
    workbook = workbooks->querySubObject("Open(QString, QVariant)", FileName);

    qDebug()<<__LINE__;
    QAxObject * worksheets = workbook -> querySubObject("Sheets");
    QAxObject * worksheet = worksheets -> querySubObject("Item(int)", 1);
    qDebug()<<__LINE__;

    QAxObject * usedrange = worksheet->querySubObject("UsedRange");

    qDebug()<<__LINE__;
    QAxObject * rows = usedrange->querySubObject("Rows");
    QAxObject * columns = usedrange->querySubObject("Columns");
    int intRows = rows->property("Count").toInt();
    int intCols = columns->property("Count").toInt();

    qDebug()<<"Excel rows = "<<intRows;
    qDebug()<<"Excel columns = "<<intCols;

    //Load data in batches
    QString Range = "A2:C1163";
    cout<<"Excel Range: "<<Range.toStdString()<<endl;

    QAxObject *allExcelData = worksheet->querySubObject("Range(QString)", Range);
    qDebug()<<__LINE__;
    QVariant allExcelDataQVariant = allExcelData->property("Value");
    qDebug()<<__LINE__;
    QVariantList allExcelDaraList = allExcelDataQVariant.toList();
    qDebug()<<__LINE__;

    for(int i = 0; i <= intRows-2; i ++)
    {
        QVariantList allExcelDaraList_i = allExcelDaraList[i].toList();
        vector<int> vec;
        for(int j = 0; j < intCols; j ++)
        {
            vec.push_back(allExcelDaraList_i[j].toInt());
        }
        range_data_vec.push_back(vec);
    }
    cout<<"Read excel successful !"<<endl;
    workbook->dynamicCall("Close()");
    return 0;

}

int write_Excel_File(QWidget *parent, QString saveName, vector<vector<Point_class>> point_3channel_vec, int pos1[3], int pos2[3])
{
    QAxObject *excel = new QAxObject(parent);
    excel -> setControl("Excel.Application"); // connect EXCEL control
    excel -> setProperty("DisplayAlerts", true); // display window
    QAxObject *workbooks = excel -> querySubObject("WorkBooks"); // obtain excel set
    workbooks -> dynamicCall("Add"); // create a new excel
    QAxObject *workbook = excel -> querySubObject("ActiveWorkBook"); // obtain current excel
    workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                            saveName, 51, QString(""), QString(""), false, false);
    //51xlsx, 56xls
    QAxObject *worksheets = workbook->querySubObject("WorkSheets"); // obtain sheets set
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet1 -> setProperty("Name", "sheet1");

    write_sheet_file(worksheet1, point_3channel_vec, pos1, pos2);

    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel->dynamicCall("Quit(void)");
    delete excel;
    return 1;
}

void write_sheet_file(QAxObject *worksheet, vector<vector<Point_class>> point_3channel_vec, int pos1[3], int pos2[3])
{
    QList<QList<QVariant>> datas_sheet;
    for(int i = 0; i < 1164; i ++)
    {
        QList <QVariant> rows;
        if(i == 0)
        {
            QString merge_cell1 = "B1:D1";
            QAxObject *merge_range1 = worksheet->querySubObject("Range(const QString&)", merge_cell1);
            merge_range1->setProperty("MergeCells", true);

            QString merge_cell2 = "F1:H1";
            QAxObject *merge_range2 = worksheet->querySubObject("Range(const QString&)", merge_cell2);
            merge_range2->setProperty("MergeCells", true);

            QString merge_cell3 = "J1:L1";
            QAxObject *merge_range3 = worksheet->querySubObject("Range(const QString&)", merge_cell3);
            merge_range3->setProperty("MergeCells", true);

            QString merge_cell4 = "N1:P1";
            QAxObject *merge_range4 = worksheet->querySubObject("Range(const QString&)", merge_cell4);
            merge_range4->setProperty("MergeCells", true);

            QString merge_cell5 = "R1:T1";
            QAxObject *merge_range5 = worksheet->querySubObject("Range(const QString&)", merge_cell5);
            merge_range5->setProperty("MergeCells", true);

            QString merge_cell6 = "V1:X1";
            QAxObject *merge_range6 = worksheet->querySubObject("Range(const QString&)", merge_cell6);
            merge_range6->setProperty("MergeCells", true);

            QString merge_cell7 = "Z1:AB1";
            QAxObject *merge_range7 = worksheet->querySubObject("Range(const QString&)", merge_cell7);
            merge_range7->setProperty("MergeCells", true);

            rows.append("");
            rows.append("filter");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("filter_shift");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("Statical standard");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("First Bountary");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("Statical standard");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("Last Bountary");
            rows.append("");
            rows.append("");
            rows.append("");
            rows.append("Center");
            rows.append("");
            rows.append("");
        }

        else if(i == 1)
        {
            rows.append("Z");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");

            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");

            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");

            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");

            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");

            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");

            rows.append("");
            rows.append("C1");
            rows.append("C2");
            rows.append("C3");
        }

        else if(i == 2)
        {
            rows.append(i-1);
            rows.append(point_3channel_vec[i-2][0].filter_data);
            rows.append(point_3channel_vec[i-2][1].filter_data);
            rows.append(point_3channel_vec[i-2][2].filter_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_shift_data);
            rows.append(point_3channel_vec[i-2][1].filter_shift_data);
            rows.append(point_3channel_vec[i-2][2].filter_shift_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data1);
            rows.append(point_3channel_vec[i-2][1].flag_data1);
            rows.append(point_3channel_vec[i-2][2].flag_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data2);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data2);
            rows.append(point_3channel_vec[i-2][1].flag_data2);
            rows.append(point_3channel_vec[i-2][2].flag_data2);
            rows.append("");

            rows.append(pos1[0]);
            rows.append(pos1[1]);
            rows.append(pos1[2]);

        }

        else if(i == 3)
        {
            rows.append(i-1);
            rows.append(point_3channel_vec[i-2][0].filter_data);
            rows.append(point_3channel_vec[i-2][1].filter_data);
            rows.append(point_3channel_vec[i-2][2].filter_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_shift_data);
            rows.append(point_3channel_vec[i-2][1].filter_shift_data);
            rows.append(point_3channel_vec[i-2][2].filter_shift_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data1);
            rows.append(point_3channel_vec[i-2][1].flag_data1);
            rows.append(point_3channel_vec[i-2][2].flag_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data2);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data2);
            rows.append(point_3channel_vec[i-2][1].flag_data2);
            rows.append(point_3channel_vec[i-2][2].flag_data2);
            rows.append("");

            rows.append(pos2[0]);
            rows.append(pos2[1]);
            rows.append(pos2[2]);

        }

        else if(i == 4)
        {
            rows.append(i-1);
            rows.append(point_3channel_vec[i-2][0].filter_data);
            rows.append(point_3channel_vec[i-2][1].filter_data);
            rows.append(point_3channel_vec[i-2][2].filter_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_shift_data);
            rows.append(point_3channel_vec[i-2][1].filter_shift_data);
            rows.append(point_3channel_vec[i-2][2].filter_shift_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data1);
            rows.append(point_3channel_vec[i-2][1].flag_data1);
            rows.append(point_3channel_vec[i-2][2].flag_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data2);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data2);
            rows.append(point_3channel_vec[i-2][1].flag_data2);
            rows.append(point_3channel_vec[i-2][2].flag_data2);
            rows.append("");

            rows.append((pos1[0]+pos2[0])/2);
            rows.append((pos1[1]+pos2[1])/2);
            rows.append((pos1[2]+pos2[2])/2);

        }

        else
        {
            rows.append(i-1);
            rows.append(point_3channel_vec[i-2][0].filter_data);
            rows.append(point_3channel_vec[i-2][1].filter_data);
            rows.append(point_3channel_vec[i-2][2].filter_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_shift_data);
            rows.append(point_3channel_vec[i-2][1].filter_shift_data);
            rows.append(point_3channel_vec[i-2][2].filter_shift_data);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data1);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data1);
            rows.append(point_3channel_vec[i-2][1].flag_data1);
            rows.append(point_3channel_vec[i-2][2].flag_data1);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][1].filter_thres_data2);
            rows.append(point_3channel_vec[i-2][2].filter_thres_data2);
            rows.append("");

            rows.append(point_3channel_vec[i-2][0].flag_data2);
            rows.append(point_3channel_vec[i-2][1].flag_data2);
            rows.append(point_3channel_vec[i-2][2].flag_data2);
            rows.append("");

        }

        datas_sheet.append(rows);
    }
    QList <QVariant> vars;
    for(auto v: datas_sheet)
        vars.append(QVariant(v));
    QVariant var = QVariant(vars);
    QString big_range = "A1:AB1164";
    QAxObject *excel_property = worksheet -> querySubObject("Range(const QString&)", big_range);
    excel_property -> setProperty("Value", var);
    excel_property -> setProperty("HorizontalAlignment", -4108);


}
