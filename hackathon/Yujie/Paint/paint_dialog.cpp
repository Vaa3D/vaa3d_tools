#include "paint_dialog.h"
#include "scribblearea.h"


Paint_Dialog::Paint_Dialog(V3DPluginCallback2 *cb, QWidget *parent) :
    QDialog(parent)
{
    callback=cb;
    paintarea = new ScribbleArea();
    image1Dc_in=0;
    create();
    previousz=-1;
    //int dataflag=0; //if loaded data,dataflag=0; if fetched,dataflag=1
    bool zoominflag=false;
}

void Paint_Dialog::create()
{

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(paintarea,1,0,1,1);
    QToolBar *tool = new QToolBar;
    tool->setGeometry(0,0,250,20);
    //tool->setOrientation(Qt::Vertical );

    QVBoxLayout *layout = new QVBoxLayout;
    QToolButton *button_load = new QToolButton;
    button_load->setGeometry(0,0,10,20);
    button_load->setText("Load");
    QToolButton *button_save = new QToolButton;
    button_save->setText("Save");
    button_save->setGeometry(0,0,10,20);
    QToolButton *button_color = new QToolButton;
    button_color->setText("Color");
    QToolButton *button_fetch = new QToolButton;
    button_fetch->setText("Fetch");
    QToolButton *button_pb = new QToolButton;
    button_pb->setText("Pushback");
    button_pb->setGeometry(0,0,10,20);
    QToolButton *button_text=new QToolButton;
    button_text->setText("Insert text");
    button_text->setGeometry(0,0,10,20);
    QToolButton *button_pen = new QToolButton;
    button_pen->setText("Pen Width");
    QToolButton *button_print = new QToolButton;
    button_print->setText("Print");
    QToolButton *button_clear = new QToolButton;
    button_clear->setText("Clear screen");
    button_clear->setGeometry(0,0,10,20);
    QToolButton *button_zoomin=new QToolButton;
    button_zoomin->setText("Zoom in");
    button_zoomin->setGeometry(0,0,10,20);
    QToolButton *button_zoomout=new QToolButton;
    button_zoomout->setText("Zoom out");
    button_zoomout->setGeometry(0,0,10,20);

    QLabel *label= new QLabel;
    label->setText("Information of selecton:");
    edit=new QPlainTextEdit;
    edit->setPlainText("");
    spin=new QSpinBox;
    spin->setMaximum(0);
    spin->setMinimum(1);
    connect(spin,SIGNAL(valueChanged(int)),this,SLOT(zdisplay(int)));
    connect(button_pb,SIGNAL(clicked()),this,SLOT(pushback()));

    tool->addWidget(button_load);
    tool->addSeparator();
    tool->addWidget(button_fetch);
    tool->addSeparator();
    tool->addWidget(button_save);
    tool->addSeparator();
    tool->addWidget(button_pb);
    tool->addSeparator();
    tool->addWidget(button_zoomin);
    tool->addSeparator();
    tool->addWidget(button_zoomout);
    tool->addSeparator();
    tool->addWidget(button_text);
    tool->addSeparator();
    tool->addWidget(button_color);
    tool->addSeparator();
    tool->addWidget(button_pen);
    tool->addSeparator();
    tool->addWidget(button_clear);
    tool->addSeparator();
    tool->addWidget(button_print);
    tool->addSeparator();
    tool->addWidget(spin);

    layout->addWidget(tool);
    gridLayout->addWidget(label,2,0,1,1);
    gridLayout->addLayout(layout,0,0,1,1);
    gridLayout->addWidget(edit,3,0,1,1);

    this->setLayout(gridLayout);
    this->setMinimumHeight(700);
    this->setMinimumWidth(500);
    connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(button_save, SIGNAL(clicked()), this, SLOT(save()));
    connect(button_color, SIGNAL(clicked()), this, SLOT(penColor()));
    connect(button_pen, SIGNAL(clicked()), this, SLOT(penWidth()));
    connect(button_clear, SIGNAL(clicked()), this, SLOT(clearimage()));
    connect(button_print, SIGNAL(clicked()), paintarea, SLOT(print()));
    connect(button_fetch, SIGNAL(clicked()), this, SLOT(fetch()));
    connect(button_zoomin,SIGNAL(clicked()),this, SLOT(zoomin()));
    connect(button_zoomout,SIGNAL(clicked()),this,SLOT(zoomout()));
    connect(button_text,SIGNAL(clicked()),this,SLOT(inserttext()));
}


