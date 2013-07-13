/* sync3D_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-07-09 : by Zhi Zhou
 */

//last edit: by PHC, 2013-07-12 have rewritten most parts of the plugin and seemingly the bug on Mac has been fixed


#include "sync3D_plugin.h"
#include <stdio.h>
#include <stdlib.h>

Q_EXPORT_PLUGIN2(sync3D, sync3D)

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

QStringList sync3D::menulist() const
{
    return QStringList()
            <<tr("synchronize 3D viewers")
           <<tr("about");
}

QString warning_msg = "Oops... The image you selected no longer exists... The file list has been refreshed now and you can try it again.";

void sync3D::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("synchronize 3D viewers"))
    {
        SynTwoImage(callback, parent);
    }
    else
    {
        v3d_msg(tr("Synchonize two 3D view windows."
                   "Developed by Zhi Zhou & Hanchuan Peng, 2013-July"));
    }
}


void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent)
{
    v3dhandleList win_list = v3d.getImageWindowList();
    if (win_list.size()<2)
    {
        v3d_msg("You need at least two opened images to synchronize their 3D views!");
        return;
    }

    if (panel)
    {
        panel->show();
        return;
    }
    else
    {
        panel = new lookPanel(v3d, parent);
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
    //panel = this;
    win_list = m_v3d.getImageWindowList();
    QStringList items;
    for (int i=0; i<win_list.size(); i++)
        items << m_v3d.getImageName(win_list[i]);

    combo_master = new QComboBox(); combo_master->addItems(items);
    combo_slave = new QComboBox(); combo_slave->addItems(items);
    label_master = new QLabel(QObject::tr("Master-window: "));
    label_slave = new QLabel(QObject::tr("Slave-window: "));
    check_rotation = new QCheckBox(); check_rotation->setText(QObject::tr("Rotation "));check_rotation->setChecked(true);
    check_shift = new QCheckBox(); check_shift->setText(QObject::tr("Shift"));check_shift->setChecked(true);
    check_zoom = new QCheckBox(); check_zoom->setText(QObject::tr("Zoom"));check_zoom->setChecked(true);
    QPushButton* ok     = new QPushButton("Sync (one shot)");
    QPushButton* cancel = new QPushButton("Close");
    syncAuto     = new QPushButton("Start Sync (real time)");

    b_autoON = false;

    gridLayout = new QGridLayout();
    gridLayout->addWidget(label_master, 1,0,1,6);
    gridLayout->addWidget(combo_master,1,1,1,6);
    gridLayout->addWidget(label_slave, 2,0,1,6);
    gridLayout->addWidget(combo_slave,2,1,1,6);
    gridLayout->addWidget(check_rotation, 4,0,1,1);
    gridLayout->addWidget(check_shift,4,1,1,1);
    gridLayout->addWidget(check_zoom, 4,2,1,1);
    gridLayout->addWidget(ok, 5,0);
    gridLayout->addWidget(cancel,5,6);
    gridLayout->addWidget(syncAuto,5,1);
    setLayout(gridLayout);
    setWindowTitle(QString("Synchronize two 3D views"));

    connect(ok,     SIGNAL(clicked()), this, SLOT(_slot_sync_onetime()));
    connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
    connect(syncAuto, SIGNAL(clicked()), this, SLOT(_slot_syncAuto()));
    connect(check_rotation, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(check_shift, SIGNAL(stateChanged(int)), this, SLOT(update()));
    connect(check_zoom, SIGNAL(stateChanged(int)), this, SLOT(update()));

    //should we check the window close event? // by PHC

    m_pTimer = new QTimer(this);
    connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
    win_list_past = win_list;

}

lookPanel::~lookPanel()
{
    if (m_pTimer) {delete m_pTimer; m_pTimer=0;}
}


void lookPanel::_slot_sync_onetime()
{
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i1 = combo_master->currentIndex();
    int i2 = combo_slave->currentIndex();

    if (i1==i2)
    {
        v3d_msg("You have selected the same image. You need to specify and sync two different images. Try again!");
        return;
    }

    if (i1 <  win_list.size() &&
            i2 < win_list.size() &&
            i1 < win_list_past.size() &&
            i2  < win_list_past.size())
    {
        QString current1 = m_v3d.getImageName(win_list[i1]);
        QString current2 = m_v3d.getImageName(win_list[i2]);
        QString past1 = m_v3d.getImageName(win_list_past[i1]);
        QString past2 = m_v3d.getImageName(win_list_past[i2]);
        if (current1==past1  && current2==past2)
        {
            if (win_list[i1] && win_list[i2])//ensure the 3d viewer window is open; if not, then open it
            {
                m_v3d.open3DWindow(win_list[i1]);
                View3DControl *view_master = m_v3d.getView3DControl(win_list[i1]);
                m_v3d.open3DWindow(win_list[i2]);
                View3DControl *view_slave = m_v3d.getView3DControl(win_list[i2]);
                if (view_master && view_slave)
                {
                    view_master->absoluteRotPose();
                    int xRot = view_master->xRot();
                    int yRot = view_master->yRot();
                    int zRot = view_master->zRot();

                    int xShift = view_master->xShift();
                    int yShift = view_master->yShift();
                    int zShift = view_master->zShift();

                    int zoom = view_master->zoom();

                    if (check_rotation->isChecked())
                    {
                        view_slave->resetRotation();
                        view_slave->doAbsoluteRot(xRot,yRot,zRot);
                    }
                    if (check_shift->isChecked())
                    {
                        view_slave->setXShift(xShift);
                        view_slave->setYShift(yShift);
                        view_slave->setZShift(zShift);
                    }
                    if (check_zoom->isChecked())
                        view_slave->setZoom(zoom);
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
            combo_slave->clear(); combo_slave->addItems(items);
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
        combo_slave->clear(); combo_slave->addItems(items);
        win_list_past = win_list;
        return;
    }
    return;
}

void lookPanel::resetSyncAutoState()
{
    if (m_pTimer)
        m_pTimer->stop();
    b_autoON = false;
    if (syncAuto)
        syncAuto->setText("Start Sync (real time)");
    if (combo_master) combo_master->setEnabled(true);
    if (combo_slave) combo_slave->setEnabled(true);
}

void lookPanel::_slot_syncAuto()
{
    v3dhandleList win_list = m_v3d.getImageWindowList();
    int i_master = combo_master->currentIndex();
    int i_slave = combo_slave->currentIndex();
    if (i_master==i_slave)
    {
        v3d_msg("You have selected the same image. You need to specify and sync two different images. Try again!");
        resetSyncAutoState();

        QStringList items;
        for (int i=0; i<win_list.size(); i++)
            items << m_v3d.getImageName(win_list[i]);
        if (combo_master)
        {
            combo_master->clear(); combo_master->addItems(items);
            combo_master->setEnabled(true);
        }

        if (combo_slave)
        {
            combo_slave->clear(); combo_slave->addItems(items);
            combo_slave->setEnabled(true);
        }

        return;
    }

    if(i_master <  win_list.size() &&
       i_slave  <  win_list.size() &&
       i_master < win_list_past.size() &&
       i_slave  < win_list_past.size())
    {
        b_autoON = !b_autoON; // a simple bi-state switch
        {
            if (b_autoON)
            {
                syncAuto->setText("Stop Sync (real time)");
                xRot_past = -1;
                yRot_past = -1;
                zRot_past = -1;
                xShift_past = -1;
                yShift_past = -1;
                zShift_past = -1;
                zoom_past = -1;

                if (!(view_master = m_v3d.getView3DControl(win_list[i_master])))
                {
                    m_v3d.open3DWindow(win_list[i_master]);
                    view_master = m_v3d.getView3DControl(win_list[i_master]);
                }

                if (!(view_slave = m_v3d.getView3DControl(win_list[i_slave])))
                {
                    m_v3d.open3DWindow(win_list[i_slave]);
                    view_slave = m_v3d.getView3DControl(win_list[i_slave]);
                }

                combo_master->setEnabled( false );
                combo_slave->setEnabled( false );

                long interval = 0.2 * 1000;
                m_pTimer->start(interval);
            }
            else
            {
                resetSyncAutoState();
                return;
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
        combo_slave->clear(); combo_slave->addItems(items);
        win_list_past = win_list;
        return;
    }
    return;
}

void lookPanel::_slot_timerupdate()
{
    int i_master = combo_master->currentIndex();
    int i_slave = combo_slave->currentIndex();
    if (i_master==i_slave)
    {
        v3d_msg("Somehow you have specified the same image for both master and slave views. Do nothing!", 0);
        resetSyncAutoState();
        return;
    }

    if (!(view_master = m_v3d.getView3DControl(win_list[i_master])))
    {
        m_v3d.open3DWindow(win_list[i_master]);
        view_master = m_v3d.getView3DControl(win_list[i_master]);
    }

    if (!(view_slave = m_v3d.getView3DControl(win_list[i_slave])))
    {
        m_v3d.open3DWindow(win_list[i_slave]);
        view_slave = m_v3d.getView3DControl(win_list[i_slave]);
    }

    if (view_master && view_slave)
    {
        if (check_rotation->isChecked())
        {
            view_master->absoluteRotPose();
            int xRot = view_master->xRot();
            int yRot = view_master->yRot();
            int zRot = view_master->zRot();

            if (xRot!=xRot_past || yRot!=yRot_past || zRot!=zRot_past)
            {
                view_slave->resetRotation();
                view_slave->doAbsoluteRot(xRot,yRot,zRot);
                xRot_past = xRot;
                yRot_past = yRot;
                zRot_past = zRot;
            }
        }

        if (check_shift->isChecked())
        {
            int xShift = view_master->xShift();
            int yShift = view_master->yShift();
            int zShift = view_master->zShift();

            if (xShift!=xShift_past || yShift!=yShift_past || zShift!=zShift_past)
            {
                view_slave->setXShift(xShift);
                view_slave->setYShift(yShift);
                view_slave->setZShift(zShift);
                xShift_past = xShift;
                yShift_past = yShift;
                zShift_past = zShift;
            }
        }

        if (check_zoom->isChecked())
        {
            int zoom = view_master->zoom();
            if (zoom!=zoom_past)
            {
                view_slave->setZoom(zoom);
                zoom_past = zoom;
            }
        }
    }
}

