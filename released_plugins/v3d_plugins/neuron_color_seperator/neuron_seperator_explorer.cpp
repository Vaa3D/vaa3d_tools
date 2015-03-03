#include "neuron_seperator_explorer.h"
#include <fstream>
#include <iostream>

#define MAX(a,b) (((a)>(b))?(a):(b))

using namespace std;
neuron_seperator_explorer::neuron_seperator_explorer(V3DPluginCallback2 * cb, QWidget *parent)
{
    creat();
    idx_ext=0;
    idx_img=0;
    checkButton();
    image1Dc_in=0;
    image1Dc_out=0;
    callback = cb;

    keyPressMask=false;
}

void neuron_seperator_explorer::creat()
{
    QGridLayout *gridLayout = new QGridLayout();

    //display zone
    QLabel* label_load = new QLabel(QObject::tr("Working Project:"));
    gridLayout->addWidget(label_load,1,0,1,2);
    btn_loadPoj = new QPushButton("Load Project"); btn_loadPoj->setAutoDefault(false);
    gridLayout->addWidget(btn_loadPoj,1,5,1,1);
    btn_loadDir = new QPushButton("Load .ANO Files"); btn_loadDir->setVisible(false);
    gridLayout->addWidget(btn_loadDir,1,4,1,1);
    edit_dir = new QLineEdit();
    edit_dir->setText(""); edit_dir->setReadOnly(true);
    gridLayout->addWidget(edit_dir,2,0,1,6);

    QLabel* label_img = new QLabel(QObject::tr("Checking Image:"));
    gridLayout->addWidget(label_img,3,0,1,2);
    btn_preImg = new QPushButton("Previous (4)"); btn_preImg->setAutoDefault(false);
    gridLayout->addWidget(btn_preImg,3,2,1,1);
    btn_nextImg = new QPushButton("Next (+)"); btn_nextImg->setAutoDefault(false);
    gridLayout->addWidget(btn_nextImg,3,5,1,1);
    btn_rejectImg = new QPushButton("Is Outlier (5)"); btn_rejectImg->setAutoDefault(false);
    gridLayout->addWidget(btn_rejectImg,3,4,1,1);
    btn_rerunImg = new QPushButton("Need Rerun (6)"); btn_rerunImg->setAutoDefault(false);
    gridLayout->addWidget(btn_rerunImg,3,3,1,1);
    edit_curimg = new QLineEdit();
    edit_curimg->setText(""); edit_curimg->setReadOnly(true);
    gridLayout->addWidget(edit_curimg,4,0,1,6);

    QLabel* label_ext = new QLabel(QObject::tr("Checking Extraction:"));
    gridLayout->addWidget(label_ext,5,0,1,2);
    btn_acceptExt = new QPushButton("Accept and Next (0)"); btn_acceptExt->setAutoDefault(false);
    gridLayout->addWidget(btn_acceptExt,5,5,1,1);
    btn_rejectExt = new QPushButton("Reject and Next (Enter)"); btn_rejectExt->setAutoDefault(false);
    gridLayout->addWidget(btn_rejectExt,5,4,1,1);
    btn_preExt = new QPushButton("Previous (1)"); btn_preExt->setAutoDefault(false);
    gridLayout->addWidget(btn_preExt,5,3,1,1);
    edit_curext = new QLineEdit();
    edit_curext->setText(""); edit_curext->setReadOnly(true);
    gridLayout->addWidget(edit_curext,6,0,1,6);

    //marker zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,10,0,1,6);
    btn_screenshot = new QPushButton("ScreenShot (8)"); btn_screenshot->setAutoDefault(false);
    gridLayout->addWidget(btn_screenshot,11,3,1,1);
    btn_save = new QPushButton("Save"); btn_save->setAutoDefault(false);
    gridLayout->addWidget(btn_save,11,4,1,1);
    btn_quit = new QPushButton("Quit"); btn_quit->setAutoDefault(false);
    gridLayout->addWidget(btn_quit,11,5,1,1);

    connect(btn_loadPoj, SIGNAL(clicked()), this, SLOT(loadPoj()));
    connect(btn_acceptExt, SIGNAL(clicked()), this, SLOT(acceptExt()));
    connect(btn_rejectExt, SIGNAL(clicked()), this, SLOT(rejectExt()));
    connect(btn_rejectImg, SIGNAL(clicked()), this, SLOT(rejectImg()));
    connect(btn_rerunImg, SIGNAL(clicked()), this, SLOT(needRerunImg()));
    connect(btn_preExt, SIGNAL(clicked()), this, SLOT(preExt()));
    connect(btn_preImg, SIGNAL(clicked()), this, SLOT(preImg()));
    connect(btn_nextImg, SIGNAL(clicked()), this, SLOT(nextImg()));
    connect(btn_save, SIGNAL(clicked()), this, SLOT(save()));
    connect(btn_screenshot, SIGNAL(clicked()), this, SLOT(screenShot()));

    connect(btn_quit, SIGNAL(clicked()), this, SLOT(reject()));

//    QAction *act_rejectExt = new QAction(this);
//    act_rejectExt->setShortcut(Qt::Key_Enter);
//    connect(act_rejectExt, SIGNAL(triggered()), this, SLOT(rejectExt()));
//    this->addAction(act_rejectExt);

//    QAction *act_acceptExt = new QAction(this);
//    act_acceptExt->setShortcut(Qt::Key_0);
//    connect(act_acceptExt, SIGNAL(triggered()), this, SLOT(acceptExt()));
//    this->addAction(act_acceptExt);

//    QAction *act_preExt = new QAction(this);
//    act_preExt->setShortcut(Qt::Key_1);
//    connect(act_preExt, SIGNAL(triggered()), this, SLOT(preExt()));
//    this->addAction(act_preExt);

//    QAction *act_preImg = new QAction(this);
//    act_preImg->setShortcut(Qt::Key_4);
//    connect(act_preImg, SIGNAL(triggered()), this, SLOT(preImg()));
//    this->addAction(act_preImg);

//    QAction *act_rejectImg = new QAction(this);
//    act_rejectImg->setShortcut(Qt::Key_5);
//    connect(act_rejectImg, SIGNAL(triggered()), this, SLOT(rejectImg()));
//    this->addAction(act_rejectImg);

//    QAction *act_rerunImg = new QAction(this);
//    act_rerunImg->setShortcut(Qt::Key_6);
//    connect(act_rerunImg, SIGNAL(triggered()), this, SLOT(needRerunImg()));
//    this->addAction(act_rerunImg);

//    QAction *act_nextImg = new QAction(this);
//    act_nextImg->setShortcut(Qt::Key_Plus);
//    connect(act_nextImg, SIGNAL(triggered()), this, SLOT(nextImg()));
//    this->addAction(act_nextImg);

//    QAction *act_screenshot = new QAction(this);
//    act_screenshot->setShortcut(Qt::Key_8);
//    connect(act_screenshot, SIGNAL(triggered()), this, SLOT(screenShot()));
//    this->addAction(act_screenshot);

    setLayout(gridLayout);
}

