#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/opencv.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/highgui/highgui.hpp"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
   // cv::Mat src =cv::imread("C:/Users/cloud/Desktop/[XDR]V$(JR@}K7H6]65N_R4.png");
   // cv::imshow("input",src);
  //  cv::waitKey(8);


}

MainWindow::~MainWindow()
{
    delete ui;
}

