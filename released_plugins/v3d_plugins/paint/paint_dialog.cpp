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
    paint_1DC=0;

}

void Paint_Dialog::create()
{
    QBoxLayout *boxlayout=new QBoxLayout(QBoxLayout::TopToBottom);
    tool = new QToolBar;
    tool->setGeometry(0,0,300,20);

    QVBoxLayout *layout = new QVBoxLayout;
    QToolButton *button_open = new QToolButton;
    button_open->setGeometry(0,0,10,30);
    button_open->setText("Open");
    QToolButton *button_save = new QToolButton;
    button_save->setText("Save");
    button_save->setGeometry(0,0,10,30);
    QToolButton *button_color = new QToolButton;
    button_color->setText(" Color");
    QToolButton *button_help = new QToolButton;
    button_help->setText("Help");
    QToolButton *button_pb = new QToolButton;
    button_pb->setText("Pushback");
    button_pb->setGeometry(0,0,10,30);
    QToolButton *button_pen = new QToolButton;
    button_pen->setText("Pen Width");
    QToolButton *button_print = new QToolButton;
    button_print->setText("Print");
    QToolButton *button_clear = new QToolButton;
    button_clear->setText("Clear painting");
    button_clear->setGeometry(0,0,10,30);
    QToolButton *button_zoomin=new QToolButton;
    button_zoomin->setText("Zoom in");
    button_zoomin->setGeometry(0,0,10,30);
    QToolButton *button_zoomout=new QToolButton;
    button_zoomout->setText("Zoom out");
    button_zoomout->setGeometry(0,0,10,30);
    button_color->setIcon(QIcon(":/paint.png"));
    QSize iconsize(28,28);
    button_color->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    savemenu=new QMenu;
    createsavemenu();
    button_save->setMenu(savemenu);
    button_save->setPopupMode(QToolButton::InstantPopup);
    connect(button_save,SIGNAL(clicked()),this,SLOT(dosavemenu()));

    openmenu=new QMenu;
    createopenmenu();
    button_open->setMenu(openmenu);
    button_open->setPopupMode(QToolButton::InstantPopup);
    connect(button_open,SIGNAL(clicked()),this,SLOT(doopenmenu()));

    QLabel *label= new QLabel;
    label->setText("Information of selecton:");
    edit=new QPlainTextEdit;
    edit->setPlainText("");
    edit->setMaximumHeight(100);
    edit->setReadOnly(true);
    spin=new QSpinBox;
    spin->setMaximum(1);
    spin->setMinimum(1);
    paintarea->installEventFilter(this);
    connect(spin,SIGNAL(valueChanged(int)),this,SLOT(zdisplay(int)));
    connect(button_pb,SIGNAL(clicked()),this,SLOT(pushback()));

    tool->addWidget(button_open);
    tool->addSeparator();
    tool->addWidget(button_save);
    tool->addSeparator();
    tool->addWidget(button_pb);
    tool->addSeparator();
    tool->addWidget(button_zoomin);
    tool->addSeparator();
    tool->addWidget(button_zoomout);
    tool->addSeparator();
    tool->addWidget(button_clear);
    tool->addSeparator();
    tool->addWidget(button_color);
    tool->addSeparator();
    tool->addWidget(button_pen);
    tool->addSeparator();
    tool->addWidget(button_print);
    tool->addSeparator();
    tool->addWidget(button_help);
    tool->addSeparator();
    tool->addWidget(spin);
    tool->addSeparator();
    tool->setIconSize(iconsize);
    layout->addWidget(tool);

    boxlayout->addLayout(layout);
    boxlayout->addWidget(paintarea);
    boxlayout->addWidget(label);
    boxlayout->addWidget(edit);

    this->setLayout(boxlayout);
    this->setMinimumHeight(200);
    this->setMinimumWidth(500);
    connect(button_color, SIGNAL(clicked()), this, SLOT(penColor()));
    connect(button_pen, SIGNAL(clicked()), this, SLOT(penWidth()));
    connect(button_clear, SIGNAL(clicked()), this, SLOT(clearimage()));
    connect(button_print, SIGNAL(clicked()), paintarea, SLOT(print()));
    connect(button_help, SIGNAL(clicked()), this, SLOT(help()));
    connect(button_zoomin,SIGNAL(clicked()),this, SLOT(zoomin()));
    connect(button_zoomout,SIGNAL(clicked()),this,SLOT(zoomout()));

}