void  neuron_seperator_explorer::keyPressEvent(QKeyEvent* event)
{
    if(event->isAutoRepeat())
        return;

    keyPressMask=true;
}

void  neuron_seperator_explorer::keyReleaseEvent(QKeyEvent* event)
{
    if(event->isAutoRepeat())
        return;
    if(!keyPressMask)
        return;

    if(event->key() == Qt::Key_Enter){
        rejectExt();
    }
    if(event->key() == Qt::Key_0){
        acceptExt();
    }
    if(event->key() == Qt::Key_1){
        preExt();
    }
    if(event->key() == Qt::Key_4){
        preImg();
    }
    if(event->key() == Qt::Key_5){
        rejectImg();
    }
    if(event->key() == Qt::Key_6){
        needRerunImg();
    }
    if(event->key() == Qt::Key_Plus){
        nextImg();
    }
    if(event->key() == Qt::Key_8){
        screenShot();
    }

    keyPressMask=false;
}


void neuron_seperator_explorer::acceptExt()
{
    if(idx_img>=this->imgs.size())
        return;
    if(idx_img<0)
        return;

    if(idx_ext==0)
        imgs[idx_img].status = 1;

    this->imgs[idx_img].status_extract[idx_ext]=1;

    idx_ext++;
    if(idx_ext>=this->imgs.at(idx_img).fnames_extract.size()){
        imgs[idx_img].status = 1;
        if(idx_img<this->imgs.size()-1){
            idx_ext=0;
            idx_img++;
        }else{
            idx_ext--;
        }
    }

    updateAll();
}

