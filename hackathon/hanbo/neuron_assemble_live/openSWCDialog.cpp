#include "openSWCDialog.h"

OpenSWCDialog::OpenSWCDialog(QWidget * parent, V3DPluginCallback2 * _callback)
: QDialog(parent)
{
	callback = _callback;
    p_img4d = 0;
    getAllWindowsAndNeurons(callback);

    QVBoxLayout * dlgLayout = new QVBoxLayout;

	QTabWidget * tabWidget = new QTabWidget(this);
	//page open from 3D viewers
	QDialog * open3DDlg = new QDialog(tabWidget);
	QVBoxLayout* layout  = new QVBoxLayout;

    listWidget = new QListWidget(open3DDlg);
    QStringList fileList=_3DViewerTreeList_name;
    listWidget->addItems(fileList);
	layout->addWidget(listWidget);

	QPushButton * bt = new QPushButton(tr("choose"));
    connect(bt, SIGNAL(clicked()), this, SLOT(chooseNeuron()));
	layout->addWidget(bt);

	open3DDlg->setLayout(layout);
    tabWidget->addTab(open3DDlg, tr("Open Single SWC from 3D Viewer"));

    //page open from 3D viewers
    QDialog * open3DDlg1 = new QDialog(tabWidget);
    QVBoxLayout* layout1  = new QVBoxLayout;

    listWidget1 = new QListWidget(open3DDlg1);
    QStringList fileList1;
    for(int i=0; i<_3DViewerList.size(); i++){
        fileList1 << callback->getImageName(_3DViewerList[i]);
    }
    listWidget1->addItems(fileList1);
    if(fileList1.size()>0)
        listWidget1->setCurrentRow(0);
    layout1->addWidget(listWidget1);

    QPushButton * bt1 = new QPushButton(tr("choose"));
    connect(bt1, SIGNAL(clicked()), this, SLOT(chooseWindow()));
    layout1->addWidget(bt1);

    open3DDlg1->setLayout(layout1);
    tabWidget->addTab(open3DDlg1, tr("Open All SWCs in 3D Viewer"));

	//page open from file
    QSettings settings("V3D plugin","neuron_live_assembler");
    QString prevFile = "";
    if(settings.contains("fname_input"))
        prevFile = settings.value("fname_input").toString();
    QFileDialog * fileDlg = new QFileDialog(tabWidget, tr("Open File"), prevFile,
                QObject::tr("Supported file (*.swc *eswc *.ano)"
                    ";;Neuron structure	(*.swc *eswc)"
                    ";;Linker file (*.ano)"
					));
	fileDlg->setOption(QFileDialog::DontUseNativeDialog, true);
	connect(fileDlg, SIGNAL(fileSelected(const QString &)), this, SLOT(setTree(const QString &)));
    connect(fileDlg, SIGNAL(rejected()), this, SLOT(reject()));

	tabWidget->addTab(fileDlg, tr("Open from File"));
    if (fileList.isEmpty()) tabWidget->setTabEnabled(0, false);
    else listWidget->setCurrentRow(0);
    if (fileList1.isEmpty()) tabWidget->setTabEnabled(1, false);
    else listWidget1->setCurrentRow(0);

	dlgLayout->addWidget(tabWidget);
	setLayout(dlgLayout);
}

bool OpenSWCDialog::chooseNeuron()
{
	int row = listWidget->currentRow();
    if(row<0){
        v3d_msg("please select neuron");
        return false;
    }
    //get neuron
    QList<NeuronTree> * ntTreeList = callback->getHandleNeuronTrees_Any3DViewer(_3DViewerTreeList_window[row]);
    ntList.append((*ntTreeList)[_3DViewerTreeList_id[row]]);
    file_name = ntList.at(0).file;

    //get image if there is any
    QString winname=callback->getImageName(_3DViewerTreeList_window[row]);
    winname=winname.left(winname.size()-1);
    winname=winname.right(winname.size()-9);
    v3dhandleList winlist = callback->getImageWindowList();
    if(p_img4d!=0){
        delete p_img4d;
        p_img4d=0;
    }
    for(int i=0; i<winlist.size(); i++){
        if(callback->getImageName(winlist.at(i))==winname){
            Image4DSimple * p_tmp = callback->getImage(winlist.at(i));
            if(!p_tmp) continue;
            qDebug()<<"loaded image";
            p_img4d = new Image4DSimple();
            p_img4d->setFileName(p_tmp->getFileName());
            V3DLONG datasize = p_tmp->getXDim()*p_tmp->getYDim()*p_tmp->getZDim()*p_tmp->getCDim()*((int)p_tmp->getDatatype());
            unsigned char * p_data = new unsigned char[datasize];
            memcpy(p_data, p_tmp->getRawData(),datasize);
            p_img4d->setData(p_data, p_tmp);
        }
    }
    if(p_img4d==0){
        getImage();
    }

	
	accept();
	return true;
}

