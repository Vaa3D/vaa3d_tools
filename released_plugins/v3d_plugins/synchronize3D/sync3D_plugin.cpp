/* sync3D_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-07-09 : by Zhi Zhou
 */

#include "sync3D_plugin.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <iostream>  

Q_EXPORT_PLUGIN2(sync3D, sync3D)

void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent);
lookPanel* lookPanel::panel = 0;

int getTime() {  
  return clock()/CLOCKS_PER_SEC;  
} 

//void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent);
 
QStringList sync3D::menulist() const
{
	return QStringList() 
		<<tr("synchronize")
		<<tr("about");
}


void sync3D::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("synchronize"))
	{

		SynTwoImage(callback, parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-07-09"));
	}
}


void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent)
{
	v3dhandleList win_list = v3d.getImageWindowList();
	if (win_list.size()<1)
	{
		QMessageBox::information(0, "Sync3D",QObject::tr("Need at least 1 images."));
		return;
	}

	if (lookPanel::panel)
	{
		lookPanel::panel->show();
		return;
	}

	lookPanel* p = new lookPanel(v3d, parent);
	if (p)	p->show();

}

lookPanel::lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) : 
	QDialog(parent),m_v3d(_v3d)
{

			panel = this;
			win_list = m_v3d.getImageWindowList();
			QStringList items;
			for (int i=0; i<win_list.size(); i++) items << m_v3d.getImageName(win_list[i]);

			//QDialog d(parent);
			combo1 = new QComboBox(); combo1->addItems(items);
			combo2 = new QComboBox(); combo2->addItems(items);
			label1 = new QLabel(QObject::tr("Source: "));
			label2 = new QLabel(QObject::tr("Target: "));
			check_rotation = new QCheckBox(); check_rotation->setText(QObject::tr("Rotation "));check_rotation->setChecked(true);
			check_shift = new QCheckBox(); check_shift->setText(QObject::tr("Shift"));check_shift->setChecked(true);
			check_zoom = new QCheckBox(); check_zoom->setText(QObject::tr("Zoom"));check_zoom->setChecked(true);
			QPushButton* ok     = new QPushButton("Sync(one shot)");
			QPushButton* cancel = new QPushButton("Close");
			QPushButton* syncAuto     = new QPushButton("Start Sync (real time)");
			

			gridLayout = new QGridLayout();
			gridLayout->addWidget(label1, 1,0,1,6);
			gridLayout->addWidget(combo1,1,1,1,6);
			gridLayout->addWidget(label2, 2,0,1,6);
			gridLayout->addWidget(combo2,2,1,1,6);
			gridLayout->addWidget(check_rotation, 4,0,1,1);
			gridLayout->addWidget(check_shift,4,1,1,1);
			gridLayout->addWidget(check_zoom, 4,2,1,1);
			gridLayout->addWidget(ok, 5,0);
			gridLayout->addWidget(cancel,5,6);
			gridLayout->addWidget(syncAuto,5,1);
			setLayout(gridLayout);
			setWindowTitle(QString("Synchronize"));

			connect(ok,     SIGNAL(clicked()), this, SLOT(_slot_sync()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
			connect(syncAuto, SIGNAL(clicked()), this, SLOT(_slot_syncAuto()));
			connect(check_rotation, SIGNAL(stateChanged(int)), this, SLOT(update()));
			connect(check_shift, SIGNAL(stateChanged(int)), this, SLOT(update()));
			connect(check_zoom, SIGNAL(stateChanged(int)), this, SLOT(update()));
			
			m_pTimer = new QTimer(this);
			connect(m_pTimer, SIGNAL(timeout()), this, SLOT(_slot_timerupdate()));
}

lookPanel::~lookPanel()
{
	panel = 0;
}


void lookPanel::_slot_sync()
{
			int i1 = combo1->currentIndex();
			int i2 = combo2->currentIndex();
			
			if (win_list[i1]&& win_list[i2])//ensure the 3d viewer window is open; if not, then open it
			{
			 	m_v3d.open3DWindow(win_list[i1]);
				View3DControl *view1 = m_v3d.getView3DControl(win_list[i1]);
				m_v3d.open3DWindow(win_list[i2]);
				View3DControl *view2 = m_v3d.getView3DControl(win_list[i2]);
                		if (view1 && view2)
				{  
				
							r = (check_rotation->isChecked()) ? true : false;
							s = (check_shift->isChecked()) ? true : false;
							z = (check_zoom->isChecked()) ? true : false;
					
							view1->absoluteRotPose();
							int xRot = view1->xRot();
							int yRot = view1->yRot();
							int zRot = view1->zRot();

							int xShift = view1->xShift();
							int yShift = view1->yShift();
							int zShift = view1->zShift();

							int zoom = view1->zoom();
								
							if (r == true)
							{
								view2->resetRotation();
								view2->doAbsoluteRot(xRot,yRot,zRot);
							}
							if (s == true)
							{
								view2->setXShift(xShift);
								view2->setYShift(yShift);
								view2->setZShift(zShift);
							}
							if (z == true) view2->setZoom(zoom);
					
							//view2->resetZoomShift();
							//m_v3d.updateImageWindow(win_list[i1]);
							m_v3d.updateImageWindow(win_list[i2]);
			 	}
						
				
			}



}
void lookPanel::_slot_syncAuto()
{
	long interval = 0.2 * 1000;
	m_pTimer->start(interval);
	int i1 = combo1->currentIndex();
	int i2 = combo2->currentIndex();

	m_v3d.open3DWindow(win_list[i1]);
	m_v3d.open3DWindow(win_list[i2]);	
}

void lookPanel::_slot_timerupdate()
{
				int i1 = combo1->currentIndex();
				int i2 = combo2->currentIndex();
				View3DControl *view1 = m_v3d.getView3DControl(win_list[i1]);
				View3DControl *view2 = m_v3d.getView3DControl(win_list[i2]);
                		if (view1 && view2)
				{  
				
							r = (check_rotation->isChecked()) ? true : false;
							s = (check_shift->isChecked()) ? true : false;
							z = (check_zoom->isChecked()) ? true : false;
					
							view1->absoluteRotPose();
							int xRot = view1->xRot();
							int yRot = view1->yRot();
							int zRot = view1->zRot();

							int xShift = view1->xShift();
							int yShift = view1->yShift();
							int zShift = view1->zShift();

							int zoom = view1->zoom();
								
							if (r == true)
							{
								view2->resetRotation();
								view2->doAbsoluteRot(xRot,yRot,zRot);
							}
							if (s == true)
							{
								view2->setXShift(xShift);
								view2->setYShift(yShift);
								view2->setZShift(zShift);
							}
							if (z == true) view2->setZoom(zoom);
					
							//view2->resetZoomShift();
							//m_v3d.updateImageWindow(win_list[i1]);
							m_v3d.updateImageWindow(win_list[i2]);
			 	}
						
				
		

}

