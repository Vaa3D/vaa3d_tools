/* sync_z_cut_plugin.h
 * This plugin syncs the z-cut of the image and the surface.
 * 2014-07-03 : by YourName
 */
 
#ifndef __SYNC_Z_CUT_PLUGIN_H__
#define __SYNC_Z_CUT_PLUGIN_H__

#ifdef Q_WS_X11
#define QEVENT_KEY_PRESS 6 //for crazy RedHat error: expected unqualified-id before numeric constant
#else
#define QEVENT_KEY_PRESS QEvent::KeyPress
#endif

#include <QtGui>
#include <v3d_interface.h>

//#include "v3dr_common.h"

//#include "qtr_widget.h"

//class V3dR_GLWidget;

class SyncZ : public QObject, public V3DPluginInterface2_1
{
	Q_OBJECT
    Q_INTERFACES(V3DPluginInterface2_1)

public:

	QStringList menulist() const;
	void domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent);

    QStringList funclist() const {return QStringList();}
    bool dofunc(const QString &func_name, const V3DPluginArgList &input, V3DPluginArgList &output, V3DPluginCallback2 &callback, QWidget *parent)
    {return false;}

    float getPluginVersion() const {return 1.1f;}

    //virtual void setZCutLock(bool b)=0;

    //virtual QList <NeuronTree> * getHandleNeuronTrees_Any3DViewer(V3dR_MainWindow *w) = 0;

};

typedef QString (* FUNC_altTip) (QWidget* parent, int v, int minv, int maxv, int offset);

class SliderTipFilter : public QObject
{
    //Q_OBJECT
public:
    SliderTipFilter(QObject* parent, QString prefix="", QString surffix="", int offset=0, FUNC_altTip func_alt_tip=0)
        : QObject (parent)
    {
        w = (QWidget*)parent;
        this->prefix = prefix;
        this->surffix = surffix;
        this->offset = offset;
        this->func_alt_tip = func_alt_tip;
    }
protected:
    QWidget* w;
    QString prefix, surffix;
    int offset;
    FUNC_altTip func_alt_tip;
    bool eventFilter(QObject *obj, QEvent *e)
    {
        QAbstractSlider* slider = (QAbstractSlider*)obj;
        if (slider)	{
            slider->setAttribute(Qt::WA_Hover); // this control the QEvent::ToolTip and QEvent::HoverMove
            slider->setFocusPolicy(Qt::WheelFocus); // accept KeyPressEvent when mouse wheel move
        }

        bool event_tip = false;
        QPoint pos(0,0);
        switch (e->type())
        {
        case QEvent::ToolTip: // must turned on by setAttribute(Qt::WA_Hover) under Mac 64bit
//			qDebug("QEvent::ToolTip in SliderTipFilter");
            pos = ((QHelpEvent*)e)->pos(); //globalPos();
            event_tip = true;
            break;
//		case QEvent::HoverMove:
//			qDebug("QEvent::HoverMove in SliderTipFilter");
//			pos = ((QHoverEvent*)e)->pos();
//			event_tip = true;
//			break;
        case QEvent::MouseMove: // for mouse dragging
//			qDebug("QEvent::MouseMove in SliderTipFilter");
            pos = ((QMouseEvent*)e)->pos();
            event_tip = true;
            break;
        case QEVENT_KEY_PRESS: //QEvent::KeyPress: // for arrow key dragging
//			qDebug("QEvent::KeyPress in SliderTipFilter");
            if (slider) pos = slider->mapFromGlobal(slider->cursor().pos());
            event_tip = true;
            break;
        }
        if (event_tip && slider)
        {
            QPoint gpos = slider->mapToGlobal(pos);
            QString tip = QString(prefix + "%1(%2~%3)" + surffix)
                            .arg(slider->value()+offset).arg(slider->minimum()+offset).arg(slider->maximum()+offset);
            if (func_alt_tip)
            {
                QString alt_tip = func_alt_tip(w, slider->value(), slider->minimum(), slider->maximum(), offset);
                tip += alt_tip;
            }
            QToolTip::showText(gpos, (tip), slider);
        }

        return QObject::eventFilter(obj, e);
    }
};

// /**
class MyComboBox : public QComboBox
{
    Q_OBJECT

public:
    V3DPluginCallback2 * m_v3d;
    MyComboBox(V3DPluginCallback2 * ini_v3d) {m_v3d = ini_v3d;}

    void enterEvent(QEvent * event);
// /**
public slots:
    void updateList();
    //**/
};
// **/