void neuron_seperator_explorer::rejectExt()
{
    if(idx_img>=this->imgs.size())
        return;
    if(idx_img<0)
        return;
    if(idx_ext==0)
        return;

    this->imgs[idx_img].status_extract[idx_ext]=2;

    idx_ext++;
    if(idx_ext>=this->imgs.at(idx_img).fnames_extract.size()){
        imgs[idx_img].status = 1;
        if(idx_img<this->imgs.size()-1){
            idx_ext=0;
            idx_img++;
        }else{
            idx_ext--;
        }
    }

    updateAll();
}

void neuron_seperator_explorer::rejectImg()
{
    if(idx_img>=this->imgs.size())
        return;
    if(idx_img<0)
        return;

    imgs[idx_img].status = 2;
    if(idx_img<this->imgs.size()-1){
        idx_ext=0;
        idx_img++;
    }

    updateAll();
}

void neuron_seperator_explorer::needRerunImg()
{
    if(idx_img>=this->imgs.size())
        return;
    if(idx_img<0)
        return;

    imgs[idx_img].status = 3;
    if(idx_img<this->imgs.size()-1){
        idx_ext=0;
        idx_img++;
    }

    updateAll();
}

void neuron_seperator_explorer::preExt()
{
    if(idx_img>=this->imgs.size())
        return;
    if(idx_img<=0 && idx_ext<=0)
        return;

    idx_ext--;
    if(idx_ext<0){
        idx_img--;
        idx_ext=MAX(0,imgs.at(idx_img).fnames_extract.size()-1);
    }

    updateAll();
}

void neuron_seperator_explorer::preImg()
{
    if(idx_img>=this->imgs.size())
        return;
    if(idx_img<0)
        return;

    if(idx_img>0){
        idx_ext=0;
        idx_img--;
    }

    updateAll();
}

void neuron_seperator_explorer::nextImg()
{
    if(idx_img>=this->imgs.size()-1)
        return;
    if(idx_img<0)
        return;

    idx_ext=0;
    idx_img++;

    updateAll();
}

void neuron_seperator_explorer::save()
{
    QString fname_output;
    fname_output=edit_dir->text();
    fname_output=QFileDialog::getSaveFileName(0, QObject::tr("Output Folder and Prefix "),
                                                   fname_output,
                                                   QObject::tr("projects (*);;All(*)"));

    if(fname_output.isEmpty()){
        return;
    }

    save(fname_output);
}

