/* mostVesselTracer.cpp
 * the plugin for most vessel tracing
 * by jpwu@CBMP, 20100419
 */


#include "mostVesselTracer.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(mostVesselTracer, mostVesselTracerPlugin);

class lookPanel : public QDialog
{
public:
    QSpinBox* box1;
    QSpinBox* box2;
    QSpinBox* box3;
    V3DPluginCallback2 &v3d;
    static lookPanel* panel;

    virtual ~lookPanel()
    {
        panel = 0;
    }
    lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) : QDialog(parent),
    v3d(_v3d)
    {
        panel = this;

        box1 = new QSpinBox(); box1->setRange(-100,100);
        box2 = new QSpinBox(); box2->setRange(-100,100);
        box3 = new QSpinBox(); box3->setRange(-100,100);
        QPushButton* ok     = new QPushButton("OK");
        QPushButton* cancel = new QPushButton("Cancel");
        QFormLayout *formLayout = new QFormLayout;
        formLayout->addRow(QObject::tr("look along X: "), box1);
        formLayout->addRow(QObject::tr("look along Y: "), box2);
        formLayout->addRow(QObject::tr("look along Z: "), box3);
        formLayout->addRow(ok, cancel);

        //QDialog d(parent);
        setLayout(formLayout);
        setWindowTitle(QString("look along vector"));

        connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
    }
    virtual void accept()
    {
        int i1 = box1->value();
        int i2 = box2->value();
        int i3 = box3->value();

        v3dhandle curwin = v3d.currentImageWindow();
        if (curwin)//ensure the 3d viewer window is open; if not, then open it
        {
            v3d.open3DWindow(curwin);

            View3DControl *view = v3d.getView3DControl(curwin);
            if (view)  view->lookAlong(i1,i2,i3);

            v3d.updateImageWindow(curwin);
        }
    }
};

lookPanel* lookPanel::panel = 0;

V3DLONG panel(V3DPluginCallback2 &v3d, QWidget *parent)
{
    if (lookPanel::panel)
    {
        lookPanel::panel->show();
        return -1;
    }

    lookPanel* p = new lookPanel(v3d, parent);
    if (p)	p->show();
    return (V3DLONG)p;
}

//plugin funcs
const QString title = "Trace vessels of MOST volume!";
QStringList mostVesselTracerPlugin::menulist() const
{
    return QStringList()
            << tr("Set Seeds")
            << tr("Start tracing")
            << tr("About");
}

void mostVesselTracerPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("Set Seeds"))
    {
        setSeeds(v3d, parent);
    }
    else if (menu_name == tr("Start tracing"))
    {
        startVesselTracing( v3d , parent );
    }
    else
    {
        QString msg = QString("MOST Vessel Tracing Plugin version %1\ndeveloped by jpwu@CBMP")
                      .arg(getPluginVersion(), 1, 'f', 1);
        QMessageBox::information(parent, "Version info", msg);
    }
}

//lookPanel* panel(V3DPluginCallback2 &v3d, QWidget *parent);


void setSeeds(V3DPluginCallback2 &v3d, QWidget *parent )
{
    v3dhandle curwin = v3d.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }

    //ensure the 3d viewer window is open; if not, then open it
    v3d.open3DWindow(curwin);
}

void startVesselTracing ( V3DPluginCallback2 &v3d, QWidget *parent )
{
    v3dhandle curwin = v3d.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }
    //ensure the 3d viewer window is open; if not, then open it
    v3d.open3DWindow(curwin);

    // get land mark list
    LandmarkList seedList = v3d.getLandmark(curwin);

    if ( seedList.isEmpty() )
    {
        LocationSimple seed;
        seed.x = 68;//135;//131;//388;
        seed.y = 102;//131;//60;//370;
        seed.z = 68;//79;//295;//247;
        seed.radius = 10;
        seedList << seed;

//        v3d_msg("You don't have any seed marker to start tracing !");
//        return;
    }

    // tracing in most image
    Image4DSimple* oldimg = v3d.getImage(curwin);

    MOSTImage img;
    // set data
    img.setData( (unsigned char*)oldimg->getRawData(), oldimg->getXDim(),oldimg->getYDim(),oldimg->getZDim(),oldimg->getCDim(),oldimg->getDatatype());

    // clear visited, only excute once
    static int init_flag = 0;
    if ( init_flag <= 0 )
    {
        visited.fill( false, oldimg->getTotalUnitNumber());
        init_flag ++;
    }

    // converte the formate
    NeuronTree vt;
    vt = img.trace_seed_list(seedList, visited);

//    NeuronTree vt_old = v3d.getSWC(curwin);

    // visualization
    v3d.setLandmark(curwin, seedList);
    v3d.setSWC(curwin,vt);
    v3d.pushObjectIn3DWindow(curwin);
    v3d.updateImageWindow(curwin);
}
