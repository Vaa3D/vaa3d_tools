#include "extractCenter_gui.h"

ExtractMeanCenterDialog :: ExtractMeanCenterDialog(QWidget *parent): QDialog(parent)
{
    rangeBox = new QSpinBox();
    rangeBox -> setRange(2, 100);
    rangeBox -> setValue(4);
    rangeBox -> setSingleStep(2);
    connect(rangeBox, SIGNAL(valueChanged(int)), rangeBox, SLOT(setValue(int)));

    openButton = new QPushButton(tr("Input a folder"));
    openEdit = new QLineEdit(tr("Select a folder !"));
    openHbox = new QHBoxLayout();
    openHbox -> addWidget(openButton);
    openHbox -> addWidget(openEdit);
    connect(openButton, SIGNAL(clicked(bool)), this, SLOT(selectFolder()));

    saveButton = new QPushButton(tr("Save files"));
    saveEdit = new QLineEdit(tr("Select a folder to save files !"));
    saveHbox = new QHBoxLayout();
    saveHbox -> addWidget(saveButton);
    saveHbox -> addWidget(saveEdit);
    connect(saveButton, SIGNAL(clicked(bool)), this, SLOT(saveFiles()));

    ok = new QPushButton(tr("ok"));
    ok -> setDefault(true);
    cancel = new QPushButton(tr("cancel"));
    cancel -> setDefault(false);
    hbox_ok = new QHBoxLayout();
    hbox_ok -> addWidget(ok);
    hbox_ok -> addWidget(cancel);
    connect(ok, SIGNAL(clicked(bool)), this, SLOT(accept()));
    connect(cancel, SIGNAL(clicked(bool)), this, SLOT(reject()));

    gridLayout = new QGridLayout();
    gridLayout -> addWidget(new QLabel("The range number is even !"), 0, 0);
    gridLayout -> addWidget(new QLabel("Input the number of center region !"), 1, 0);
    gridLayout -> addWidget(rangeBox, 1, 1);

    gridLayout -> addLayout(openHbox, 4, 0);
    gridLayout -> addLayout(saveHbox, 5, 0);
    gridLayout -> addLayout(hbox_ok, 6, 0);

    this -> setLayout(gridLayout);
    this -> setWindowTitle("Extract the mean image");

}

void ExtractMeanCenterDialog :: selectFolder()
{
    QPushButton *button = (QPushButton *) sender();
    if(button == openButton)
    {
        openFolderFiles = QFileDialog::getExistingDirectory(NULL, tr("Select a folder"),"D:\\");
        openEdit ->setText(openFolderFiles);
        if(openFolderFiles.isEmpty())
        {
            openEdit->setText("Select a folder !");
            v3d_msg("Folder is empty ! Input again !");
        }
    }
}

void ExtractMeanCenterDialog :: saveFiles()
{
    QPushButton *button1 = (QPushButton *) sender();
    if(button1 == saveButton)
    {
        saveFolderFiles = QFileDialog :: getExistingDirectory(0, tr("Select a folder to save files"), openFolderFiles);
        saveEdit->setText(saveFolderFiles);
        if(saveFolderFiles.isEmpty())
        {
            saveEdit->setText("Select a folder to save files !");
            v3d_msg("Please input a folder to save again !");
        }
    }

}



