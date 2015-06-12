/* saveToanoPlugin_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-01-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "saveToanoPlugin_plugin.h"
#include <iostream>
#include <fstream>
#include "basic_surf_objs.h"




using namespace std;
Q_EXPORT_PLUGIN2(save3dviewer2linker, saveToanoPlugin);

void generatorAno43Dviewer(V3DPluginCallback2 &callback, QWidget *parent);
void generatorAno43Dviewer_timestamp(V3DPluginCallback2 &callback, QWidget *parent);

controlPanel* controlPanel::panel = 0;

 
QStringList saveToanoPlugin::menulist() const
{
	return QStringList() 
        <<tr("save all SWC, APO, and Surface files to an ANO file")
       <<tr("1_click_timestamp_save")
		<<tr("about");
}

QStringList saveToanoPlugin::funclist() const
{
	return QStringList()
		<<tr("help");
}

void saveToanoPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("save all SWC, APO, and Surface files to an ANO file"))
	{
        generatorAno43Dviewer(callback,parent);
	}
    if (menu_name == tr("1_click_timestamp_save"))
    {
        generatorAno43Dviewer_timestamp(callback,parent);
    }
	else
	{
        v3d_msg(tr("This is a plugin to save all SWC, APO, and Surface files from a 3D viewer,"
            "Developed by Zhi Zhou, 2013-01-13"));
	}
}

bool saveToanoPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

void generatorAno43Dviewer_timestamp(V3DPluginCallback2 &callback, QWidget *parent)
{
    //select the window to operate
    QList <V3dR_MainWindow *> allWindowList = callback.getListAll3DViewers();
    QList <V3dR_MainWindow *> selectWindowList;
    V3dR_MainWindow * v3dwin;
    QList<NeuronTree> * ntTreeList;
    int winid;
    qDebug("search for 3D windows");
    for (V3DLONG i=0;i<allWindowList.size();i++)
    {
        ntTreeList = callback.getHandleNeuronTrees_Any3DViewer(allWindowList[i]);
        if(ntTreeList->size()>0){
            QString fname_win=callback.getImageName(allWindowList[i]);
            if(fname_win.contains("[") && fname_win.contains("]"))
                selectWindowList.append(allWindowList[i]);
        }
    }
    qDebug("match and select 3D windows");
    if(selectWindowList.size()<1){
        v3d_msg("Cannot find 3D view with SWC file. Please load the SWC files you want to color in the 3D view");
        return;
    }else if(selectWindowList.size()>1){
        //pop up a window to select

        QStringList items;
        for(int i=0; i<selectWindowList.size(); i++){
            items.append(callback.getImageName(selectWindowList[i]));
        }
        bool ok;
        QString selectitem = QInputDialog::getItem(parent, QString::fromUtf8("1-click neuron color"), QString::fromUtf8("Select A Window to Operate"), items, 0, false, &ok);
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

    //get the file name
    QString fname_win=callback.getImageName(v3dwin).remove("3D View [").remove("]");
    QDateTime mytime = QDateTime::currentDateTime();
    QString fname_out=fname_win + "_stamp_" + mytime.toString("yyyy_MM_dd_hh_mm");
    qDebug()<<"Will save file to "<<fname_out;

    //get content
    ntTreeList=callback.getHandleNeuronTrees_Any3DViewer(v3dwin);
    QStringList SWC_list;
    for(int i=0; i<ntTreeList->size(); i++){
        QString fname_swc=fname_out;
        if(i>0){
            fname_swc=fname_swc+"_nt"+QString::number(i);
        }
        fname_swc=fname_swc+".swc";
        SWC_list.append(fname_swc);
    }

    DataLists_in_3dviewer listItem = callback.fetch_3dviewer_datafilelist(callback.getImageName(v3dwin));

    QList<NeuronTree> * mTreeList;
    mTreeList = callback.getHandleNeuronTrees_Any3DViewer(v3dwin);

    QList<CellAPO> * mAPOList;
    mAPOList = callback.getHandleAPOCellList_Any3DViewer(v3dwin);

    QList<CellAPO> mAPOList_v2;
    for (int i = 0; i < mAPOList->size();i++)
            mAPOList_v2.push_back(mAPOList->at(i));

    QString imgname = listItem.imgfile;
    QString labelfieldname = listItem.labelfield_file;
    QString surfacename = listItem.surface_file;
    QStringList APO_list = listItem.pointcloud_file_list;

    //start saving
    QString filename=fname_out+".ano";
    ofstream anofile(filename.toStdString().c_str(),ios::out | ios::app );
    if(!anofile.is_open()){
        v3d_msg("Error opening file to save: "+filename);
        return;
    }

    if(SWC_list.count()>0)
    {
        for(V3DLONG i = 0; i < SWC_list.count(); i++)
        {
            NeuronTree mTree = mTreeList->at(i);
            QString newSWCname = SWC_list.at(i);
            writeSWC_file(newSWCname,mTree);
            anofile << "SWCFILE=" << newSWCname.toStdString().c_str() << endl;
        }

    }

    if(APO_list.count()>0)
    {
            QString newAPOname = fname_out + "_" + QFileInfo(APO_list.at(0)).fileName();
            writeAPO_file(newAPOname,mAPOList_v2);
            anofile << "APOFILE=" << newAPOname.toStdString().c_str() << endl;
    }

    if(imgname.size()>0) anofile << "RAWIMG=" << imgname.toStdString().c_str() << endl;
    if(surfacename.size()>0) anofile << "SURFILE=" << surfacename.toStdString().c_str() << endl;
    if(labelfieldname.size()>0) anofile << "SURFILE=" << labelfieldname.toStdString().c_str() << endl;

    anofile.close();

    qDebug()<<QString("The ANO file is save in:[%1]").arg(filename.toStdString().c_str());
    return;
}

void generatorAno43Dviewer(V3DPluginCallback2 &v3d, QWidget *parent)
{
    if (controlPanel::panel)
    {
        controlPanel::panel->show();
        return;
    }
    else
    {
        controlPanel* p = new controlPanel(v3d, parent);
        if (p)
        {
            p->show();
            p->raise();
            p->move(50,50);
            p->activateWindow();
        }
    }
}

void MyComboBox::enterEvent(QEvent *e)
{
    updateList();
    QComboBox::enterEvent(e);
}

void MyComboBox::updateList()
{
    if (!m_v3d)
        return;

    QString lastDisplayfile = currentText();

    // now re-check the currently opened windows
    QList <V3dR_MainWindow *> cur_list_3dviewer = m_v3d->getListAll3DViewers();

    QStringList items;
    int i;

    for (i=0; i<cur_list_3dviewer.count(); i++)
    {
        items << m_v3d->getImageName(cur_list_3dviewer[i]);
    }

    //update the list now
    clear();
    addItems(items);

    //search if the lastDisplayfile exists, if yes, then highlight it (set as current),
    //otherwise do nothing (i.e. in this case the list will highlight the 1st one which is new)

    int curDisplayIndex=-1; //-1 for invalid index
    for (i=0; i<items.size(); i++)
        if (items[i]==lastDisplayfile)
        {
            curDisplayIndex = i;
            break;
        }

    if (curDisplayIndex>=0)
        setCurrentIndex(curDisplayIndex);

    //
    update();

    return;
}


controlPanel::controlPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    QPushButton* btn_saveano = new QPushButton("Save a linker file of all displayed content in a 3D viewer(without saving content)");
    QPushButton* btn_saveano_content = new QPushButton("Save a linker file of all displayed content in a 3D viewer(with saving content)");

    m_pLineEdit_filename = new QLineEdit();
    list_3dviewer = m_v3d.getListAll3DViewers();

    combo_surface = new MyComboBox(&m_v3d);
    combo_surface->updateList();

    label_surface = new QLabel(QObject::tr("3D View Window List: "));

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_surface, 1,0,1,5);
    gridLayout->addWidget(combo_surface, 2,0,1,5);
    gridLayout->addWidget(btn_saveano, 3,0,1,1);
    gridLayout->addWidget(btn_saveano_content, 4,0,1,1);


    setLayout(gridLayout);
    setWindowTitle(QString("Save a linker file"));

    connect(btn_saveano, SIGNAL(clicked()), this, SLOT(_slot_saveano()));
    connect(btn_saveano_content, SIGNAL(clicked()), this, SLOT(_slot_saveano_content()));

}

controlPanel::~controlPanel()
{
     if(panel){delete panel; panel=0;}

}

void controlPanel::_slot_saveano()
{
    if(combo_surface->currentIndex() <0)
    {
        v3d_msg("Please open a 3D viewer window");
        return;
    }
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, "Save Linker File",
            "linker_file.ano",
            "Linker File (*.ano)");

    if(fileName.length() == 0)
        return;

    QString curwinname = combo_surface->currentText().remove("3D View [").remove("]");
    DataLists_in_3dviewer listItem = m_v3d.fetch_3dviewer_datafilelist(curwinname);

    list_3dviewer = m_v3d.getListAll3DViewers();
    surface_win = list_3dviewer[combo_surface->currentIndex()];
    QList<NeuronTree> * mTreeList;
    mTreeList = m_v3d.getHandleNeuronTrees_Any3DViewer(surface_win);

    QStringList SWC_list = listItem.swc_file_list;
    QString imgname = listItem.imgfile;
    QString labelfieldname = listItem.labelfield_file;
    QString surfacename = listItem.surface_file;
    QStringList APO_list = listItem.pointcloud_file_list;

    ofstream anofile;
    anofile.open (fileName.toStdString().c_str(),ios::out | ios::app );

    if(SWC_list.count()>0)
    {
        for(V3DLONG i = 0; i < SWC_list.count(); i++)
        {
            anofile << "SWCFILE=" << SWC_list.at(i).toStdString().c_str() << endl;
        }

    }

    if(APO_list.count()>0)
    {
        for(V3DLONG i = 0; i < APO_list.count(); i++)
        {
            anofile << "APOFILE=" << APO_list.at(i).toStdString().c_str() << endl;
        }

    }

    if(imgname.size()>0) anofile << "RAWIMG=" << imgname.toStdString().c_str() << endl;
    if(surfacename.size()>0) anofile << "SURFILE=" << surfacename.toStdString().c_str() << endl;
    if(labelfieldname.size()>0) anofile << "SURFILE=" << labelfieldname.toStdString().c_str() << endl;

    anofile.close();

    v3d_msg(QString("The APO file is save in:[%1]").arg(fileName.toStdString().c_str()));
    return;
}

void controlPanel::_slot_saveano_content()
{
    if(combo_surface->currentIndex() <0)
    {
        v3d_msg("Please open a 3D viewer window");
        return;
    }

    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, "Save Linker File",
            "linker_file.ano",
            "Linker File (*.ano)");

    if(fileName.length() == 0)
        return;
    QString curwinname = combo_surface->currentText().remove("3D View [").remove("]");
    DataLists_in_3dviewer listItem = m_v3d.fetch_3dviewer_datafilelist(curwinname);

    list_3dviewer = m_v3d.getListAll3DViewers();
    surface_win = list_3dviewer[combo_surface->currentIndex()];
    QList<NeuronTree> * mTreeList;
    mTreeList = m_v3d.getHandleNeuronTrees_Any3DViewer(surface_win);

    QList<CellAPO> * mAPOList;
    mAPOList = m_v3d.getHandleAPOCellList_Any3DViewer(surface_win);

    QList<CellAPO> mAPOList_v2;
    for (int i = 0; i < mAPOList->size();i++)
            mAPOList_v2.push_back(mAPOList->at(i));

    QStringList SWC_list = listItem.swc_file_list;
    QString imgname = listItem.imgfile;
    QString labelfieldname = listItem.labelfield_file;
    QString surfacename = listItem.surface_file;
    QStringList APO_list = listItem.pointcloud_file_list;

    ofstream anofile;
    anofile.open (fileName.toStdString().c_str(),ios::out | ios::app );

    if(SWC_list.count()>0)
    {
        for(V3DLONG i = 0; i < SWC_list.count(); i++)
        {
            NeuronTree mTree = mTreeList->at(i);
            QString newSWCname = fileName + "_" + QFileInfo(SWC_list.at(i)).fileName();
            writeSWC_file(newSWCname,mTree);
            anofile << "SWCFILE=" << newSWCname.toStdString().c_str() << endl;
        }

    }

    if(APO_list.count()>0)
    {
            QString newAPOname = fileName + "_" + QFileInfo(APO_list.at(0)).fileName();
            writeAPO_file(newAPOname,mAPOList_v2);
            anofile << "APOFILE=" << newAPOname.toStdString().c_str() << endl;
    }

    if(imgname.size()>0) anofile << "RAWIMG=" << imgname.toStdString().c_str() << endl;
    if(surfacename.size()>0) anofile << "SURFILE=" << surfacename.toStdString().c_str() << endl;
    if(labelfieldname.size()>0) anofile << "SURFILE=" << labelfieldname.toStdString().c_str() << endl;

    anofile.close();

    v3d_msg(QString("The APO file is save in:[%1]").arg(fileName.toStdString().c_str()));
    return;
}

