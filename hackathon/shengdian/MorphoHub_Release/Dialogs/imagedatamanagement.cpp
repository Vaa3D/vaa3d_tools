#include "imagedatamanagement.h"
ImageDataManagement::ImageDataManagement(mFileSystem imfs, QWidget *parent)
    :QDialog(parent)
{
    this->mfs=imfs;
    IDMInit();
    createTabView();
    createActions();
    mainLayout=new QGridLayout;
    mainLayout->addWidget(tabwidget,0,0,8,6);
    mainLayout->addWidget(functionGroupBox,0,6,4,2);
    mainLayout->addWidget(logTextEdit,4,6,4,2);
    setLayout(mainLayout);
}
void ImageDataManagement::IDMInit()
{
    setWindowTitle(tr("MorphoHub-ImageData-Management"));
    logTextEdit=new QTextEdit(this);
    logTextEdit->setText("Welcome to image datasets management");
    if(mfs.dbPath.isEmpty())
    {
        QMessageBox::warning(this,tr("Path Error"),tr("please setup dbpath"));
        return;
    }

    this->imgconfpath=mfs.getImgConfPath();
    if(this->imgconfpath.isEmpty())
    {
        qDebug()<<"Can't get conf path from "<<imgconfpath<<endl;
        return;
    }
    mImage tmpImage;
    imgconfTitle=tmpImage.mFeatures;

    //load from conf file or start from new
    QFile conffile(this->imgconfpath);
    if(!conffile.exists())
    {
        //load from file
        QString title="please select image datasets configuration path";
        QString infile = QFileDialog::getOpenFileName(this, title,
                                                         mfs.dbPath,
                                                         QObject::tr("configuration file (*.conf *.csv)"));
        QFile tmpconffile(infile);
        if(tmpconffile.exists())
        {
            //read from the input conf file
            QList<mImage> mimage=getImagelist(infile);
            if(mimage.size())
            {
                idb.listImage=mimage;
                idb.mdbPath=mfs.imgDBPath;
                idb.mdbconf=mfs.getImgConfPath();
                this->imgconfpath=mfs.getImgConfPath();
                bool flag=writeImagelistToFile(idb.mdbconf,mimage);
                if(flag)
                    toLog(tr("update image conf file at %1").arg(idb.mdbconf));
                else
                    toLog(tr("failed to update conf file"));
            }
            else
                toLog(tr("File empty"));
        }
        else
        {
            //write init conf to file
            if(conffile.open(QIODevice::ReadWrite | QIODevice::Text))
            {
                QString data=imgconfTitle[0];
                for(int i=1;i<imgconfTitle.size();i++)
                {
                    data=data+","+imgconfTitle[i];
                }
                conffile.write(data.toAscii());
                conffile.close();
            }
        }
    }
    else
    {
        QList<mImage> mimage=getImagelist(imgconfpath);
        if(mimage.size())
        {
            idb.listImage=mimage;
            idb.mdbPath=mfs.imgDBPath;
            idb.mdbconf=mfs.getImgConfPath();
        }
        else
            toLog(tr("File empty"));
    }
}
void ImageDataManagement::createActions()
{
    functionGroupBox=new QGroupBox(tr("Operation Buttons"));
    functionLayout=new QVBoxLayout;
    saveButton=new QPushButton(tr("Save"));
    saveButton->setEnabled(false);
    connect(saveButton,SIGNAL(clicked()),this,SLOT(saveButton_slot()));
    newButton=new QPushButton(tr("New Image"));
    connect(newButton,SIGNAL(clicked()),this,SLOT(newButton_slot()));
    deleteButton=new QPushButton(tr("Delete Image"));
    connect(deleteButton,SIGNAL(clicked()),this,SLOT(deleteButton_slot()));

    newitemButton=new QPushButton(tr("New Feature"));
    newitemButton->setEnabled(false);
    deleteitemButton=new QPushButton(tr("Delete Feature"));
    deleteitemButton->setEnabled(false);
    functionLayout->addWidget(saveButton);
    functionLayout->addWidget(newButton);
    functionLayout->addWidget(deleteButton);
    functionLayout->addWidget(newitemButton);
    functionLayout->addWidget(deleteitemButton);
    functionGroupBox->setLayout(functionLayout);
}
void ImageDataManagement::createTabView()
{
    tabwidget=new QTabWidget();
    tabwidget->setTabsClosable(false);
    imageTab=createTableImage();
    tabwidget->addTab(imageTab,tr("Image Datasets"));
    //set close function
    connect(tabwidget,SIGNAL(tabCloseRequested(int)),this,SLOT(removeSubTab(int)));
    connect(tabwidget,SIGNAL(currentChanged(int)),this,SLOT(switchTab(int)));
}
void ImageDataManagement::removeSubTab(int subindex)
{
    tabwidget->removeTab(subindex);
}
void ImageDataManagement::switchTab(int toindex)
{
    if(toindex==0)
    {
        newButton->setEnabled(true);
        deleteButton->setEnabled(true);
    }
    else
    {
        saveButton->setEnabled(false);
        newButton->setEnabled(false);
        deleteButton->setEnabled(false);
    }
    tabwidget->setCurrentIndex(toindex);
}
QTableWidget* ImageDataManagement::createTableImage()
{
    //scan sourcedata.conf
    //get every cloum
    QStringList title=this->imgconfTitle;
    QTableWidget* itablewidget;
    int col = title.size();
    int  row=0;
    if(title.size()>0)
    {
        QList<mImage> templist=idb.listImage;
        row=templist.size();
        itablewidget= new QTableWidget(row,col, this);
        itablewidget->setHorizontalHeaderLabels(title);
        if(templist.size())
        {
            for(int i=0;i<templist.size();i++)
            {
                mImage tempi=templist.at(i);
//                //foreach item
                itablewidget->setItem(i,0,new QTableWidgetItem(QString::number(tempi.id)));
                itablewidget->setItem(i,1,new QTableWidgetItem(tempi.name));
                itablewidget->setItem(i,2,new QTableWidgetItem(tempi.objectType));
                itablewidget->setItem(i,3,new QTableWidgetItem(QString::number(tempi.bit)));
                itablewidget->setItem(i,4,new QTableWidgetItem(tempi.dirname));
                itablewidget->setItem(i,5,new QTableWidgetItem(tempi.mformat));
                itablewidget->setItem(i,6,new QTableWidgetItem(QString::number(tempi.mIS.sx)));
                itablewidget->setItem(i,7,new QTableWidgetItem(QString::number(tempi.mIS.sy)));
                itablewidget->setItem(i,8,new QTableWidgetItem(QString::number(tempi.mIS.sz)));
                itablewidget->setItem(i,9,new QTableWidgetItem(QString::number(tempi.mIR.rx)));
                itablewidget->setItem(i,10,new QTableWidgetItem(QString::number(tempi.mIR.ry)));
                itablewidget->setItem(i,11,new QTableWidgetItem(QString::number(tempi.mIR.rz)));
            }
        }
        itablewidget->setEditTriggers(QAbstractItemView::DoubleClicked);
        itablewidget->setSelectionBehavior(QAbstractItemView::SelectRows);
        itablewidget->setSelectionMode(QAbstractItemView::SingleSelection);
        itablewidget->resizeColumnsToContents();
        itablewidget->resizeRowsToContents();
//        connect(sdtablewidget,SIGNAL(cellEntered(int,int)),this,SLOT(createTab_slot(int,int)));
        connect(itablewidget,SIGNAL(cellChanged(int,int)),this,SLOT(reviseTab_slot(int,int)));
    }
    return itablewidget;
}
void ImageDataManagement::newButton_slot()
{
    //This button only open its function to source data management
    int tabindex=tabwidget->currentIndex();
    if(tabindex==0)
    {
        bool ok;
        QString newImageName=QInputDialog::getText(this,tr("New Image name"),tr("please input a new image name :"),QLineEdit::Normal,"12345",&ok);
        if(ok)
        {
            mImage tempi;
            tempi.name=newImageName;
            int i=imageTab->rowCount();
            imageTab->insertRow(i);
            //init
            imageTab->setItem(i,0,new QTableWidgetItem(QString::number(tempi.id)));
            imageTab->setItem(i,1,new QTableWidgetItem(tempi.name));
            imageTab->setItem(i,2,new QTableWidgetItem(tempi.objectType));
            imageTab->setItem(i,3,new QTableWidgetItem(QString::number(tempi.bit)));
            imageTab->setItem(i,4,new QTableWidgetItem(tempi.dirname));
            imageTab->setItem(i,5,new QTableWidgetItem(tempi.mformat));
            imageTab->setItem(i,6,new QTableWidgetItem(QString::number(tempi.mIS.sx)));
            imageTab->setItem(i,7,new QTableWidgetItem(QString::number(tempi.mIS.sy)));
            imageTab->setItem(i,8,new QTableWidgetItem(QString::number(tempi.mIS.sz)));
            imageTab->setItem(i,9,new QTableWidgetItem(QString::number(tempi.mIR.rx)));
            imageTab->setItem(i,10,new QTableWidgetItem(QString::number(tempi.mIR.ry)));
            imageTab->setItem(i,11,new QTableWidgetItem(QString::number(tempi.mIR.rz)));
            saveButton->setEnabled(true);
        }
    }
}
void ImageDataManagement::saveButton_slot()
{
    //save changes from image tab to conf file.
    int sdtrownum=imageTab->rowCount();
    QList<mImage> taballitems;
    for(int i=0;i<sdtrownum;i++)
    {
        mImage tempi;
        tempi.id=imageTab->item(i,0)->text().toUInt();
        tempi.name=imageTab->item(i,1)->text();
        tempi.objectType=imageTab->item(i,2)->text();
        tempi.bit=imageTab->item(i,3)->text().toUInt();
        tempi.dirname=imageTab->item(i,4)->text();
        tempi.mformat=imageTab->item(i,5)->text();
        tempi.mIS.sx=imageTab->item(i,6)->text().toULong();
        tempi.mIS.sy=imageTab->item(i,7)->text().toULong();
        tempi.mIS.sz=imageTab->item(i,8)->text().toULong();
        tempi.mIR.rx=imageTab->item(i,9)->text().toDouble();
        tempi.mIR.ry=imageTab->item(i,10)->text().toDouble();
        tempi.mIR.rz=imageTab->item(i,11)->text().toDouble();
        taballitems.append(tempi);
    }
    //2.write to conf. need a function.
    bool flag=writeImagelistToFile(idb.mdbconf,taballitems);
    if(flag)
    {
        saveButton->setEnabled(false);
        toLog(QString("Saved all the changes to %1").arg(idb.mdbconf));
    }
    else
        toLog(QString("Save Failed to %1").arg(idb.mdbconf));
}
void ImageDataManagement::deleteButton_slot()
{
    int rowIndex=imageTab->currentRow();
    int prerowcount=imageTab->rowCount();
    if(rowIndex!=-1)
    {
        toLog(QString("Deleted %1 info at configuration.").arg(imageTab->item(rowIndex,0)->text()));
        imageTab->removeRow(rowIndex);
        saveButton->setEnabled(true);
        if(prerowcount==1)
            deleteButton->setEnabled(false);
    }
}
void ImageDataManagement::reviseTab_slot(int row, int column)
{
    if(column>0)
    {
        saveButton->setEnabled(true);
    }
}
void ImageDataManagement::toLog(const QString &logtext)
{
    QString getlogtext=logTextEdit->toPlainText();
    QString showText=getlogtext+"\n"+logtext;
    logTextEdit->setText(showText);
    logTextEdit->moveCursor(QTextCursor::End);
}
ImageDataManagement::ImageDataManagement()
{
}
ImageDataManagement::~ImageDataManagement()
{

}