bool OpenSWCDialog::chooseWindow()
{
    int row = listWidget1->currentRow();
    if(row<0){
        v3d_msg("please select window");
        return false;
    }
    //get neuron
    QList<NeuronTree> * ntTreeList = callback->getHandleNeuronTrees_Any3DViewer(_3DViewerList[row]);
    for(int i=0; i<ntTreeList->size(); i++){
        ntList.append((*ntTreeList)[i]);
    }
    file_name = ntList.at(0).file;

    //get image if there is any
    QString winname=callback->getImageName(_3DViewerTreeList_window[row]);
    winname=winname.left(winname.size()-1);
    winname=winname.right(winname.size()-9);
    v3dhandleList winlist = callback->getImageWindowList();
    if(p_img4d!=0){
        delete p_img4d;
        p_img4d=0;
    }
    for(int i=0; i<winlist.size(); i++){
        if(callback->getImageName(winlist.at(i))==winname){
            Image4DSimple * p_tmp = callback->getImage(winlist.at(i));
            if(!p_tmp) continue;
            qDebug()<<"loaded image";
            p_img4d = new Image4DSimple();
            p_img4d->setFileName(p_tmp->getFileName());
            V3DLONG datasize = p_tmp->getXDim()*p_tmp->getYDim()*p_tmp->getZDim()*p_tmp->getCDim()*((int)p_tmp->getDatatype());
            unsigned char * p_data = new unsigned char[datasize];
            memcpy(p_data, p_tmp->getRawData(),datasize);
            p_img4d->setData(p_data, p_tmp);
        }
    }
    if(p_img4d==0){
        getImage();
    }

    accept();
    return true;
}

bool OpenSWCDialog::setTree(const QString & file)
{
    //get neuron
    QFileInfo fileinfo(file);
    if(fileinfo.suffix().toUpper().contains("SWC")){
        NeuronTree nt = readSWC_file(file);
        ntList.append(nt);
    }
	file_name = file;

    QSettings settings("V3D plugin","neuron_live_assembler");
    settings.setValue("fname_input",file);

    getImage();

	accept();
	return true;
}

void OpenSWCDialog::getImage()
{
    //get image
    if(p_img4d!=0){
        delete p_img4d;
        p_img4d=0;
    }
    QFileInfo fileinfo(file_name);
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Set Image", "Do you want to set an image of reconstructions to display?",QMessageBox::Yes|QMessageBox::No);
    if(reply == QMessageBox::Yes){
        QString fname_input;
        while(1){
            QSettings settings("V3D plugin","neuron_live_assembler");
            QString prevFile = "";
            if(settings.contains("fname_img"))
                prevFile = settings.value("fname_img").toString();
            else
                prevFile = fileinfo.dir().path();
            fname_input = QFileDialog::getOpenFileName(this, QObject::tr("Choose the Image of Bottom Section "),
                                                       prevFile,
                                                       QObject::tr("Images (*.raw *.tif *.lsm *.v3dpbd *.v3draw);;All(*)"));
            if(!fname_input.isEmpty()){
                unsigned char * p_data = 0;
                V3DLONG sz_img[4];
                int type_img;
                if(!simple_loadimage_wrapper(*callback, fname_input.toStdString().c_str(), p_data, sz_img, type_img)){
                    v3d_msg("failed to load image: "+fname_input);
                }else{
                    p_img4d = new Image4DSimple();
                    p_img4d->setFileName(fname_input.toStdString().c_str());
                    p_img4d->setData(p_data, sz_img[0], sz_img[1], sz_img[2], sz_img[3], (ImagePixelType)type_img);
                    settings.setValue("fname_img",fname_input);
                    break;
                }
            }else{
                break;
            }
        }
    }
}

void OpenSWCDialog::getAllWindowsAndNeurons(V3DPluginCallback2 * callback)
{
	QList <V3dR_MainWindow *> windowList = callback->getListAll3DViewers();
	for (V3DLONG i=0;i<windowList.size();i++)
    {
		QList<NeuronTree> * ntTreeList = callback->getHandleNeuronTrees_Any3DViewer(windowList[i]);
        if(ntTreeList->size()>0){
            _3DViewerList.append(windowList[i]);
        }
        for (V3DLONG j=0;j<ntTreeList->size();j++){
            _3DViewerTreeList_id.append(j);
            _3DViewerTreeList_name.append(ntTreeList->at(j).file);
            _3DViewerTreeList_window.append(windowList[i]);
        }
	}
	return;
}
