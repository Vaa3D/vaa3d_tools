#include "paint_dialog.h"
#include "scribblearea.h"


Paint_Dialog::Paint_Dialog(V3DPluginCallback2 *cb, QWidget *parent) :
    QDialog(parent)
{
    callback=cb;
    paintarea = new ScribbleArea();
    image1Dc_in=0;
    backupdata=0;
    create();
    previousz=-1;
    zoominflag=false;
}

void Paint_Dialog::create()
{

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(paintarea,1,0,1,1);
    QToolBar *tool = new QToolBar;
    tool->setGeometry(0,0,270,20);
    //tool->setOrientation(Qt::Vertical );

    QVBoxLayout *layout = new QVBoxLayout;
    QToolButton *button_load = new QToolButton;
    button_load->setGeometry(0,0,10,20);
    button_load->setText("Load");
    QToolButton *button_save = new QToolButton;
    button_save->setText("Save image");
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
    QToolButton *button_savefile=new QToolButton;
    button_savefile->setText("Save File");
    button_savefile->setGeometry(0,0,10,20);

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
    tool->addWidget(button_save);
    tool->addSeparator();
    tool->addWidget(button_savefile);
    tool->addSeparator();
    tool->addWidget(button_print);
    tool->addSeparator();
    tool->addWidget(spin);
    tool->addSeparator();

    layout->addWidget(tool);
    gridLayout->addWidget(label,2,0,1,1);
    gridLayout->addLayout(layout,0,0,1,1);
    gridLayout->addWidget(edit,3,0,1,1);

    this->setLayout(gridLayout);
    this->setMinimumHeight(700);
    this->setMinimumWidth(500);
    connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(button_save, SIGNAL(clicked()), this, SLOT(saveimage()));
    connect(button_color, SIGNAL(clicked()), this, SLOT(penColor()));
    connect(button_pen, SIGNAL(clicked()), this, SLOT(penWidth()));
    connect(button_clear, SIGNAL(clicked()), this, SLOT(clearimage()));
    connect(button_print, SIGNAL(clicked()), paintarea, SLOT(print()));
    connect(button_fetch, SIGNAL(clicked()), this, SLOT(fetch()));
    connect(button_zoomin,SIGNAL(clicked()),this, SLOT(zoomin()));
    connect(button_zoomout,SIGNAL(clicked()),this,SLOT(zoomout()));
    connect(button_text,SIGNAL(clicked()),this,SLOT(inserttext()));
    connect(button_savefile,SIGNAL(clicked()),this,SLOT(saveFile()));
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

            return saveFile();
        } else if (ret == QMessageBox::Cancel) {
            return false;
        }
        else if (ret== QMessageBox::Discard) {
            return true;
        }
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
    if (maybeSave()){
        fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                 QDir::currentPath(),QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    }
    if (!fileName.isEmpty())
    {
        resetdata();
        if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
            v3d_msg("load image "+fileName+" error!");
            return false;
        }



        V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        if(intype!=1)
        {
            if (intype == 2) //V3D_UINT16;
            {
                Paint_Dialog::convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
            }
            else if(intype == 4) //V3D_FLOAT32;
            {
                Paint_Dialog::convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
            }
            else
            {
                v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.", 0);
                return false;
            }
        }

        backupdata=datacopy(image1Dc_in,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        paint_1DC=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*3];
        memset(paint_1DC,0,sz_img[0]*sz_img[1]*sz_img[2]*3*sizeof(unsigned char));
        datasource=1;

        QSize newSize;
        newSize.setWidth(sz_img[0]);
        newSize.setHeight(sz_img[1]);
        paintarea->setFixedSize(newSize);
        spin->setMaximum(sz_img[2]-1);
        qDebug()<<"before spin set value";
        spin->setValue(sz_img[2]/2);  //spin change value will trigger zdisplay
        zdisplay(sz_img[2]/2);

        return true;
    }
    return false;
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

    resetdata();

    sz_img[0]=p4DImage->getXDim();
    sz_img[1]=p4DImage->getYDim();
    sz_img[2]=p4DImage->getZDim();
    sz_img[3]=p4DImage->getCDim();

    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

    //image1Dc_in=new unsigned char [size_tmp];
    image1Dc_in = p4DImage->getRawData();

    ImagePixelType pixeltype = p4DImage->getDatatype();

    if(pixeltype!=1)
    {
        if (pixeltype == 2) //V3D_UINT16;
        {
            Paint_Dialog::convert2UINT8((unsigned short*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else if(pixeltype == 4) //V3D_FLOAT32;
        {
            Paint_Dialog::convert2UINT8((float*)image1Dc_in, image1Dc_in, size_tmp);
        }
        else
        {
            v3d_msg("Currently this program only supports UINT8, UINT16, and FLOAT32 data type.", 0);
        }
    }

    backupdata=datacopy(image1Dc_in,size_tmp);
    paint_1DC=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*3];
    memset(paint_1DC,0,sz_img[0]*sz_img[1]*sz_img[2]*3*sizeof(unsigned char));
    datasource=2;

    QSize newSize;
    newSize.setWidth(sz_img[0]);
    newSize.setHeight(sz_img[1]);
    paintarea->setFixedSize(newSize);

    TriviewControl *tript=callback->getTriviewControl(curwin);
    V3DLONG x,y,z;
    tript->getFocusLocation(x,y,z);
    spin->setMaximum(sz_img[2]);
    spin->setValue(z);
    zdisplay(z);

}

void Paint_Dialog::resetdata()
{
    qDebug()<<"in resetdata";
    if(image1Dc_in != 0){
       if(datasource==1) {
        delete []image1Dc_in; image1Dc_in=0;
        }
       else if(datasource==2){
        image1Dc_in=0;
       }
    }
    sz_img[0]=sz_img[1]=sz_img[2]=sz_img[3]=0;
    zoominflag=false;
    previousz=-1;
    intype=0;
    if(paint_1DC!=0) {
        delete []paint_1DC; paint_1DC=0;
    }
    if(backupdata!=0) {
        delete []backupdata; backupdata=0;
    }
}

void Paint_Dialog::savezimage(int z)
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


void Paint_Dialog::zdisplay(int z_in)
{
    int z=z_in-1;

    //if zoomin, zoomout first and then store the image
    if (zoominflag)
    {
        //qDebug()<<"I am zoomed in";
        QImage q=paintarea->image.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        QImage p=paintarea->paintImage.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        paintarea->image=q;
        paintarea->paintImage=p;
    }
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

    //Write image data to display on the screen
    for(int x=0; x< sz_img[0]; x++){
        for(int y=0; y<sz_img[1]; y++){
            int p1=backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];
            int p2=0;
            int p3=0;
            if (sz_img[3]>1){
                p2=backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];
            }else{
                p2=p1;
            }
            if (sz_img[3]>2){
                p3=backupdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];
            }else{
                p3=p1;
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
    //qDebug()<<"In zdisplay";
    //if in zoomin mode, needs to zoom the pic in again.
    if (zoominflag){
        paintarea->image=newimage;
        paintarea->paintImage=newimage2;
        zoomin();
    }
    else {
    paintarea->openImage(newimage,newimage2);
    }
    QString tmp="Image Size: \nx: " + QString::number(sz_img[0]) + " y: " + QString::number(sz_img[1]) +
        " z: " + QString::number(sz_img[2]) + "\nCurrent z: " + QString::number(spin->value());
    edit->setPlainText(tmp);
    previousz=spin->value();

}



void Paint_Dialog::clearimage()
{
    previousz=-1; //skip the savezimage part

    //in case you flip pages and then want to clear image
    int z=spin->value()-1;
    memset(paint_1DC+z*sz_img[0]*sz_img[1],0,sz_img[0]*sz_img[1]*sizeof(unsigned char));
    memset(paint_1DC+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2],0,sz_img[0]*sz_img[1]*sizeof(unsigned char));
    memset(paint_1DC+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2],0,sz_img[0]*sz_img[1]*sizeof(unsigned char));

    zdisplay(z+1);
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
    if (datasource!=1 && datasource!=2){
        v3d_msg("No image available to zoom in", 0);
        return;
    }
    zoominflag=true;
    QSize newSize;
    newSize.setWidth(600);
    newSize.setHeight(800);
    paintarea->setFixedSize(newSize);

    QImage q=paintarea->image.scaled(sz_img[0]*2,sz_img[1]*2,Qt::KeepAspectRatio);
    QImage p=paintarea->paintImage.scaled(sz_img[0]*2,sz_img[1]*2,Qt::KeepAspectRatio);
    paintarea->image=q;
    paintarea->paintImage=p;
    paintarea->setPenWidth(22);
    paintarea->openImage(q,p);
    //qDebug()<<"Zoom in is working";

}

