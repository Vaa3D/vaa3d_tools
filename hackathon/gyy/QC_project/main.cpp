#include "mainwindow.h"
#include "x_y_shift.h"
#include "matsource.h"
#include <QApplication>
#include "opencv2/opencv.hpp"
#include "opencv2/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/stitching.hpp"
#include <QFileDialog>
#include <iostream>
#include <vector>
#include <algorithm>
#include <QAxObject>
#include <QMainWindow>
#include "matsource.h"

using namespace std;
using namespace cv;

void save_tiles_shift(vector<x_y_shift> XY_shift_vec, QString saveName);

int main(int argc, char *argv[])
{
      QApplication a(argc, argv);
      cout<<"Start!"<<endl;

      QString TilesName = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
      cout<<"TileName: "<<TilesName.toStdString().c_str()<<endl;
//      QString savePath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save","D:\\");
//      cout<<"savePath: "<<savePath.toStdString().c_str()<<endl;

      QDir tileDir(TilesName);
      QStringList filters;
      filters << "*.tif" << "*.tiff";
      QFileInfoList imageList = tileDir.entryInfoList(filters, QDir::Files);
      vector<Mat> imagesMat;
      for(int i = 0; i < imageList.size(); i ++)
      {
          cout<<"image["<<i<<"]: "<<imageList[i].filePath().toStdString().c_str()<<endl;
          Mat tempImage=imread(imageList[i].filePath().toStdString(),2);
          imagesMat.push_back(tempImage);
      }

      x_y_shift result = MatSource::mat_x_y_shift(imagesMat[4], imagesMat[8], "down",  160);
      cout<<result.x<<endl;
      cout<<result.y<<endl;


      /*
      vector<x_y_shift> XY_shift_vec;
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[0], imagesMat[1], "left",   200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[0], imagesMat[4], "down",   160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[1], imagesMat[2], "left",   200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[1], imagesMat[5], "down",  160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[2], imagesMat[3], "left",   200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[2], imagesMat[6], "down",  160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[3], imagesMat[7], "down",  160));

      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[4], imagesMat[5], "left",  200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[4], imagesMat[8], "down",  160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[5], imagesMat[6], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[5], imagesMat[9], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[6], imagesMat[7], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[6], imagesMat[10], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[7], imagesMat[11], "down", 160));

      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[8], imagesMat[9], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[8], imagesMat[12], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[9], imagesMat[10], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[9], imagesMat[13], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[10], imagesMat[11], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[10], imagesMat[14], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[11], imagesMat[15], "down", 160));

      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[12], imagesMat[13], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[12], imagesMat[16], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[13], imagesMat[14], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[13], imagesMat[17], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[14], imagesMat[15], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[14], imagesMat[18], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[15], imagesMat[19], "down", 160));

      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[16], imagesMat[17], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[16], imagesMat[20], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[17], imagesMat[18], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[17], imagesMat[21], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[18], imagesMat[19], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[18], imagesMat[22], "down", 160));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[19], imagesMat[23], "down", 160));

      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[20], imagesMat[21], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[21], imagesMat[22], "left", 200));
      XY_shift_vec.push_back(MatSource::mat_x_y_shift(imagesMat[22], imagesMat[23], "left", 200));

      QString saveName = savePath + "\\24_Tiles_XY_shift.xlsx";
      save_tiles_shift(XY_shift_vec, saveName);
*/





      /*
      QString TileName11 = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder","D:\\");
      cout<<"TileName: "<<TileName11.toStdString().c_str()<<endl;
      QString TileName21 = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder",TileName11);
      cout<<"TileName: "<<TileName21.toStdString().c_str()<<endl;

      QString TileName12 = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder",TileName11);
      cout<<"TileName: "<<TileName12.toStdString().c_str()<<endl;
      QString TileName22 = QFileDialog::getExistingDirectory(nullptr, "Select tile1 folder",TileName12);
      cout<<"TileName: "<<TileName22.toStdString().c_str()<<endl;

      //QString savePath = QFileDialog::getExistingDirectory(nullptr, "Select a folder to save","D:\\");
      //cout<<"savePath: "<<savePath.toStdString().c_str()<<endl;

      vector<String> tile11 = MatSource::readMats(TileName11);
      vector<String> tile21 = MatSource::readMats(TileName21);
      vector<String> tile12 = MatSource::readMats(TileName12);
      vector<String> tile22 = MatSource::readMats(TileName22);


      int Zshift11_12=MatSource::A_B_z_shift(tile11,tile12,200,"left",1600,2000,10);
      int Zshift11_21=MatSource::A_B_z_shift(tile11,tile21,160,"down",1600,2000,10);
      int Zshift12_22=MatSource::A_B_z_shift(tile12,tile22,160,"down",1600,2000,10);
      int Zshift21_22=MatSource::A_B_z_shift(tile21,tile22,200,"left",1600,2000,10);
      cout<<"Zshift11_12 = "<<Zshift11_12<<endl;
      cout<<"Zshift11_21 = "<<Zshift11_21<<endl;
      cout<<"Zshift12_22 = "<<Zshift12_22<<endl;
      cout<<"Zshift21_22 = "<<Zshift21_22<<endl;
*/

      /*
      Mat MIP11_right = MatSource::getMIP(tile11,200,"left", 1600,2000,10);
      Mat MIP11_down  = MatSource::getMIP(tile11,160,"down",   1600,2000,10);
      Mat MIP21_right = MatSource::getMIP(tile21,200,"left", 1600,2000,10);
      Mat MIP21_up    = MatSource::getMIP(tile21,160,"up", 1600,2000,10);
      Mat MIP12_left  = MatSource::getMIP(tile12,200,"right",1600,2000,10);
      Mat MIP12_down  = MatSource::getMIP(tile12,160,"down",   1600,2000,10);
      Mat MIP22_left  = MatSource::getMIP(tile22,200,"right",1600,2000,10);
      Mat MIP22_up    = MatSource::getMIP(tile22,160,"up", 1600,2000,10);

      QString path11_right = savePath+"\\MIP11_right.tif";
      imwrite(path11_right.toStdString(), MIP11_right);
      QString path11_down = savePath+"\\MIP11_down.tif";
      imwrite(path11_down.toStdString(), MIP11_down);

      QString path21_right = savePath+"\\MIP21_right.tif";
      imwrite(path21_right.toStdString(), MIP21_right);
      QString path21_up = savePath+"\\MIP21_up.tif";
      imwrite(path21_up.toStdString(), MIP21_up);

      QString path12_left = savePath+"\\MIP12_left.tif";
      imwrite(path12_left.toStdString(), MIP12_left);
      QString path12_down = savePath+"\\MIP12_down.tif";
      imwrite(path12_down.toStdString(), MIP12_down);

      QString path22_left = savePath+"\\MIP22_left.tif";
      imwrite(path22_left.toStdString(), MIP22_left);
      QString path22_up = savePath+"\\MIP22_up.tif";
      imwrite(path22_up.toStdString(), MIP22_up);

*/

       //while(1);




    cout<<"Successful!"<<endl;
    return a.exec();
}

