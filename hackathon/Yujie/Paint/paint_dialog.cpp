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
}

void Paint_Dialog::create()
{

    QGridLayout *gridLayout = new QGridLayout();
    gridLayout->addWidget(paintarea,1,0,1,1);
    QToolBar *tool = new QToolBar;
    tool->setGeometry(0,0,200,20);
    QVBoxLayout *layout = new QVBoxLayout;
    QToolButton *button_load = new QToolButton;
    button_load->setGeometry(0,0,10,20);
    button_load->setText("Load");
    QToolButton *button_save = new QToolButton;
    button_save->setText("Save");
    QToolButton *button_color = new QToolButton;
    button_color->setText("Color");
    QToolButton *button_fetch = new QToolButton;
    button_fetch->setText("Fetch");
    QToolButton *button_pb = new QToolButton;
    button_pb->setText("Pushback");
    QToolButton *button_pen = new QToolButton;
    button_pen->setText("Pen Width");
    QToolButton *button_print = new QToolButton;
    button_print->setText("Print");
    QToolButton *button_clear = new QToolButton;
    button_clear->setText("Clear screen");
    QLabel *label= new QLabel;
    label->setText("Information of selecton:");
    edit=new QPlainTextEdit;
    edit->setPlainText("");
    spin=new QSpinBox;
    spin->setMaximum(0);
    spin->setMinimum(0);
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
    tool->addWidget(button_color);
    tool->addSeparator();
    tool->addWidget(button_pen);
    tool->addSeparator();
    tool->addWidget(button_clear);
    tool->addSeparator();
    tool->addWidget(button_print);
    tool->addSeparator();

    layout->addWidget(tool);
    gridLayout->addWidget(label,2,0,1,1);
    gridLayout->addLayout(layout,0,0,1,1);
    gridLayout->addWidget(edit,3,0,1,1);
    gridLayout->addWidget(spin,0,1,1,1);
    this->setLayout(gridLayout);

    this->setMinimumHeight(800);
    this->setMinimumWidth(800);
    connect(button_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(button_save, SIGNAL(clicked()), this, SLOT(save()));
    connect(button_color, SIGNAL(clicked()), this, SLOT(penColor()));
    connect(button_pen, SIGNAL(clicked()), this, SLOT(penWidth()));
    connect(button_clear, SIGNAL(clicked()), this, SLOT(clearimage()));
    connect(button_print, SIGNAL(clicked()), paintarea, SLOT(print()));
    connect(button_fetch, SIGNAL(clicked()), this, SLOT(fetch()));

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


bool Paint_Dialog::load()
{
    if (maybeSave())
    {fileName = QFileDialog::getOpenFileName(0, QObject::tr("Choose the input image "),
                                         QDir::currentPath(),
       QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
    }
    if (!fileName.isEmpty())
    {
        qDebug()<<"1";

        if (!simple_loadimage_wrapper(*callback, fileName.toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
            v3d_msg("load image "+fileName+" error!");
            return false;
        }
        qDebug()<<"2";

        QSize newSize;
        newSize.setWidth(sz_img[0]);
        newSize.setHeight(sz_img[1]);
        paintarea->setFixedSize(newSize);

        datacopy(image1Dc_in,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);

        spin->setMaximum(sz_img[2]-1);
        spin->setValue(sz_img[2]/2);
        //spin change value will trigger zdisplay
        return true;
    }
    return false;
}

void Paint_Dialog::zdisplay(int z)
{
    if (previousz>=0)
    {
      savezimage(previousz);
    }

    QSize newSize;
    newSize.setWidth(sz_img[0]);
    newSize.setHeight(sz_img[1]);
    QImage newimage(newSize, QImage::Format_RGB16);
    QRgb value;

    z=z-2;
    if (z<0)  z=0; //bug in the v3d main

    //Write image data to display on the screen
    for(int x=0; x< sz_img[0]; x++){
        for(int y=0; y<sz_img[1]; y++){
            int p1=image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]];
            int p2=image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[2]*sz_img[0]*sz_img[1]];
            int p3=image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[2]*sz_img[0]*sz_img[1]];
            value=qRgb(p1,p2,p3);
            newimage.setPixel(x,y,value);
        }
    }

    paintarea->openImage(newimage);

    QString tmp="Image Size: \nx: " + QString::number(sz_img[0]) + " y: " + QString::number(sz_img[1]) +
            " z: " + QString::number(sz_img[2]) + "\nCurrent z: " + QString::number(spin->value());

    edit->setPlainText(tmp);

    previousz=spin->value();
}