void Paint_Dialog::zoomout()
{
    QImage q=paintarea->image.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
    QImage p=paintarea->paintImage.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
    paintarea->image=q;
    paintarea->paintImage=p;
    paintarea->openImage(q,p);
    zoominflag=false;
    paintarea->setPenWidth(15);
    qDebug()<<"Zoomout....";
}

void Paint_Dialog::pushback()
{
    qDebug()<<"Inpushback now";

    curwin = callback->currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    if (datasource==1)
    {
        QMessageBox::information(0, "", "Cannot push back. Please load the image in Vaa3D main");
        return;
    }
    if (zoominflag)
    {
        QImage q=paintarea->image.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        QImage p=paintarea->paintImage.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        paintarea->image=q;
        paintarea->paintImage=p;
     }

    savezimage(spin->value());

    unsigned char * image1Dc_out=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*3];
    memset(image1Dc_out,0,sz_img[0]*sz_img[1]*sz_img[2]*3*sizeof(unsigned char));
    memcpy(image1Dc_out,backupdata,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));

    if(sz_img[3]==1){
        memcpy(image1Dc_out+sz_img[0]*sz_img[1]*sz_img[2],backupdata,sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
        memcpy(image1Dc_out+2*sz_img[0]*sz_img[1]*sz_img[2],backupdata,sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
    }

    for (int z=0;z<sz_img[2];z++) {
        for (int x=0;x<sz_img[0];x++) {
           for (int y=0;y<sz_img[1];y++) {
               int p4=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];
               int p5=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];
               int p6=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];

               if (p4!=0||p5!=0||p6!=0)
               {
                   image1Dc_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=p4;
                   image1Dc_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]]=p5;
                   image1Dc_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]]=p6;
               }
           }
        }
    }
    //Push the new image back to current window
    Image4DSimple image4D;
    image4D.setData(image1Dc_out,sz_img[0],sz_img[1],sz_img[2],3,V3D_UINT8);
    callback->setImage(curwin, &image4D);
    callback->setImageName(curwin, "Paint result");
    callback->updateImageWindow(curwin);
    delete []image1Dc_out;
    image1Dc_out=0;
}