void save_tiles_shift(vector<x_y_shift> XY_shift_vec, QString saveName)
{

    QAxObject *excel1 = new QAxObject();
    excel1 -> setControl("Excel.Application"); // connect EXCEL control
    excel1 -> setProperty("DisplayAlerts", true); // display window
    QAxObject *workbooks = excel1 -> querySubObject("WorkBooks"); // obtain excel set
    workbooks -> dynamicCall("Add"); // create a new excel
    QAxObject *workbook = excel1 -> querySubObject("ActiveWorkBook"); // obtain current excel
    workbook -> dynamicCall("SaveAs(const QString&, int, const QString&, const QString&, bool, bool)",
                            saveName, 51, QString(""), QString(""), false, false);
    //51xlsx, 56xls
    QAxObject *worksheets = workbook->querySubObject("WorkSheets"); // obtain sheets set
    QAxObject *worksheet1 = workbook -> querySubObject("WorkSheets(int)", 1); // the first sheet
    worksheet1 -> setProperty("Name", "sheet1");

    QList<QList<QVariant>> datas_sheet;
    for(int i = 0; i < XY_shift_vec.size()+1; i ++)
    {
        QList <QVariant> rows;
        if(i == 0)
        {
            rows.append("Z");
            rows.append("overlap");
            rows.append("shift");
        }

        else
        {
            rows.append(i);
            rows.append(XY_shift_vec[i-1].x);
            rows.append(XY_shift_vec[i-1].y);
        }
        datas_sheet.append(rows);
    }

    QList <QVariant> vars;
    for(auto v: datas_sheet)
        vars.append(QVariant(v));
    QVariant var = QVariant(vars);
    QString num1 = QString::fromStdString(to_string(XY_shift_vec.size()+1));
    QString big_range = "A1:C"+num1;
    QAxObject *excel_property = worksheet1 -> querySubObject("Range(const QString&)", big_range);
    excel_property -> setProperty("Value", var);
    excel_property -> setProperty("HorizontalAlignment", -4108);


    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close(Boolean)", false);
    excel1->dynamicCall("Quit(void)");
    delete excel1;
}