bool Paint_Dialog::maybeSave()
{
    if (paintarea->isModified()) {
       QMessageBox::StandardButton ret;
       ret = QMessageBox::warning(this, tr("Paint"),
                          tr("The image has been modified.\n"
                             "Do you want to save your changes?"),
                          QMessageBox::Save | QMessageBox::Discard
                          | QMessageBox::Cancel);
        if (ret == QMessageBox::Save) {
            return saveFile("jpg");
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
        qDebug()<<"MaybeSave";
    }
    return true;
}

//void Paint_Dialog::inserttext()
//{
//  QPainter *textpainter=new QPainter;
//  QRect *rect=new QRect(0,0,100,100);
//  QString text="Write text";
//  textpainter->drawImage(0,0,image,QRect);

//}

bool Paint_Dialog::load()
{
    qDebug()<<"In load now";
    if (maybeSave())
    {fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                                         QDir::currentPath(),
       QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    }
    if (!fileName.isEmpty())
    {

        if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
            v3d_msg("load image "+fileName+" error!");
            return false;
        }
        qDebug()<<"2";



        backupdata=datacopy(image1Dc_in,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        paint_1DC=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*3];
        memset(paint_1DC,0,sz_img[0]*sz_img[1]*sz_img[2]*3*sizeof(unsigned char));

        qDebug()<<"3";

        QSize newSize;
        newSize.setWidth(sz_img[0]);
        newSize.setHeight(sz_img[1]);
        paintarea->setFixedSize(newSize);
        qDebug()<<"4";
        spin->setMaximum(sz_img[2]-1);
        spin->setValue(sz_img[2]/2);
        //spin change value will trigger zdisplay
        return true;
    }
    return false;
}

void Paint_Dialog::zdisplay(int z)
{
    //test whether the image is in zoomin mode or not
    if (zoominflag)
    {
        qDebug()<<"I am zoomed in";
        qDebug()<<zoominflag;
        QImage q=paintarea->image.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        QImage p=paintarea->paintImage.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        paintarea->image=q;
        paintarea->paintImage=p;
    }
        qDebug()<<"I am not zoom in";
        if (previousz>=0)
        {
          savezimage(previousz);
        }

        QSize newSize;
        newSize.setWidth(sz_img[0]);
        newSize.setHeight(sz_img[1]);


        QImage newimage(newSize, QImage::Format_RGB16);
        QImage newimage2(newSize, QImage::Format_RGB16);
        QRgb value,value1;

        z=z-1;
        //int p3=0;
        //int p2=0;
        //Write image data to display on the screen
        for(int x=0; x< sz_img[0]; x++){
            for(int y=0; y<sz_img[1]; y++){

                int p1=backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];
                int p2=p1;
                int p3=p1;
                if (sz_img[3]>1){

                    p2=backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];
                }
                if (sz_img[3]>2)
                {
                    p3=backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];
                }
                int p4=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];

                int p5=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];

                int p6=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];

                value1=qRgb(p4,p5,p6);
                newimage2.setPixel(x,y,value1);

                if (p4!=0 || p5!=0 ||p6!=0)
                {

                    p1=p4;
                    p2=p5;
                    p3=p6;
                }
                value=qRgb(p1,p2,p3);
                newimage.setPixel(x,y,value);

            }
        }


        if (zoominflag)
        {

            paintarea->image=newimage;
            paintarea->paintImage=newimage2;
            zoomin();
        }

        else
        {
        paintarea->openImage(newimage,newimage2);
        }
        QString tmp="Image Size: \nx: " + QString::number(sz_img[0]) + " y: " + QString::number(sz_img[1]) +
            " z: " + QString::number(sz_img[2]) + "\nCurrent z: " + QString::number(spin->value());

        edit->setPlainText(tmp);

        previousz=spin->value();
  }