void neuron_seperator_explorer::save(QString fname_output)
{
    QString fname_acceptAno=fname_output+"_ano_accept.txt";
    QString fname_rejectAno=fname_output+"_ano_reject.txt";
    QString fname_todoAno=fname_output+"_ano_todo.txt";
    QString fname_outlierImg=fname_output+"_img_outlier.txt";
    QString fname_rerunImg=fname_output+"_img_rerun.txt";
    QString fname_acceptExtract=fname_output+"_extract_accept.txt";
    QString fname_rejectExtract=fname_output+"_extract_reject.txt";

    ofstream fp_acceptAno(fname_acceptAno.toStdString().c_str());
    if(!fp_acceptAno.is_open()){
        v3d_msg("error: failed to open "+fname_acceptAno);
        return;
    }
    ofstream fp_rejectAno(fname_rejectAno.toStdString().c_str());
    if(!fp_rejectAno.is_open()){
        v3d_msg("error: failed to open "+fname_rejectAno);
        return;
    }
    ofstream fp_todoAno(fname_todoAno.toStdString().c_str());
    if(!fp_todoAno.is_open()){
        v3d_msg("error: failed to open "+fname_todoAno);
        return;
    }
    ofstream fp_outlierImg(fname_outlierImg.toStdString().c_str());
    if(!fp_outlierImg.is_open()){
        v3d_msg("error: failed to open "+fname_outlierImg);
        return;
    }
    ofstream fp_rerunImg(fname_rerunImg.toStdString().c_str());
    if(!fp_rerunImg.is_open()){
        v3d_msg("error: failed to open "+fname_rerunImg);
        return;
    }
    ofstream fp_acceptExtract(fname_acceptExtract.toStdString().c_str());
    if(!fp_acceptExtract.is_open()){
        v3d_msg("error: failed to open "+fname_acceptExtract);
        return;
    }
    ofstream fp_rejectExtract(fname_rejectExtract.toStdString().c_str());
    if(!fp_rejectExtract.is_open()){
        v3d_msg("error: failed to open "+fname_rejectExtract);
        return;
    }

    for(int i=0; i<imgs.size(); i++){
        if(imgs.at(i).status == 0){
            fp_todoAno<<fnames_ano.at(i).toStdString().c_str()<<endl;
            continue;
        }else if(imgs.at(i).status > 1){
            fp_rejectAno<<fnames_ano.at(i).toStdString().c_str()<<endl;
            if(imgs.at(i).status == 2)
                fp_outlierImg<<imgs.at(i).fnames_extract.at(0).toStdString().c_str()<<endl;
            else if(imgs.at(i).status == 3)
                fp_rerunImg<<imgs.at(i).fnames_extract.at(0).toStdString().c_str()<<endl;
            continue;
        }else{
            fp_acceptAno<<fnames_ano.at(i).toStdString().c_str()<<endl;
            for(int j=1; j<imgs.at(i).fnames_extract.size(); j++)
            {
                if(imgs.at(i).status_extract.at(j)==1){
                    fp_acceptExtract<<imgs.at(i).fnames_extract.at(j).toStdString().c_str()<<endl;
                }
                if(imgs.at(i).status_extract.at(j)==2){
                    fp_rejectExtract<<imgs.at(i).fnames_extract.at(j).toStdString().c_str()<<endl;
                }
            }
        }
    }

    fp_acceptAno.close();
    fp_rejectAno.close();
    fp_todoAno.close();
    fp_acceptExtract.close();
    fp_rejectExtract.close();
    fp_outlierImg.close();
    fp_rerunImg.close();
}

void neuron_seperator_explorer::screenShot()
{
    if(imgs.size()==0){
        return;
    }
    QString fname=this->anopath+"_screenshot_ano"+QString::number(idx_img)+"_ext"+QString::number(idx_ext)+"_a.tif";
    if(!simple_saveimage_wrapper(*callback, fname.toStdString().c_str(), image1Dc_in, sz_img, intype)){
        v3d_msg("Error:failed to save screenshot to "+fname);
        return;
    }
    fname=this->anopath+"_screenshot_ano"+QString::number(idx_img)+"_ext"+QString::number(idx_ext)+"_b.tif";
    if(!simple_saveimage_wrapper(*callback, fname.toStdString().c_str(), image1Dc_out, sz_img, intype)){
        v3d_msg("Error:failed to save screenshot to "+fname);
        return;
    }
    v3d_msg("ScreenShot saved to "+fname);
}