void Paint_Dialog::createsavemenu()
{
    QAction* Act;
    Act = new QAction(tr("Save entire 3D-stack"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(saveFile()));
    savemenu->addAction(Act);

    Act = new QAction(tr("Save only current section"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(saveimage()));
    savemenu->addAction(Act);
}

void Paint_Dialog::dosavemenu()
{
   savemenu->exec(QCursor::pos());
}


void Paint_Dialog::createopenmenu()
{
    QAction* Act;

    Act = new QAction(tr("Load image files"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(load()));
    openmenu->addAction(Act);

    Act = new QAction(tr("Fetch from 3D viewer"), this);
    connect(Act, SIGNAL(triggered()), this, SLOT(fetch()));
    openmenu->addAction(Act);
}

void Paint_Dialog::doopenmenu()
{
   openmenu->exec(QCursor::pos());
}


bool Paint_Dialog::maybeSave()
{
    if (paintarea->isModified())
    {
       QMessageBox mybox;
       mybox.setText("The document has been modified.");
       mybox.setInformativeText("Do you want to save your changes?");
       QPushButton *push_button=mybox.addButton(tr("Push back"),QMessageBox::ActionRole);
       QPushButton *save_button = mybox.addButton(QMessageBox::Save);
       QPushButton *cancel_button=mybox.addButton(QMessageBox::Cancel);
       QPushButton *discard_button=mybox.addButton(QMessageBox::Discard);
       mybox.setDefaultButton(QMessageBox::Save);
       mybox.exec();

        if (mybox.clickedButton() == save_button) {
            return saveFile();
        } else if (mybox.clickedButton() == cancel_button) {
            return false;
        }
        else if (mybox.clickedButton()== discard_button) {
            return true;
        }
        else if (mybox.clickedButton()==push_button) {
            return pushback();
        }
    }
    return true;
}


bool Paint_Dialog::load()
{
    if (maybeSave()){
        fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                 QDir::currentPath(),QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

        if (!fileName.isEmpty())
        {
            resetdata();
            if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), image1Dc_in, sz_img, intype))
            {
                QMessageBox::information(0,"","load image "+fileName+" error!");
                return false;
            }

            if (sz_img[3]>3)
            {
                QMessageBox::information(0,"","Currently this program only supports 1-3 color channels.", 0);
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
                    QMessageBox::information(0,"","Currently this program only supports UINT8, UINT16, and FLOAT32 data type.");
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
            spin->setMaximum(sz_img[2]);
            //qDebug()<<"before spin set value";
            spin->setValue(sz_img[2]/2);  //spin change value will trigger zdisplay
            zdisplay(sz_img[2]/2);
            this->setFixedHeight(paintarea->height()+edit->height()+spin->height()+tool->height()+50);
            return true;
        }
    return false;
    }
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

    if (sz_img[3]>3)
    {
        QMessageBox::information(0, "", "Currently this program only supports 1-3 color channels.");
        return;
    }

    V3DLONG size_tmp=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];

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
            QMessageBox::information(0, "", "Currently this program only supports 1-3 color channels.");
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

    this->setFixedHeight(paintarea->height()+edit->height()+spin->height()+tool->height()+50);
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
    //if in zoomin mode, needs to zoom in the image again.
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
    if (datasource!=1 && datasource!=2){
        QMessageBox::information(0, "", "No image is loaded/fetched");
        return;
    }
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
    return qcopydata;
}


void Paint_Dialog::zoomin()
{
    if (datasource!=1 && datasource!=2){
       QMessageBox::information(0, "", "No image available to zoom in.");
        return;
    }
    zoominflag=true;
    QSize newSize;
    newSize.setWidth(sz_img[0]*2);
    newSize.setHeight(sz_img[1]*2);
    paintarea->setFixedSize(newSize);

    QImage q=paintarea->image.scaled(sz_img[0]*2,sz_img[1]*2,Qt::KeepAspectRatio);
    QImage p=paintarea->paintImage.scaled(sz_img[0]*2,sz_img[1]*2,Qt::KeepAspectRatio);

    paintarea->image=q;
    paintarea->paintImage=p;
    paintarea->setPenWidth(22);
    paintarea->openImage(q,p);

    this->setFixedHeight(paintarea->height()+edit->height()+spin->height()+tool->height()+50);
}