void Paint_Dialog::clearimage()
{
    previousz=-1;
    zdisplay(spin->value());
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


    unsigned char* data1d = p4DImage->getRawData();
    image1Dc_in=data1d;

    ImagePixelType pixeltype = p4DImage->getDatatype();

    V3DLONG N = p4DImage->getXDim();
    sz_img[0]=N;
    V3DLONG M = p4DImage->getYDim();
    sz_img[1]=M;
    V3DLONG P = p4DImage->getZDim();
    sz_img[2]=P;
    V3DLONG sc = p4DImage->getCDim();
    sz_img[3]=sc;

    qDebug()<<"N:"<<N <<"M: "<<M <<"P: "<<P;
    //save image1Dc_in data in qcopydata
    datacopy(data1d,N*M*P*sc);

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


void Paint_Dialog::datacopy(unsigned char *data,long size)
{
    qcopydata=new unsigned char [size];
    for (int i=0;i<size;i++)
    {qcopydata[i]=data[i];
    }
    qDebug()<<"I have been copied";

}

void Paint_Dialog::savezimage(int z)
{
    if (!paintarea->isModified())
    {qDebug()<<"I have not changed";
        return;
    }
    else
    {
        QColor color;
        qDebug()<<"z image is saved: "<<z;
        z=z-2;
        if (z<0) z=0;
        for(int x=0; x< sz_img[0]; x++){
            for(int y=0; y<sz_img[1]; y++){
                color=paintarea->image.pixel(QPoint(x,y));
                int red=color.red();
                int blue=color.blue();
                int green=color.green();
                image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=red;
                image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[2]*sz_img[0]*sz_img[1]]=green;
                image1Dc_in[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[2]*sz_img[0]*sz_img[1]]=blue;
             }
        }

    }
}

void Paint_Dialog::pushback()
{   qDebug()<<"Inpushback now";
    unsigned char* newdata=0;
    long size=sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
    //newdata=(unsigned char *) calloc(size, sizeof(unsigned char));
    newdata=image1Dc_in;

    QColor color;
    int z=spin->value();
    qDebug()<<"Pushback spin value:"<<z;
    z=z-2;
    if (z<0) z=0;
    for(int x=0; x< sz_img[0]; x++){
        for(int y=0; y<sz_img[1]; y++){
            color=paintarea->image.pixel(QPoint(x,y));
            int red=color.red();
            int blue=color.blue();
            int green=color.green();
            newdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]]=red; //red;
            newdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+sz_img[2]*sz_img[0]*sz_img[1]]=green; //blue;
            newdata[x+sz_img[0]*y+z*sz_img[0]*sz_img[1]+2*sz_img[2]*sz_img[0]*sz_img[1]]=blue; //green;

        }
    }

    qDebug()<<"after loop";
    Image4DSimple * new4DImage = new Image4DSimple();
    new4DImage->setData(newdata,sz_img[0],sz_img[1],sz_img[2], sz_img[3], pixeltype);
    //v3dhandle newwin = callback->newImageWindow();
    callback->setImage(curwin, new4DImage);
    callback->setImageName(curwin, "Paint result");
    callback->updateImageWindow(curwin);
    qDebug()<<"After updatewindow";

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
