#include "newimage_gui.h"
NewImage_gui::NewImage_gui(mImage &newI, QWidget *parent)
    :QDialog(parent)
{
    /*this dialog will get the input features of new image from user
     * and return a mImage struct
    */
    this->newimage=&newI;
    initialization();
    this->setLayout(gridlayout);
    this->setWindowTitle("MorphoHub: New Image Panel");
}
void NewImage_gui::initialization()
{
    iSampleID=new QLineEdit(QString("00000"));
    iSourceID=new QLineEdit(QString("00000"));
    iFormat=new QLineEdit(QString("mouse"));
    iObject=new QLineEdit(QString("terafly"));
    iSizeX=new QLineEdit(QString("").setNum(0));
    iSizeY=new QLineEdit(QString("").setNum(0));
    iSizeZ=new QLineEdit(QString("").setNum(0));
    iResolutionX=new QLineEdit(QString("").setNum(0));
    iResolutionY=new QLineEdit(QString("").setNum(0));
    iResolutionZ=new QLineEdit(QString("").setNum(0));
    iBit=new QLineEdit(QString("").setNum(12));
    iComments=new QLineEdit(QString(""));
    ok = new QPushButton(tr("ok"));
    cancel = new QPushButton(tr("cancel"));
    gridlayout=new QGridLayout();
    gridlayout->addWidget(new QLabel("Image sampleID : "),0,0,1,4);
    gridlayout->addWidget(iSampleID,0,4,1,2);
    gridlayout->addWidget(new QLabel("Image sourceID : "),1,0,1,4);
    gridlayout->addWidget(iSourceID,1,4,1,2);
    gridlayout->addWidget(new QLabel("Image format : "),2,0,1,4);
    gridlayout->addWidget(iFormat,2,4,1,2);
    gridlayout->addWidget(new QLabel("Image object : "),3,0,1,4);
    gridlayout->addWidget(iObject,3,4,1,2);
    gridlayout->addWidget(new QLabel("Image Size X : "),4,0,1,4);
    gridlayout->addWidget(iSizeX,4,4,1,2);
    gridlayout->addWidget(new QLabel("Image Size Y : "),5,0,1,4);
    gridlayout->addWidget(iSizeY,5,4,1,2);
    gridlayout->addWidget(new QLabel("Image Size Z : "),6,0,1,4);
    gridlayout->addWidget(iSizeZ,6,4,1,2);
    gridlayout->addWidget(new QLabel("Image Resolution X : "),7,0,1,4);
    gridlayout->addWidget(iResolutionX,7,4,1,2);
    gridlayout->addWidget(new QLabel("Image Resolution Y : "),8,0,1,4);
    gridlayout->addWidget(iResolutionY,8,4,1,2);
    gridlayout->addWidget(new QLabel("Image Resolution Z : "),9,0,1,4);
    gridlayout->addWidget(iResolutionZ,9,4,1,2);
    gridlayout->addWidget(new QLabel("Image bit size : "),10,0,1,4);
    gridlayout->addWidget(iBit,10,4,1,2);
    gridlayout->addWidget(new QLabel("Image comments : "),11,0,1,4);
    gridlayout->addWidget(iComments,11,4,1,2);
    gridlayout->addWidget(cancel, 12, 0, 1, 3, Qt::AlignRight);
    gridlayout->addWidget(ok, 12, 3, 1, 3, Qt::AlignRight);
    connect(ok,SIGNAL(clicked()),this,SLOT(okButton_slot()));
    connect(cancel,SIGNAL(clicked()),this,SLOT(reject()));
}
void NewImage_gui::okButton_slot()
{
    newimage->sampleID=iSampleID->text();
    newimage->sourceID=iSourceID->text();
    for(unsigned int i=0;i<newimage->mFormatList.size();i++)
        if(iFormat->text().compare(newimage->mFormatList.at(i)))
            newimage->mFormatID=i;
    for(unsigned int i=0;i<newimage->mObjectList.size();i++)
        if(iObject->text().compare(newimage->mObjectList.at(i)))
            newimage->mObjectID=i;
    newimage->mIS.setmIS(iSizeX->text().toLong(),iSizeY->text().toLong(),iSizeZ->text().toLong());
    newimage->mVR.setmVR(iResolutionX->text().toDouble(),
                        iResolutionY->text().toDouble(),
                        iResolutionZ->text().toDouble());
    newimage->bit=iBit->text().toUInt();
    newimage->comments=iComments->text();
    this->close();
}
NewImage_gui::NewImage_gui(){}
NewImage_gui::~NewImage_gui(){}