class lookPanel: public QDialog
{
    Q_OBJECT

public:
    lookPanel(V3DPluginCallback2 &v3d, QWidget *parent);
    ~lookPanel();

public:

    QAbstractSlider *createCutPlaneSlider(int maxval, Qt::Orientation hv = Qt::Horizontal);
    MyComboBox* combo_surface;

    QList <V3dR_MainWindow *> list_3dviewer;
    V3dR_MainWindow *surface_win;
    v3dhandleList list_triview;
    View3DControl *view;
    v3dhandle curwin;


    QString Cut_altTip(int dim_i, int v, int minv, int maxv, int offset);
    /**
    void initVolumeCutRange();  // called by initControlValue
    void initSurfaceCutRange();
    **/

    //QSpinBox* box_ZCut_Min;
    //QSpinBox* box_ZCut_Max;
    //QListWidget *list_anchors;
    //QListWidget *list_min_max;
    QComboBox* combo_master; //a usable drop down menu
    //QComboBox* new_combo_master;
    QLabel* label_master;
    QLabel* label_display;
    //QCheckBox* check_zed; //take this out later; unnecessary
    QGridLayout *gridLayout;
    v3dhandleList win_list;
    v3dhandleList win_list_past;
    V3DPluginCallback2 & m_v3d;
    QTimer *m_pTimer;

   // View3DControl *view_master;
    QList<NeuronTree> *nt_list;

    v3dhandleList threeD_list;

    bool b_autoON;

    QAbstractSlider *zcminSlider, *zcmaxSlider;
    //QAbstractSlider *zSminSlider, *zSmaxSlider;

    QAbstractButton *zcLock;
    QPushButton *updateSurf;

    //V3dR_GLWidget * glWidget;

    //QString Cut_altTip(int dim_i, int v, int minv, int maxv, int offset);

/**
virtual int zCut0() const {return _zCut0; }
virtual int zCut1() const { return _zCut1; }
virtual int zClip0() const { return _zClip0; }
virtual int zClip1() const { return _zClip1; }
**/

public slots:

//virtual void setZClip00(int s);
/**
virtual void setZCut0(int s);
virtual void setZCut1(int s);
virtual void setZClip0(int s);
virtual void setZClip1(int s);
virtual void setZCutLock(bool); //only for image
virtual void setZSurfCutLock(bool); //for the surface
virtual void enableZSlice(bool); //only for image
**/
    //void setZCut0(float s);
    //void setZCut1(float s);

private slots:
    //void _slot_sync_onetime();
    void reject();
    void change_z_min();
    void change_z_max();
    void setZCutLockIcon(bool);

    //void setZCutLockIcon(bool);
    void setZCutLock(bool);
    void update_traces();
    //void showZCutLock();
    //void change_zed_min(); //obviously does more than that now
    //void SWC_min_and_max();
    //void showSWC_min_and_max();

    //void initVolumeCutRange();
    //void change_zed_max();

/**
signals:
void changeEnableCut0Slider(bool);
void changeEnableCut1Slider(bool);
void changeZCut0(int s);
void changeZCut1(int s);
void changeZClip0(int s);
void changeZClip1(int s);
**/

public:
//int _zCut0, _zCut1;
//int dzCut, lockZ, lockSurfZ; //added the last one
//int _zClip0, _zClip1;

int zCut0;
int zCut1;
int zClip0;
int zClip1;
float min_num;
float max_num;
//int zcutmin;
//int zcutmax;
float dist_MIN;
float dist_MAX;
int lockZ;
int X;
int Y;
int dzCut;
int save_z_min;
int save_z_max;
V3DLONG sz2;
float sz22;
float X_rescaled;
float Y_rescaled;
//NeuronTree nt1;
//NeuronTree nt2;
//int m;
//int m2;

void init_members()
{
//_zCut0 = _zCut1 = dzCut = lockZ = lockSurfZ = _zClip0 = _zClip1 = 0; //added lockSurfZ
zCut0 = zCut1 = 0;
zClip0 = zClip1 = 0;
zcminSlider = 0;
zcmaxSlider = 0;
lockZ = 0;
zcLock = 0;
updateSurf = 0;
X = 0;
Y = 0;
dzCut = 0;
sz22 = 0;
save_z_min = 0;
save_z_max = 0;
sz2 = 0;
X_rescaled = 0;
Y_rescaled = 0;
//m = 0;
//m2 = 0;
//nt1 = 0;

//zcmaxSlider = zSminSlider = zSmaxSlider = 0;
//glWidget = 0;
}

};

#endif

