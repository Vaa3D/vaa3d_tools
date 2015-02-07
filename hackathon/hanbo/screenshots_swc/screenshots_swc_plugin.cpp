/* screenshots_swc_plugin.cpp
 * This plugin will make screenshots for a series swc files.
 * 2015-02-06 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "screenshots_swc_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(screenshots_swc, screenshots_swc);
 
QStringList screenshots_swc::menulist() const
{
	return QStringList() 
		<<tr("generate_screenshots")
		<<tr("about");
}

QStringList screenshots_swc::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void screenshots_swc::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("generate_screenshots"))
	{
        //select the window to operate
        QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
        QList <V3dR_MainWindow *> selectWindowList;
        V3dR_MainWindow * v3dwin;
//        QList<NeuronTree> * ntTreeList;
        int winid;
        qDebug("search for 3D windows");
        for (V3DLONG i=0;i<allWindowList.size();i++)
        {
//            ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
//            if(ntTreeList->size()>=1)
                selectWindowList.append(allWindowList[i]);
        }
        qDebug("match and select 3D windows");
        if(selectWindowList.size()<1){
            v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to tile in the same 3D view");
            return;
        }else if(selectWindowList.size()>1){
            //pop up a window to select

            QStringList items;
            for(int i=0; i<selectWindowList.size(); i++){
                items.append(callback.getImageName(selectWindowList[i]));
            }
            bool ok;
            QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("Neuron Tile Viewer"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
            if(!ok) return;
            for(int i=0; i<selectWindowList.size(); i++){
                if(selectitem==callback.getImageName(selectWindowList[i]))
                {
                    winid=i;
                    break;
                }
            }
        }else{
            winid=0;
        }
        v3dwin = selectWindowList[winid];

        screenshots_swc_dialog * myDialog = NULL;
        myDialog = new screenshots_swc_dialog(&callback, v3dwin);
        myDialog->show();
	}
	else
	{
		v3d_msg(tr("This plugin will make screenshots for a series swc files.. "
			"Developed by Hanbo Chen, 2015-02-06"));
	}
}

bool screenshots_swc::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("func2"))
	{
		v3d_msg("To be implemented.");
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

screenshots_swc_dialog::screenshots_swc_dialog(V3DPluginCallback2 * cb, V3dR_MainWindow* inwin)
{
    callback=cb;
    v3dwin=inwin;
    creat();
}


void screenshots_swc_dialog::creat()
{
    QGridLayout *gridLayout = new QGridLayout();

    //I/O zone
    QLabel* label_load = new QLabel(QObject::tr("SWC files folder:"));
    gridLayout->addWidget(label_load,0,0,1,1);
    edit_load = new QLineEdit();
    edit_load->setText(""); edit_load->setReadOnly(true);
    gridLayout->addWidget(edit_load,0,1,1,4);
    btn_load = new QPushButton("...");
    gridLayout->addWidget(btn_load,0,5,1,1);

    QLabel* label_output = new QLabel(QObject::tr("Output Prefix:"));
    gridLayout->addWidget(label_output,1,0,1,1);
    edit_output = new QLineEdit();
    edit_output->setText(""); edit_output->setReadOnly(true);
    gridLayout->addWidget(edit_output,1,1,1,4);
    btn_output = new QPushButton("...");
    gridLayout->addWidget(btn_output,1,5,1,1);

    connect(btn_load, SIGNAL(clicked()), this, SLOT(load()));
    connect(btn_output, SIGNAL(clicked()), this, SLOT(output()));

    //marker zone
    QFrame *line_1 = new QFrame();
    line_1->setFrameShape(QFrame::HLine);
    line_1->setFrameShadow(QFrame::Sunken);
    gridLayout->addWidget(line_1,4,0,1,6);

    btn_run = new QPushButton("Make ScreenShots");
    gridLayout->addWidget(btn_run,11,4,1,2);

    connect(btn_run, SIGNAL(clicked()), this, SLOT(run()));

    setLayout(gridLayout);
}


bool screenshots_swc_dialog::load()
{
    checkwindow();

    fname_input = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the input folder "),
                                               QDir::currentPath(),0);
    if(fname_input.isEmpty()){
        return false;
    }
    edit_load->setText(fname_input);

    return true;
}

bool screenshots_swc_dialog::output()
{
    checkwindow();

    fname_output = QFileDialog::getExistingDirectory(0, QObject::tr("Choose the output folder "),
                                                             fname_input,0);
    if(fname_input.isEmpty()){
        return false;
    }
    edit_output->setText(fname_output);

    return true;
}

void screenshots_swc_dialog::checkwindow()
{
    //check if current window is closed
    if (!callback){
        this->hide();
        return;
    }

    bool isclosed = true;
    //search to see if the window is still open
    QList <V3dR_MainWindow *> allWindowList = callback->getListAll3DViewers();
    for (V3DLONG i=0;i<allWindowList.size();i++){
        if(allWindowList.at(i)==v3dwin){
            isclosed = false;
            break;
        }
    }

    //close the window
    if(isclosed){
        this->hide();
        return;
    }
}

void screenshots_swc_dialog::run()
{
    checkwindow();

    QStringList swcList = importSWCFileList_addnumbersort(fname_input);

    v3d_msg("cojoc: "+QString::number(swcList.size())+" swc files found");

    for(int i=0; i<swcList.size(); i++){
        //load and update window
        NeuronTree nt = readSWC_file(swcList.at(i));
        nt.color.r=nt.color.g=nt.color.b=0;
        nt.color.a=255;
        callback->getHandleNeuronTrees_Any3DViewer(v3dwin)->clear();
        callback->getHandleNeuronTrees_Any3DViewer(v3dwin)->append(nt);
        callback->pushObjectIn3DWindow(v3dwin);
        callback->update_NeuronBoundingBox(v3dwin);
        callback->update_3DViewer(v3dwin);

        //save
        QFile file(swcList.at(i));
        QString output = QDir(fname_output).filePath(file.fileName());
        callback->screenShot_Any3DViewer(v3dwin, output);
    }
}

QStringList importSWCFileList_addnumbersort(const QString & curFilePath)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";

    QDir dir(curFilePath);
    if (!dir.exists())
    {
        qWarning("Cannot find the directory");
        return myList;
    }

    foreach (QString file, dir.entryList(imgSuffix, QDir::Files, QDir::Name))
    {
        myList += QFileInfo(dir, file).absoluteFilePath();
    }

    // print filenames
    foreach (QString qs, myList)  qDebug() << qs;

    return myList;
}
