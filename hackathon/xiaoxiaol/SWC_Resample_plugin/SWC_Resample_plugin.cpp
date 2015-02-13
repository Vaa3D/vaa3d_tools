/* SWC_Resample_plugin.cpp
 * This plugin allows users to resample displayed SWCs.
 * 2015-02-05 : by Xiaoxiao Liu
 */
 
#include "v3d_message.h"
#include <vector>
#include <QObject>
#include "SWC_Resample_plugin.h"
#include "../../../released_plugins/v3d_plugins/resample_swc/resampling.h"  // this resampling implementation should be in trunk
#include "../../../v3d_main/neuron_editing/neuron_xforms.h"



using namespace std;
Q_EXPORT_PLUGIN2(SWC_Resample, SWCResample);
 
static resampleDialog * mydialog = 0;


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


    QList <V3dR_MainWindow *> cur_list_3dviewer = m_v3d->getListAll3DViewers();

    QStringList items;
    int i;

    for (i=0; i<cur_list_3dviewer.count(); i++)
    {
        items << m_v3d->getImageName(cur_list_3dviewer[i]);
    }


    clear();
    addItems(items);

    int curDisplayIndex = -1; //-1 for invalid index
    for (i = 0; i < items.size(); i++)
        if (items[i] == lastDisplayfile)
        {
            curDisplayIndex = i;
            break;
        }

    if (curDisplayIndex>=0)
        setCurrentIndex(curDisplayIndex);

    update();

    return;
}

resampleDialog :: resampleDialog(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    this->combobox_win = new MyComboBox(&m_v3d); // combo box for selecting the window that contains swc objects
    this->combobox_win->updateList();

    QLabel *label_surface = new QLabel(QObject::tr("3D Viewer Window List: "));

    QLabel *label_steplength = new QLabel(QObject::tr("Fixed Step Length (distance between neighboring nodes"));

    this->spinbox_steplength = new QSpinBox;
    this->spinbox_steplength->setRange(1, INT32_MAX);
    this->spinbox_steplength->setSingleStep(1);
    this->spinbox_steplength->setValue(5);

    QPushButton *btn_Run = new QPushButton("Run");

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_surface, 1,0,1,5);
    gridLayout->addWidget(this->combobox_win, 2,0,1,5);
    gridLayout->addWidget(label_steplength,3,0,1,0);
    gridLayout->addWidget(this->spinbox_steplength, 4,0,1,0);
    gridLayout->addWidget(btn_Run, 5,0,1,0);

    setLayout(this->gridLayout);
    setWindowTitle(QString("Resample Parameters"));

    connect(btn_Run, SIGNAL(clicked()), this, SLOT(_slot_run()));
}


resampleDialog::~resampleDialog()
{
    if ( mydialog )
    {
        delete mydialog;
        mydialog = 0;
    }
}


void resampleDialog::_slot_run()
{
    int steplength = this->spinbox_steplength->value();

    //obtain the selected 3D viewer
    list_3dviewer = m_v3d.getListAll3DViewers();
    if (list_3dviewer.size() < 1)
    {
        v3d_msg("Please open up a SWC file from the main menu first!");
        return;
    }

    surface_win = list_3dviewer[combobox_win->currentIndex()];

    if (!surface_win){
        v3d_msg("Please open up a SWC file from the main menu first!");
        return;
    }

    //get the current displayed neurons in the selected 3d viewer
    QList<NeuronTree> * mTreeList = m_v3d.getHandleNeuronTrees_Any3DViewer(surface_win);


    // open up a new 3D viewer window
    QString originalTitle = m_v3d.getImageName(surface_win).remove("3D View [").remove("]");
    QString title = QString("Resampled ") +  originalTitle;


    bool b_found = false;
    V3dR_MainWindow * new3DWindow = NULL;
    for (int j= 0; j < list_3dviewer.size(); j++)
    {
        if ( m_v3d.getImageName(list_3dviewer[j]).contains(title))
        {
            b_found = true;
            new3DWindow = list_3dviewer[j];
            break;
        }
    }

    if ( ! b_found)
    {
        new3DWindow = m_v3d.createEmpty3DViewer();
        if (!new3DWindow)
        {
            v3d_msg(QString("Failed to open an empty window!"));
            return;
        }
    }


    QList<NeuronTree> * new_treeList = m_v3d.getHandleNeuronTrees_Any3DViewer (new3DWindow);
    if (!new_treeList)
    {
        v3d_msg(QString("New 3D viewer has invalid neuron tree list"));
        return;
    }

    NeuronTree myTree;
    NeuronTree resultTree;

    for (int i = 0 ; i < mTreeList->size(); i++)
    {
        myTree = mTreeList->at(i);

        resultTree = resample(myTree,double(steplength));

        // need to reset the color to zero of display with color (using the types)
        resultTree.color.r = 0;
        resultTree.color.g = 0;
        resultTree.color.b = 0;
        resultTree.color.a = 0;

        new_treeList->push_back(resultTree);
    }

    //make a copy of the original tree list and shif them for visual comparision
    for (int i = 0 ; i < mTreeList->size(); i++)
    {
        myTree = mTreeList->at(i);
        resultTree.copy(myTree);

        proc_neuron_add_offset(&resultTree, 0, 0, 100);

        // need to reset the color to zero of display with color (using the types)
        resultTree.color.r = 0;
        resultTree.color.g = 0;
        resultTree.color.b = 0;
        resultTree.color.a = 0;

        new_treeList->push_back(resultTree);
    }



    //m_v3d.pushObjectIn3DWindow(new3DWindow);  this does not work
    // in XFormWidget::pushObjectIn3DWindow, because the triView is updated?

    m_v3d.setWindowDataTitle(new3DWindow, title);

    //m_v3d.update_3DViewer(new3DWindow); // this will not update the bounding box ?, and adding this will only cuase
    //the window to delay the refreshing until user interaction

    m_v3d.update_NeuronBoundingBox(new3DWindow); //without this step, the neuron won't show...


}



QStringList SWCResample::menulist() const
{
    return QStringList()
            <<tr("Resample SWC Displayed in 3D Views")
           <<tr("about");
}

QStringList SWCResample::funclist() const
{
    return QStringList()
            <<tr("resample")
           <<tr("help");
}

void SWCResample::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Resample SWC Displayed in 3D Views"))
    {
        mydialog = new resampleDialog(callback, parent);
        mydialog->show();

        return;
    }
    else
    {
        v3d_msg(tr("This plugin allows users to resample displayed SWCs.. "
                   "Developed by Xiaoxiao Liu, 2015-02-05"));
    }
}

bool SWCResample::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("resample"))
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