void Paint_Dialog::zoomout()
{
    if (zoominflag==true){
        QImage q=paintarea->image.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);
        QImage p=paintarea->paintImage.scaled(sz_img[0],sz_img[1],Qt::KeepAspectRatio);

        paintarea->image=q;
        paintarea->paintImage=p;
        paintarea->setFixedSize(sz_img[0],sz_img[1]);

        paintarea->openImage(q,p);
        zoominflag=false;
        paintarea->setPenWidth(15);

        this->setFixedHeight(paintarea->height()+edit->height()+spin->height()+tool->height()+50);
    }
}

bool Paint_Dialog::pushback()
{
    curwin = callback->currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return false;
    }
    if (datasource!=1&datasource!=2) {
        QMessageBox::information(0, "", "Cannot push back. No image is fetched");
        return false;
    }

    if (datasource==1)
    {
        QMessageBox::information(0, "", "Cannot push back. Please load the image in Vaa3D main");
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

    //Set the paintarea back to the zoom in mode
    if (zoominflag)
    {
        QImage m=paintarea->image.scaled(sz_img[0]*2,sz_img[1]*2,Qt::KeepAspectRatio);
        QImage n=paintarea->paintImage.scaled(sz_img[0]*2,sz_img[1]*2,Qt::KeepAspectRatio);
        paintarea->image=m;
        paintarea->paintImage=n;
     }
    paintarea->modified=false;
    return true;
}

bool Paint_Dialog::saveFile()//const QByteArray &fileFormat)
{
    QString initialPath = QDir::currentPath() + "/untitled" + ".v3draw";
    QFileDialog qdilog(0,tr("Save As"),initialPath,
                QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));

    qdilog.setAcceptMode(QFileDialog::AcceptSave);
    QString fileout;

    if(qdilog.exec()){
        fileout=qdilog.selectedFiles().at(0);
    }
    else {
        return false;
    }

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


bool Paint_Dialog::eventFilter(QObject *obj, QEvent *event)
{
        if (obj == paintarea && event->type() == QEvent::Wheel)
        {
            QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
            if(wheelEvent->delta() > 0) {
                this->spin->setValue(this->spin->value() + 1);

            }
            else
                this->spin->setValue(this->spin->value() - 1);

            return true;

        }
        return false;
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
                  tr("Select pen width:"),paintarea->penWidth(),1, 50, 1, &ok);
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

void Paint_Dialog::help()
{
    QMessageBox::about(this, tr("How to use Paint plug-in"),
    tr("<p>The <b>Paint</b> plug-in is designed to help users do simple paintings on 2D images.<br>"
               "<b>Load/Fetch</b> -- Users can choose to load the image from local image files or fetch from current"
               " Vaa3D main window by using 'load' or 'Fetch' button in the pop-up list of 'open' button.<br>"
               "<b>Save entire 3D stack</b> -- The entire 3D stack file including the user-drawn paintings on all slices"
               " will be saved.<br>"
               "<b>Save only current section</b> -- The current 2D image will be saved in jpg format."
               "Changes made on other slices will not be stored.<br>"
               "<b>Pushback</b> -- Pushback function is enabled only if the image is fetched from current Vaa3D main"
               "window. Once the pushback button clicked, the image together with the painting will be sent back to the current "
               "Vaa3D main window.<br>"
               "<b>Zoom in/out</b>-- Image is scaled to double the original size/back to the original size.<br>"
               "<b>Clear painting</b> -- Clear all the drawings made by users on the current 2D image.<br>"
               "<b>Color</b> -- Change the pen color to user-specified color.<br>"
               "<b>Pen width</b> --Change the pen width to user-specified width.<br>"
               "<b>Print</b> -- Users can print out the current 2D image.<br>"
               "<b>Spin boxes</b>-- Scroll up and down to visualize different slices. Number reflects current"
               "  slice number.</p>"
               "<p>For further questions, please contact Yujie Li at yujie.jade@gmail.com)</p>"));
}
