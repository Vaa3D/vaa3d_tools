/* sync_z_cut_plugin.cpp
 * This plugin syncs the z-cut of the image and the surface.
 * 2014-07-03 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "sync_z_cut_plugin.h"
#include <stdio.h>
#include <stdlib.h>

//using namespace std;

Q_EXPORT_PLUGIN2(sync_z_cut, SyncZ)

void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent);

static lookPanel *panel = 0;

void finishSyncPanel()
{
    if (panel)
    {
        delete panel;
        panel=0;
    }
}

//generic construction
QStringList SyncZ::menulist() const
{
    return QStringList()
        <<tr("sync z cut");
}

QString warning_msg = "Oops...that is not a valid z slice";

void SyncZ::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("sync z cut"))
    {
        SynTwoImage(callback, parent);
    }
    else
    {
        v3d_msg(tr("This plugin syncs the z-cut of the image and the surface.. "
            "Developed by YourName, 2014-07-03"));
    }
}

//main functions
void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent)
{
    v3dhandleList win_list = v3d.getImageWindowList();
    if (win_list.size()<1)
    {
        v3d_msg("You need one opened image to get 3D view data!");
        return;
    }

//I added this if bracket
    if (win_list.size()>1)
    {
        v3d_msg("Cannot analyze multiple windows. Close all windows but the desired one and try again.");
        return;
    }

    if (panel)
    {
        panel->show();
        return;
    }
    else
    {
        panel = new lookPanel(v3d, parent); //this shouldn't happen.

        if (panel)
        {
            panel->show();
            panel->raise();
            panel->move(100,100);
            panel->activateWindow();
        }
    }
}

void lookPanel::reject()
{
    finishSyncPanel();
}

lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) :
    QDialog(parent), m_v3d(_v3d)
{
    win_list = m_v3d.getImageWindowList();
    QStringList items;
    for (int i=0; i<win_list.size(); i++)
        items << m_v3d.getImageName(win_list[i]);

    combo_master = new QComboBox(); combo_master->addItems(items);

    label_master = new QLabel(QObject::tr("Master-window: "));

    check_zed = new QCheckBox(); check_zed->setText(QObject::tr("zcutmin"));check_zed->setChecked(true);

    QPushButton* ok     = new QPushButton("Show zed");
    QPushButton* cancel = new QPushButton("Close");
    QPushButton* update = new QPushButton("Update zed");

    box_ZCut_Min = new QSpinBox();
    box_ZCut_Max = new QSpinBox();
    QLabel* SampleName = new QLabel(QObject::tr("Change Zed Min"));
    QLabel* SampleNameTwo = new QLabel(QObject::tr("Change Zed Max"));

    b_autoON = false; //no idea what this means

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_master, 1,0,1,6);
    gridLayout->addWidget(combo_master,1,1,1,6);

    gridLayout->addWidget(check_zed, 4,2,1,1);
    gridLayout->addWidget(ok, 5,0); //needed?
    gridLayout->addWidget(cancel,5,6); //needed?
    gridLayout->addWidget(update, 5,12);

    setLayout(gridLayout);
    setWindowTitle(QString("Get zed info"));
    gridLayout->addWidget(box_ZCut_Min, 13,0,1,2);
    gridLayout->addWidget(SampleName, 12,0,1,1);
    gridLayout->addWidget(box_ZCut_Max, 13,4,1,2);
    gridLayout->addWidget(SampleNameTwo, 12,4,1,1);

    connect(ok, SIGNAL(clicked()), this, SLOT(_slot_sync_onetime())); //see zcutmin
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(update, SIGNAL(clicked()), this, SLOT(change_zed_min()));
    connect(check_zed, SIGNAL(stateChanged(int)), this, SLOT(update()));

        box_ZCut_Min->setMaximum(100);
        box_ZCut_Min->setMinimum(0);
        box_ZCut_Min->setValue(0);

        box_ZCut_Max->setMaximum(100);
        box_ZCut_Max->setMinimum(0);
        box_ZCut_Max->setValue(100);

    connect(box_ZCut_Min, SIGNAL(valueChanged(double)), this, SLOT(update()));
    connect(box_ZCut_Max, SIGNAL(valueChanged(double)), this, SLOT(update()));

    //should we check the window close event? // by PHC

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
    win_list_past = win_list; //no idea what this is

list_anchors = new QListWidget();
    gridLayout->addWidget(list_anchors,6,0,5,5);

}

lookPanel::~lookPanel()
{
    if (m_pTimer) {delete m_pTimer; m_pTimer=0;}
}

void lookPanel::change_zed_min()
{
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i1 = combo_master->currentIndex();

    if (i1 <  win_list.size() &&
            i1 < win_list_past.size() )
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);
        if (current1==past1)
        {
           if (win_list[i1])//ensure the 3d viewer window is open; if not, then open it (took out  && win_list[i2])
           {
                m_v3d.open3DWindow(win_list[i1]);
                View3DControl *view_master = m_v3d.getView3DControl(win_list[i1]);

                if (view_master)
                {
                    view_master->absoluteRotPose();

                    int  N = box_ZCut_Min->text().toInt();
                    view_master->setZCut0(N);

                    int L = box_ZCut_Max->text().toInt();
                    view_master->setZCut1(L); //added

                }
           }
        }
        else
        {
            v3d_msg(warning_msg);
            QStringList items;
            for (int i=0; i<win_list.size(); i++)
                items << m_v3d.getImageName(win_list[i]);
            combo_master->clear(); combo_master->addItems(items);
            win_list_past = win_list;
            return;

        }
    }
    else
    {
        v3d_msg(warning_msg);
        QStringList items;
        for (int i=0; i<win_list.size(); i++)
            items << m_v3d.getImageName(win_list[i]);
        combo_master->clear(); combo_master->addItems(items);
        win_list_past = win_list;
        return;
    }
    return;
}

void lookPanel::_slot_sync_onetime() //significantly changed from previous, took out syncing functions
{
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i1 = combo_master->currentIndex();

    if (i1 <  win_list.size() &&
            i1 < win_list_past.size() )
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);

        if (current1==past1)
        {
           if (win_list[i1])//ensure the 3d viewer window is open; if not, then open it
           {
                m_v3d.open3DWindow(win_list[i1]);
                View3DControl *view_master = m_v3d.getView3DControl(win_list[i1]);

                if (view_master)
                {
                    view_master->absoluteRotPose();

                    int zcutmin = view_master->zCut0(); //maybe take this as the parameter
                    int zcutmax = view_master->zCut1();

                    if (check_zed->isChecked())
                    {
                        QString curstr = QString("%1,%2").arg(zcutmin).arg(zcutmax);
                                    curstr = curstr.prepend(QString("").setNum(list_anchors->count()+1) + ": [ ");
                                    curstr = curstr.append(" ]");
                         list_anchors->addItem(new QListWidgetItem(curstr));
                        return;
                    }

                }
           }
        }
        else
        {
            v3d_msg(warning_msg);
            QStringList items;
            for (int i=0; i<win_list.size(); i++)
                items << m_v3d.getImageName(win_list[i]);
            combo_master->clear(); combo_master->addItems(items);
            win_list_past = win_list;
            return;

        }
    }
    else
    {
        v3d_msg(warning_msg);
        QStringList items;
        for (int i=0; i<win_list.size(); i++)
            items << m_v3d.getImageName(win_list[i]);
        combo_master->clear(); combo_master->addItems(items);
        win_list_past = win_list;
        return;
    }
    return;
}

//code from interface
/**
void V3dR_GLWidget::setRenderMode_Mip(bool b, bool useMin)
{
    //qDebug("V3dR_GLWidget::setRenderMode_Mip = %i",b);
    if (b) {
        if (!useMin) {	//max IP
            _renderMode = int(Renderer::rmMaxIntensityProjection);
            if (renderer) renderer->setRenderMode(Renderer::rmMaxIntensityProjection);
        } else {
            //mIP
            _renderMode = int(Renderer::rmMinIntensityProjection);
            if (renderer) renderer->setRenderMode(Renderer::rmMinIntensityProjection);
        }
        // restore renderer->Cut0
        if (renderer) renderer->setXCut0(_xCut0);
        if (renderer) renderer->setYCut0(_yCut0);
        if (renderer) renderer->setZCut0(_zCut0);
        POST_updateGL();
    }

    if (!useMin)
        emit changeDispType_maxip(b);
    else
        emit changeDispType_minip(b);

    emit changeTransparentSliderLabel("Threshold");
    emit changeEnableCut0Slider(b);
    emit changeEnableCut1Slider( !b);
    if (b) emit changeCurrentTabCutPlane(0);
    emit changeEnableTabCutPlane(0, b);
    emit changeEnableTabCutPlane(1, !b);
}


void V3dR_GLWidget::setRenderMode_Alpha(bool b)
{
    //qDebug("V3dR_GLWidget::setRenderMode_Alpha = %i",b);
    if (b) {
        _renderMode = int(Renderer::rmAlphaBlendingProjection);
        if (renderer) renderer->setRenderMode(Renderer::rmAlphaBlendingProjection);
        // restore renderer->Cut0
        if (renderer) renderer->setXCut0(_xCut0);
        if (renderer) renderer->setYCut0(_yCut0);
        if (renderer) renderer->setZCut0(_zCut0);
        POST_updateGL();
    }
    emit changeDispType_alpha(b);

    emit changeTransparentSliderLabel("Threshold");
    emit changeEnableCut0Slider(b);
    emit changeEnableCut1Slider( !b);
    if (b) emit changeCurrentTabCutPlane(0);
    emit changeEnableTabCutPlane(0, b);
    emit changeEnableTabCutPlane(1, !b);
}

void V3dR_GLWidget::setRenderMode_Cs3d(bool b)
{
    //qDebug("V3dR_GLWidget::setRenderMode_Cs3d = %i",b);
    if (b) {
        _renderMode = int(Renderer::rmCrossSection);
        if (renderer) renderer->setRenderMode(Renderer::rmCrossSection);
        // using widget->Cut1 to control renderer->Cut0
//		if (renderer) renderer->setXCut0(_xCut1);
//		if (renderer) renderer->setYCut0(_yCut1);
//		if (renderer) renderer->setZCut0(_zCut1);
        if (renderer) renderer->setXCut0(_xCS);
        if (renderer) renderer->setYCut0(_yCS);
        if (renderer) renderer->setZCut0(_zCS);
        POST_updateGL();
    }
    emit changeDispType_cs3d(b);

    emit changeTransparentSliderLabel("Transparency");
    emit changeEnableCut0Slider( !b);
    emit changeEnableCut1Slider(b);
    if (b) emit changeCurrentTabCutPlane(1);
    emit changeEnableTabCutPlane(0, !b);
    emit changeEnableTabCutPlane(1, b);
}

void V3dR_GLWidget::enableZSlice(bool s)
{
    if (renderer)
    {
        renderer->bZSlice = (s);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZCut0(int s)
{
    if (_zCut0 != s) {
        int DZ = MAX(0, dataDim3()-1);
        if (s+dzCut>DZ)  s = DZ-dzCut;

        _zCut0 = s;
        if (renderer) renderer->setZCut0(s);

        if (_zCut0+dzCut>_zCut1)	setZCut1(_zCut0+dzCut);
        if (lockZ && _zCut0+dzCut<_zCut1)	setZCut1(_zCut0+dzCut);
        emit changeZCut0(_zCut0);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZCut1(int s)
{
    if (_zCut1 != s) {
        if (s-dzCut<0)  s = dzCut;

        _zCut1 = s;
        if (renderer) renderer->setZCut1(s);

        if (_zCut0>_zCut1-dzCut)	setZCut0(_zCut1-dzCut);
        if (lockZ && _zCut0<_zCut1-dzCut)	setZCut0(_zCut1-dzCut);
        emit changeZCut1(_zCut1);
        POST_updateGL();
    }
}

void V3dR_GLWidget::setZCutLock(bool b)
{
    if (b)	dzCut = _zCut1-_zCut0;
    else    dzCut = 0;
    lockZ = b? 1:0;  //110714
}

void V3dR_GLWidget::setZClip0(int s)
{
    if (_zClip0 != s) {
        _zClip0 = s;
        if (renderer) renderer->setZClip0(s/(float)CLIP_RANGE);

        if (_zClip0>_zClip1)	setZClip1(_zClip0); //081031
        emit changeZClip0(s);
        POST_updateGL();
    }
}
void V3dR_GLWidget::setZClip1(int s)
{
    if (_zClip1 != s) {
        _zClip1 = s;
        if (renderer) renderer->setZClip1(s/(float)CLIP_RANGE);

        if (_zClip0>_zClip1)	setZClip0(_zClip1); //081031
        emit changeZClip1(s);
        POST_updateGL();
    }
}
**/

//generic

/**
bool SyncZ::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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
**/