void Paint_Dialog::clearimage()
{
    if (pushback()){
        QMessageBox::information(0, "", "The data has been pushed back to main window.");
        return;
    }

    previousz=-1;

    int z=spin->value();

      for(int x=0; x< sz_img[0]; x++){
          for(int y=0; y<sz_img[1]; y++){
              image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];

              if (sz_img[3]>1)
              {
              image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]]=
                      backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];
              }
              if (sz_img[3]>2){
              image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]]=
                      backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];
              }
          }
      }
      zdisplay(z);

}


void Paint_Dialog::fetch()
{
    qDebug()<<"In fetch now";
    curwin = callback->currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback->getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }


    image1Dc_in = p4DImage->getRawData();
    ImagePixelType pixeltype = p4DImage->getDatatype();

    sz_img[0]=p4DImage->getXDim();
    sz_img[1]=p4DImage->getYDim();
    sz_img[2]=p4DImage->getZDim();
    sz_img[3]=p4DImage->getCDim();

    backupdata=datacopy(image1Dc_in,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
    paint_1DC=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*3];
    memset(paint_1DC,0,sz_img[0]*sz_img[1]*sz_img[2]*3*sizeof(unsigned char));
    qDebug()<<"datacopy and initialized";

    QSize newSize;
    newSize.setWidth(sz_img[0]);
    newSize.setHeight(sz_img[1]);
    paintarea->setFixedSize(newSize);

    TriviewControl *tript=callback->getTriviewControl(curwin);
    V3DLONG x,y,z;
    tript->getFocusLocation(x,y,z);
    qDebug()<<"fetch z value:"<<z;
    spin->setMaximum(sz_img[2]);
    spin->setValue(z);

}


unsigned char * Paint_Dialog::datacopy(unsigned char *data,long size)
{
    unsigned char * qcopydata=new unsigned char [size];
    for (int i=0;i<size;i++)
    {
        qcopydata[i]=data[i];
    }
    qDebug()<<"I have been copied";
    return qcopydata;
}


void Paint_Dialog::zoomin()
{
    zoominflag=true;
    QSize newSize;
    newSize.setWidth(600);
    newSize.setHeight(800);
    paintarea->setFixedSize(newSize);

    QImage q=paintarea->image.scaled(450,450,Qt::KeepAspectRatio);
    QImage p=paintarea->paintImage.scaled(450,450,Qt::KeepAspectRatio);
    paintarea->image=q;
    paintarea->paintImage=p;
    paintarea->setPenWidth(22);


    paintarea->openImage(q,p);
    qDebug()<<"Zoom in is working";

}

void Paint_Dialog::zoomout()
{
    QImage q=paintarea->image.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
    QImage p=paintarea->paintImage.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
    paintarea->image=q;
    paintarea->paintImage=p;
    paintarea->openImage(q,p);
    zoominflag=false;
    qDebug()<<"Zoomout....";
}

void Paint_Dialog::savezimage(int z)
{
    if (!paintarea->isModified())
    {
        qDebug()<<"I have not changed";
        return;
    }
    else
    {
        QColor color;
        z=z-1;

        for(int x=0; x< sz_img[0]; x++){
            for(int y=0; y<sz_img[1]; y++){
                color=paintarea->paintImage.pixel(QPoint(x,y));
                int red=color.red();
                int green=color.green();
                int blue=color.blue();
                paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=red;
                paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[2]*sz_img[0]*sz_img[1]]=green;
                paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[2]*sz_img[0]*sz_img[1]]=blue;
                }
             }
        }
     qDebug()<<"z image is saved: "<<z;

}