void neuron_seperator_explorer::loadPoj()
{
    QString fname_input;
    if(this->edit_dir->text().isEmpty()){
        fname_input="";
    }else{
        fname_input=this->edit_dir->text();
    }
    fname_input = QFileDialog::getOpenFileName(0, QObject::tr("Choose the working project "),
                                                   fname_input,
                                                   QObject::tr("projects (*.txt);;All(*)"));

    if(fname_input.isEmpty()){
        return;
    }

    qDebug()<<"NeuronSeperatorExplorer: is going to load project"<<fname_input;
    QFile file(fname_input);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::information(0, "error", file.errorString());
        return;
    }

    fnames_ano.clear();
    imgs.clear();
    idx_img=0;
    idx_ext=0;
    pre_img=-1;
    pre_ext=-1;

    QTextStream in(&file);
    do{
        //this will check the progress
//        QString line = in.readLine();
//        QStringList fields = line.split(QRegExp("\\s"));
//        if(!QFile::exists(fields.at(0))){
//            continue;
//        }
//        if(fields.size()>1){
//            bool ok;
//            int status = fields.at(1).toInt(&ok);
//            if(ok){
//                extraction tmp_ext;
//                tmp_ext.status = status;
//                this->fnames_ano.append(fields.at(0));
//                this->imgs.push_back(tmp_ext);

//                if(status!=0) idx_img=imgs.size();
//            }
//        }
        QString line = in.readLine();
        if(QFile::exists(line.trimmed())){
            extraction tmp_ext;
            tmp_ext.status = 0;
            this->fnames_ano.append(line.trimmed());
            this->imgs.push_back(tmp_ext);
        }
    }while(!in.atEnd());
    file.close();

    edit_dir->setText(fname_input);
    anopath=fname_input;
    v3d_msg("Project file: " + fname_input + " loaded.\nFound "
            + QString::number(fnames_ano.size()) + " ANO files.");
//            + QString::number(idx_img) + " were already checked.");

    updateAll();
}

