/* neuron_tile_display_plugin.cpp
 * This Plugin will tile neuron to display
 * 2014-10-28 : by Hanbo Chen
 */
 
#include "v3d_message.h"
#include <vector>
#include "neuron_tile_display_plugin.h"

using namespace std;
Q_EXPORT_PLUGIN2(neuron_tile_display, neuron_tile_display);
 
QStringList neuron_tile_display::menulist() const
{
	return QStringList() 
		<<tr("neuron_tile_display")
        <<tr("neuron_tile_XY")
        <<tr("tile_multi_windows (seletced swc files)")
        <<tr("tile_multi_windows (swc files under one folder)")
		<<tr("about");
}

QStringList neuron_tile_display::funclist() const
{
	return QStringList()
		<<tr("help");
}

QStringList importFileList_addnumbersort(const QString & curFilePath, int method_code)
{
    QStringList myList;
    myList.clear();

    // get the image files namelist in the directory
    QStringList imgSuffix;
    if (method_code ==1)
        imgSuffix<<"*.swc"<<"*.eswc"<<"*.SWC"<<"*.ESWC";
    else if (method_code ==2)
        imgSuffix<<"*.marker";

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
void neuron_tile_display::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("neuron_tile_display"))
	{
        dotile(callback, parent);
	}
    else if (menu_name == tr("neuron_tile_XY"))
    {
        doxytile(callback, parent);
    }
    else if (menu_name == tr("tile_multi_windows (swc files selected)"))
    {
        QStringList swcList;
        swcList = QFileDialog::getOpenFileNames(0, QObject::tr("Open File"),
                "",
                QObject::tr("Supported file (*.swc *.eswc)"
                    ";;Neuron structure	(*.swc)"
                    ";;Extended neuron structure (*.eswc)"
                    ));
        if(swcList.isEmpty())
            return;

        bool ok1, ok2, ok3, ok4;
        unsigned int col=1, row=1, xRez=1, yRez=1;
        col = QInputDialog::getInteger(parent, "",
                                      "number of displayed windows per row:",
                                      1, 1, 1000, 1, &ok1);

        if(ok1)
        {
            row = QInputDialog::getInteger(parent, "",
                                          "number of displayed windows per column:",
                                          1, 1, 1000, 1, &ok2);
        }
        else
            return;

        if(ok2)
        {
            xRez = QInputDialog::getInteger(parent, " ",
                                          "offset X:",
                                          1, 1, 10000, 1, &ok3);
        }
        else
            return;

        if(ok3)
        {
            yRez = QInputDialog::getInteger(parent, " ",
                                          "offset Y:",
                                          1, 1, 10000, 1, &ok4);
        }
        else
            return;


        V3dR_MainWindow * new3DWindow = NULL;
        int offsety = -1;
        for(V3DLONG i = 0; i < swcList.size(); i++)
        {
            QString curPathSWC = swcList.at(i);
            new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
            if( (i%col)*xRez ==0)
                offsety++;
            callback.moveWindow(new3DWindow,(i%col)*xRez,offsety*yRez);
            callback.resizeWindow(new3DWindow,xRez,yRez);
//            v3d_msg(QString("offsetx: %1, offsety:%2").arg((i%col)*xRez).arg(offsety*yRez));
        }

    }
    else if (menu_name == tr("tile_multi_windows (swc files under one folder)"))
    {
        QString m_InputfolderName = QFileDialog::getExistingDirectory(parent, QObject::tr("Choose the directory including all swc files "),
                                                                      QDir::currentPath(),
                                                                      QFileDialog::ShowDirsOnly);


        bool ok1, ok2, ok3, ok4;
        unsigned int col=1, row=1, xRez=1, yRez=1;
        col = QInputDialog::getInteger(parent, "",
                                      "number of displayed windows per row:",
                                      1, 1, 1000, 1, &ok1);

        if(ok1)
        {
            row = QInputDialog::getInteger(parent, "",
                                          "number of displayed windows per column:",
                                          1, 1, 1000, 1, &ok2);
        }
        else
            return;

        if(ok2)
        {
            xRez = QInputDialog::getInteger(parent, " ",
                                          "offset X:",
                                          1, 1, 10000, 1, &ok3);
        }
        else
            return;

        if(ok3)
        {
            yRez = QInputDialog::getInteger(parent, " ",
                                          "offset Y:",
                                          1, 1, 10000, 1, &ok4);
        }
        else
            return;
        QStringList swcList = importFileList_addnumbersort(m_InputfolderName, 1);

        V3dR_MainWindow * new3DWindow = NULL;
        int offsety = -1;
        for(V3DLONG i = 0; i < swcList.size(); i++)
        {

            QString curPathSWC = swcList.at(i);
            new3DWindow = callback.open3DViewerForSingleSurfaceFile(curPathSWC);
            if( (i%col)*xRez ==0)
                offsety++;
            callback.moveWindow(new3DWindow,(i%col)*xRez,offsety*yRez);
            callback.resizeWindow(new3DWindow,xRez,yRez);
        }

    }
	else
	{
		v3d_msg(tr("This Plugin will tile neuron to display. "
			"Developed by Hanbo Chen, 2014-10-28"));
	}
}

bool neuron_tile_display::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void neuron_tile_display::dotile(V3DPluginCallback2 &callback, QWidget *parent)
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
        if(ntTreeList->size()>1)
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

    NeuronTileDisplayDialog * myDialog = NULL;
    myDialog = new NeuronTileDisplayDialog(&callback, v3dwin);
    myDialog->show();
}

void neuron_tile_display::doxytile(V3DPluginCallback2 &callback, QWidget *parent)
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
        if(ntTreeList->size()>1)
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

    NeuronXYTileDialog * myDialog = NULL;
    myDialog = new NeuronXYTileDialog(&callback, v3dwin);
    myDialog->show();
}