bool ExtractMeanCenterDialog :: extractImage(V3DPluginCallback2 &callback, QFileInfoList imageList, unsigned short * & pdata_XZ, unsigned short * & pdata_YZ, V3DLONG rangeNum, const QString openFolderFiles, const QString saveFolderFiles)
{

     unsigned char * data1d = 0;
     V3DLONG in_sz[4]={0};
     int datatype;

     QString file = imageList[0].filePath();
     if(!simple_loadimage_wrapper(callback, file.toStdString().c_str(), data1d, in_sz, datatype))
     {
         qDebug()<< "Error happens in reading the subject file.";
         return false;
     }

     if (rangeNum > in_sz[0] ||rangeNum > in_sz[1])
         return false;
     cout<<"datatype = "<<datatype<<endl;
     for(V3DLONG i = 0; i < 4; i ++)
         cout<<"in_sz["<<i<<"] = "<<in_sz[i]<<endl;
     V3DLONG imageN = in_sz[0]*in_sz[1]*in_sz[2]*in_sz[3];

     if (pdata_XZ) {delete []pdata_XZ; pdata_XZ=0;}
     if (pdata_YZ) {delete []pdata_YZ; pdata_YZ=0;}
     pdata_XZ = new unsigned short[in_sz[0] * imageList.size() * datatype/2];
     pdata_YZ = new unsigned short[in_sz[1] * imageList.size() * datatype/2];

     for(V3DLONG i = 0; i < imageList.size(); i ++)
     {
         cout<<"This is "<<i+1<<" image"<<endl;
         unsigned char * data1d_temp = 0;
         V3DLONG in_sz_temp[4]={0};
         int datatype_temp;
         if(!simple_loadimage_wrapper(callback, imageList[i].filePath().toStdString().c_str(), data1d_temp, in_sz_temp, datatype_temp))
         {
             qDebug()<< "Error happens in reading the subject file.";
             return false;
         }

         unsigned short * data1d_copy = new unsigned short[imageN*datatype/2];
         memcpy(data1d_copy, data1d_temp, imageN*datatype);

         for(V3DLONG j = 0; j < in_sz[0]; j ++)
         {
             V3DLONG record1 = 0, record2 = 0;
             double values1 = 0, sum1 = 0;
             double values2 = 0, sum2 = 0;

             // XZ image
             V3DLONG msx = 0, mex = 0;
             V3DLONG nsx = in_sz_temp[1]/2-rangeNum/2;
             V3DLONG nex = in_sz_temp[1]/2+rangeNum/2-1;

             msx = ((j-rangeNum/2)>0) ? (j-rangeNum/2) : 0;
             mex = ((j+rangeNum/2-1)<in_sz_temp[0]) ? (j+rangeNum/2-1) : (in_sz_temp[0]-1);


             for(V3DLONG mx = msx; mx <= mex; mx ++)
                 for(V3DLONG nx = nsx; nx <= nex; nx ++)
                {
                     sum1 += data1d_copy[nx*in_sz_temp[0]+mx];
                     record1 ++;
                }

             //             cout<<"record = "<<record<<endl;
                          values1 = sum1/record1;
             //             cout<<"values1 = "<<values1<<endl;
                          pdata_XZ[i*in_sz_temp[0]+j] = values1;

             //YZ image
             V3DLONG msy = in_sz_temp[0]/2-rangeNum/2;
             V3DLONG mey = in_sz_temp[0]/2+rangeNum/2-1;
             V3DLONG nsy = 0;
             V3DLONG ney = 0;

             nsy = ((j-rangeNum/2)>0) ? (j-rangeNum/2) : 0;
             ney = ((j+rangeNum/2-1)<in_sz_temp[1]) ? (j+rangeNum/2-1) : (in_sz_temp[1]-1);


             for(V3DLONG my = msy; my <= mey; my ++)
                 for(V3DLONG ny = nsy; ny <= ney; ny ++)
                {
                     sum2 += data1d_copy[ny*in_sz_temp[0]+my];
                     record2 ++;
                }

             values2 = sum2/record2;
             pdata_YZ[i*in_sz_temp[0]+j] = values2;
         }

      }

      QFileInfo midname(openFolderFiles);
      QString save_XZ = saveFolderFiles + "\\" + midname.baseName() + "_Mean_XZ.v3draw";
      QString save_YZ = saveFolderFiles + "\\" + midname.baseName() + "_Mean_YZ.v3draw";

      // Save XZ image
       unsigned char * pdata1d_XZ = new unsigned char[in_sz[0] * imageList.size()*datatype];
       memcpy(pdata1d_XZ, pdata_XZ, in_sz[0] * imageList.size()*datatype);

       V3DLONG in_sz_XZ[4]={in_sz[0], imageList.size(), 1, 1};

       if(simple_saveimage_wrapper(callback, save_XZ.toStdString().c_str(), pdata1d_XZ, in_sz_XZ, datatype))
       {
           qDebug()<<"Save XZ successfully!";
       }

       // Save YZ image
       unsigned char * pdata1d_YZ = new unsigned char[in_sz[1] * imageList.size()*datatype];
       memcpy(pdata1d_YZ, pdata_YZ, in_sz[1] * imageList.size()*datatype);

       V3DLONG in_sz_YZ[4]={in_sz[1], imageList.size(), 1, 1};

       if(simple_saveimage_wrapper(callback, save_YZ.toStdString().c_str(), pdata1d_YZ, in_sz_YZ, datatype))
       {
           qDebug()<<"Save YZ successfully!";
       }
}