void neuron_seperator_explorer::updateAll()
{
    qDebug()<<"cojoc: "<<idx_img<<":"<<idx_ext;
    checkButton();

    if(idx_img<0 || idx_img>=imgs.size()){
        return;
    }

    //init anoinfo on first visit
    if(imgs.at(idx_img).fnames_extract.size()==0){
        imgs[idx_img].status_extract.clear();
        QFile file(fnames_ano.at(idx_img));
        if(!file.open(QIODevice::ReadOnly)) {
            QMessageBox::information(0, "error", file.errorString());
            idx_ext=pre_ext;
            idx_img=pre_img;
            return;
        }

        QTextStream in(&file);
        while(!in.atEnd()) {
            QString line = in.readLine();
            QStringList fields = line.split(QRegExp("="));

            if(fields.size()>1){
                if(!QFile::exists(fields.at(1).trimmed())){
                    continue;
                }
                imgs[idx_img].fnames_extract.push_back(fields.at(1).trimmed());
                imgs[idx_img].status_extract.push_back(0);
            }
        }
        file.close();
    }

    //reload background image and push for visualization
    if(pre_img!=idx_img){
        QString tmp=imgs[idx_img].fnames_extract.at(0)
                +"("+QString::number(idx_img+1)+"/"+QString::number(imgs.size())+")";
        edit_curimg->setText(tmp);
        qDebug()<<"the file to open=["<<imgs[idx_img].fnames_extract.at(0)<<"]";
        if(!simple_loadimage_wrapper(*callback, imgs[idx_img].fnames_extract.at(0).toStdString().c_str(), image1Dc_in, sz_img, intype))
        {
          v3d_msg("load image "+imgs[idx_img].fnames_extract.at(0)+" error!");
          return;
        }
        if(image1Dc_out!=0) delete[] image1Dc_out;
        image1Dc_out = new unsigned char[sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]];

        updateInputWindow();
        save(this->anopath+"_autoSave");

        pre_img=idx_img;
    }

    //append extraction on original image and visualize
    if(idx_ext>0 && idx_ext<imgs[idx_img].fnames_extract.size()){
        qDebug()<<"cojoc: loading seperate image";
        QString tmp=imgs[idx_img].fnames_extract.at(idx_ext)
                +"("+QString::number(idx_ext)+"/"+QString::number(imgs.at(idx_img).fnames_extract.size()-1)+")";
        edit_curext->setText(tmp);
        unsigned char *image1Dc_tmp=0;
        qDebug()<<"the file to open=["<<imgs[idx_img].fnames_extract.at(idx_ext)<<"]";
        if(!simple_loadimage_wrapper(*callback, imgs[idx_img].fnames_extract.at(idx_ext).toStdString().c_str(), image1Dc_tmp, sz_out, outtype))
        {
          v3d_msg("load image "+imgs[idx_img].fnames_extract.at(idx_ext)+" error!");
          return;
        }
        if(sz_img[0]!=sz_out[0]) return;
        if(sz_img[1]!=sz_out[1]) return;
        if(sz_img[2]!=sz_out[2]) return;
        if(sz_out[3]!=1) return;
        memcpy(image1Dc_out,image1Dc_in,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        for(V3DLONG vid=0; vid<sz_img[0]*sz_img[1]*sz_img[2]; vid++){
            if(image1Dc_tmp[vid]!=0){
                for(V3DLONG cid=0; cid<sz_img[3]; cid++){
                    image1Dc_out[vid+sz_img[0]*sz_img[1]*sz_img[2]*cid]=image1Dc_tmp[vid];
                }
            }
        }
        delete[] image1Dc_tmp;

        updateOutputWindow();

        pre_ext=idx_ext;
    }else if(idx_ext==0){
        qDebug()<<"cojoc: appending seperate image";
        unsigned char *image1Dc_tmp=0;
        memcpy(image1Dc_out,image1Dc_in,sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3]);
        for(int tmp_idx=1; tmp_idx<imgs[idx_img].fnames_extract.size(); tmp_idx++){
            QString tmp=imgs[idx_img].fnames_extract.at(tmp_idx)
                    +"("+QString::number(idx_ext)+"/"+QString::number(imgs.at(idx_img).fnames_extract.size()-1)+")";
            edit_curext->setText(tmp);
            qDebug()<<"the file to open=["<<imgs[idx_img].fnames_extract.at(tmp_idx)<<"]";
            if(!simple_loadimage_wrapper(*callback, imgs[idx_img].fnames_extract.at(tmp_idx).toStdString().c_str(), image1Dc_tmp, sz_out, outtype))
            {
              v3d_msg("load image "+imgs[idx_img].fnames_extract.at(tmp_idx)+" error!");
              return;
            }
            if(sz_img[0]!=sz_out[0]) continue;
            if(sz_img[1]!=sz_out[1]) continue;
            if(sz_img[2]!=sz_out[2]) continue;
            if(sz_out[3]!=1) continue;
            for(V3DLONG vid=0; vid<sz_img[0]*sz_img[1]*sz_img[2]; vid++){
                if(image1Dc_tmp[vid]!=0){
                    for(V3DLONG cid=0; cid<sz_img[3]; cid++){
                        int tmp=image1Dc_out[vid+sz_img[0]*sz_img[1]*sz_img[2]*cid]+image1Dc_tmp[vid];
                        image1Dc_out[vid+sz_img[0]*sz_img[1]*sz_img[2]*cid]=tmp<255?tmp:255;
                    }
                }
            }
        }
        delete[] image1Dc_tmp;

        updateOutputWindow();

        pre_ext=idx_ext;
    }
    else{
        QString tmp="(0/0)";
        edit_curext->setText(tmp);
    }

    checkButton();
}