bool Paint_Dialog::saveFile()//const QByteArray &fileFormat)
{
    QString initialPath = QDir::currentPath() + "/untitled" + ".v3draw";

    QString fileout = QFileDialog::getSaveFileName(this, tr("Save As"),initialPath,
                      QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

    //Get the combined file of raw data and paint file
    unsigned char * image1Dc_out=new unsigned char [sz_img[0]*sz_img[1]*sz_img[2]*3];
    memset(image1Dc_out,0,sz_img[0]*sz_img[1]*sz_img[2]*3*sizeof(unsigned char));
    memcpy(image1Dc_out,backupdata,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]*sizeof(unsigned char));

    if(sz_img[3]==1){
        memcpy(image1Dc_out+sz_img[0]*sz_img[1]*sz_img[2],backupdata,sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
        memcpy(image1Dc_out+2*sz_img[0]*sz_img[1]*sz_img[2],backupdata,sz_img[0]*sz_img[1]*sz_img[2]*sizeof(unsigned char));
    }

    for (int z=0;z<sz_img[2];z++) {
        for (int x=0;x<sz_img[0];x++) {
           for (int y=0;y<sz_img[1];y++) {
               int p4=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];
               int p5=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]];
               int p6=paint_1DC[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]];

               if (p4!=0||p5!=0||p6!=0)
               {
                   image1Dc_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=p4;
                   image1Dc_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[0]*sz_img[1]*sz_img[2]]=p5;
                   image1Dc_out[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[0]*sz_img[1]*sz_img[2]]=p6;
               }
           }
        }
    }

    if(!simple_saveimage_wrapper(*callback, qPrintable(fileout),image1Dc_out,sz_img,1)){
        QMessageBox::information(0, "", "File not saved");
        return false;
    }else{
        paintarea->modified=false;
        return true;
    }

}


bool Paint_Dialog::saveimage()
{
    const QByteArray &fileFormat="jpg";
    QString initialPath = QDir::currentPath() + "/untitled." + fileFormat;

    QString fileName = QFileDialog::getSaveFileName(this, tr("Save As"),
                           initialPath,
                           tr("%1 Files (*.%2);;All Files (*)")
                           .arg(QString(fileFormat.toUpper()))
                           .arg(QString(fileFormat)));
    if (fileName.isEmpty()) {
    return false;
    } else {
    paintarea->modified=false;
    return paintarea->saveImage(fileName, fileFormat);
    }

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

void Paint_Dialog::convert2UINT8(unsigned short *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    unsigned short* pPre = (unsigned short*)pre1d;
    unsigned short max_v=0, min_v = 255;
    for(V3DLONG i=0; i<imsz; i++)
    {
        if(max_v<pPre[i]) max_v = pPre[i];
        if(min_v>pPre[i]) min_v = pPre[i];
    }
    max_v -= min_v;
    if(max_v>0)
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
        }
    }
    else
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) pPre[i];
        }
    }
}

void Paint_Dialog::convert2UINT8(float *pre1d, unsigned char *pPost, V3DLONG imsz)
{
    float* pPre = (float*)pre1d;
    float max_v=0, min_v = 65535;
    for(V3DLONG i=0; i<imsz; i++)
    {
        if(max_v<pPre[i]) max_v = pPre[i];
        if(min_v>pPre[i]) min_v = pPre[i];
    }
    max_v -= min_v;
    if(max_v>0)
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) 255*(double)(pPre[i] - min_v)/max_v;
        }
    }
    else
    {
        for(V3DLONG i=0; i<imsz; i++)
        {
            pPost[i] = (unsigned char) pPre[i];
        }
    }
}

void Paint_Dialog::closeEvent(QCloseEvent *event)
 {
     if (maybeSave()) {
         event->accept();
     } else {
         event->ignore();
     }
 }
