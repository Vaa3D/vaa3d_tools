/* saveToanoPlugin_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-01-13 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "saveToanoPlugin_plugin.h"
#include <iostream>
#include <fstream>



using namespace std;
Q_EXPORT_PLUGIN2(saveToanoPlugin, saveToanoPlugin);

void generatorAno43Dviewer(V3DPluginCallback2 &callback, QWidget *parent);
controlPanel* controlPanel::panel = 0;

 
QStringList saveToanoPlugin::menulist() const
{
	return QStringList() 
        <<tr("save all SWC, APO, and Surface files to an ANO file")
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
    QPushButton* btn_saveano = new QPushButton("Save a linker file of all displayed content in a 3D viewer");
    m_pLineEdit_filename = new QLineEdit();
    list_3dviewer = m_v3d.getListAll3DViewers();

    combo_surface = new MyComboBox(&m_v3d);
    combo_surface->updateList();

    label_surface = new QLabel(QObject::tr("3D View Window List: "));

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_surface, 1,0,1,5);
    gridLayout->addWidget(combo_surface, 2,0,1,5);
    gridLayout->addWidget(btn_saveano, 3,0,1,1);

    setLayout(gridLayout);
    setWindowTitle(QString("Save a linker file"));

    connect(btn_saveano, SIGNAL(clicked()), this, SLOT(_slot_saveano()));
}

controlPanel::~controlPanel()
{
     if(panel){delete panel; panel=0;}

}

void controlPanel::_slot_saveano()
{
    QString fileName;
    fileName = QFileDialog::getSaveFileName(this, "Save Linker File",
            "linker_file.ano",
            "Linker File (*.ano)");

    if(!fileName.isEmpty())
    {
        return;
    }

    DataLists_in_3dviewer listItem = m_v3d.fetch_3dviewer_datafilelist(combo_surface->currentText());



    QStringList SWC_list = listItem.swc_file_list;
    QString imgname = listItem.imgfile;
    QString surfacename = listItem.surface_file;
    QStringList APO_list = listItem.pointcloud_file_list;

    v3d_msg(imgname);
    printf("size is %d\n",SWC_list.size());
    ofstream anofile;
    anofile.open (fileName.toStdString().c_str(),ios::out | ios::app );

    if(SWC_list.size()>0)
    {
        for(V3DLONG i = 0; i < SWC_list.size(); i++)
        {
            anofile << "SWCFILE=" << SWC_list.at(i).toStdString().c_str() << endl;
        }

    }

    anofile.close();
    return;
}