void neuron_seperator_explorer::checkButton()
{
    if(imgs.size()==0){
        btn_acceptExt->setEnabled(false);
        btn_nextImg->setEnabled(false);
        btn_preExt->setEnabled(false);
        btn_preImg->setEnabled(false);
        btn_rejectExt->setEnabled(false);
        btn_rejectImg->setEnabled(false);
        btn_save->setEnabled(false);
        btn_rerunImg->setEnabled(false);
        btn_screenshot->setEnabled(false);
    }else{
        btn_acceptExt->setEnabled(true);
        btn_nextImg->setEnabled(true);
        btn_preExt->setEnabled(true);
        btn_preImg->setEnabled(true);
        btn_rejectExt->setEnabled(true);
        btn_rejectImg->setEnabled(true);
        btn_save->setEnabled(true);
        btn_rerunImg->setEnabled(true);
        btn_screenshot->setEnabled(true);
        if(idx_img>=imgs.size()-1){
            btn_nextImg->setEnabled(false);
            if(idx_img>=imgs.size()){
                btn_rejectImg->setEnabled(false);
                btn_rerunImg->setEnabled(false);
            }
            if(idx_ext>=imgs.at(idx_img).fnames_extract.size()-1){
                btn_acceptExt->setEnabled(false);
                btn_rejectExt->setEnabled(false);
            }
        }
        if(idx_img<=0){
            btn_preImg->setEnabled(false);
            if(idx_ext<=0){
                btn_preExt->setEnabled(false);
            }
        }
        if(idx_ext==0){
            btn_rejectExt->setEnabled(false);
        }
    }
}

void neuron_seperator_explorer::updateInputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
        {
            winfound=true;
        }
    }
    if(image1Dc_in != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;
        image4d.setFileName(this->imgs.at(idx_img).fnames_extract.at(0).toStdString().c_str());
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1D_input=new unsigned char[size_page*intype];
        memcpy(image1D_input, image1Dc_in, size_page*intype);
        image4d.setData(image1D_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], (ImagePixelType)intype);

        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            //update the image
            callback->setImage(v3dhandle_main, &image4d);
            callback->setImageName(v3dhandle_main, NAME_INWIN);
            callback->updateImageWindow(v3dhandle_main);
        }else{
            v3dhandleList_current=callback->getImageWindowList();
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_INWIN))
                {
                    //update the image
                    if(!callback->setImage(v3dhandleList_current[i], &image4d)){
                        v3d_msg("Failed to update input image");
                    }
                    callback->updateImageWindow(v3dhandleList_current[i]);
                }
            }
        }
    }
}

void neuron_seperator_explorer::updateOutputWindow()
{
    //search in open windows
    bool winfound=false;
    v3dhandleList v3dhandleList_current=callback->getImageWindowList();
    for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
    {
        if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
        {
            winfound=true;
        }
    }
    if(image1Dc_out != 0){ //image loaded
        //generate a copy and show it
        Image4DSimple image4d;
        image4d.setFileName(this->imgs.at(idx_img).fnames_extract.at(idx_ext).toStdString().c_str());
        V3DLONG size_page = sz_img[0]*sz_img[1]*sz_img[2]*sz_img[3];
        unsigned char* image1Dc_input=new unsigned char[size_page*intype];
        memcpy(image1Dc_input, image1Dc_out, size_page*intype);
        image4d.setData(image1Dc_input, sz_img[0], sz_img[1], sz_img[2], sz_img[3], (ImagePixelType)intype);

        if(!winfound){ //open a window if none is open
            v3dhandle v3dhandle_main=callback->newImageWindow();
            callback->setImage(v3dhandle_main, &image4d);
            callback->setImageName(v3dhandle_main, NAME_OUTWIN);
            callback->updateImageWindow(v3dhandle_main);
        }else{
            //update the image
            for (V3DLONG i=0;i<v3dhandleList_current.size();i++)
            {
                if(callback->getImageName(v3dhandleList_current[i]).contains(NAME_OUTWIN))
                {
                    if(!callback->setImage(v3dhandleList_current[i], &image4d)){
                        v3d_msg("Failed to update input image");
                    }
                    callback->updateImageWindow(v3dhandleList_current[i]);
                }
            }
        }
    }
}