bool Paint_Dialog::pushback()
{   qDebug()<<"Inpushback now";

    curwin = callback->currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return false;
    }

    if (zoominflag)
    {
        QImage q=paintarea->image.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        QImage p=paintarea->paintImage.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        paintarea->image=q;
        paintarea->paintImage=p;
     }

      savezimage(spin->value());
    qDebug()<<"First stop";
    qDebug()<<"sz_img[3]"<<sz_img[3];
    if (sz_img[3]==1){

        image1D_out=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*3];
        memset(image1D_out,0,sz_img[0]*sz_img[1]*sz_img[2]*3*sizeof(unsigned char));
        memcpy(image1D_out, backupdata,sizeof(image1Dc_in));
//        image1Dc_in=0;
//        image1Dc_in=image1D_out;
        qDebug()<<"Before loop";
        for (int z=0;z<sz_img[2];z++) {
           for (int x=0;x<sz_img[0];x++) {
               for (int y=0;y<sz_img[1];y++) {
                   int p4=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];

                   int p5=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];

                   int p6=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];

                   if (p4!=0||p5!=0||p6!=0)
                   {
                       image1D_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=p4;

                       image1D_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]]=p5;

                       image1D_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]]=p6;

                   }

               }
           }
          }
        qDebug()<<"After loop";
        Image4DSimple image4D;
        qDebug()<<"Before setdata";
        image4D.setData(image1D_out,sz_img[0],sz_img[1],sz_img[2],3,(ImagePixelType)1);
        qDebug()<<"2";
        v3dhandle newwindow=callback->newImageWindow();
        qDebug()<<"3";
        callback->setImage(newwindow, &image4D);
        qDebug()<<"4";
        callback->setImageName(newwindow, "Paint result");
        callback->updateImageWindow(newwindow);


//        callback->setImage(curwin, &image4D);
//        callback->setImageName(curwin, "Paint result");
//        callback->updateImageWindow(curwin);


    }

    else{

         for (int z=0;z<sz_img[2];z++) {
            for (int x=0;x<sz_img[0];x++) {
                for (int y=0;y<sz_img[1];y++) {
                    int p4=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];

                    int p5=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];

                    int p6=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];

                    if (p4!=0||p5!=0||p6!=0)
                    {
                        image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=p4;

                        if (sz_img[3]>1) {

                        image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]]=p5;
                        }
                        if (sz_img[3]>2) {

                        image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]]=p6;
                        }
                    }

                }
            }
           }


    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData(image1Dc_in,sz_img[0],sz_img[1],sz_img[2],3,pixeltype);
    //v3dhandle newwindow=callback->newImageWindow();
    //callback->setImage(newwindow, new4DImage);
    callback->setImage(curwin, new4DImage);
    callback->setImageName(curwin, "Paint result");
    callback->updateImageWindow(curwin);
    qDebug()<<"After updatewindow";
}
    return true;

}

bool Paint_Dialog::saveFile(const QByteArray &fileFormat)
{
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                               initialPath,
                               tr("%1 Files (*.%2);;All Files (*)")
                               .arg(QString(fileFormat.toUpper()))
                               .arg(QString(fileFormat)));
    if (fileName.isEmpty()) {
        return false;
    } else {
        return paintarea->saveImage(fileName, fileFormat);
    }
    qDebug()<<"SaveFile";
}


void Paint_Dialog::save()
{

    saveFile("jpg");

}

void Paint_Dialog::penColor()
{
    QColor newColor = QColorDialog::getColor(paintarea->penColor());
    if (newColor.isValid())
        paintarea->setPenColor(newColor);
}

void Paint_Dialog::penWidth()
{
    bool ok;
    int newWidth = QInputDialog::getInteger(this, tr("Paint"),
                                            tr("Select pen width:"),
                                            paintarea->penWidth(),
                                            1, 50, 1, &ok);
    if (ok)
        paintarea->setPenWidth(newWidth);
}
