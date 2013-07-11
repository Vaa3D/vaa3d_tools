/* sync3D_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-07-09 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "sync3D_plugin.h"
#include <QtGui>
#include <stdlib.h>
#include <time.h>

using namespace std;
Q_EXPORT_PLUGIN2(sync3D, sync3D);

void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent);

class lookPanel : public QDialog
	{
		Q_OBJECT
	public:
		QComboBox* combo1;
		QComboBox* combo2;
		QLabel* label1;
		QLabel* label2;
		QCheckBox* check_rotation;
		QCheckBox* check_shift;
		QCheckBox* check_zoom;
		QGridLayout *gridLayout;
		bool r,s,z;
		v3dhandleList win_list;		
		V3DPluginCallback2 &v3d;
		static lookPanel* panel;
	private slots:
		void _slot_syncAuto();
	public:

		virtual ~lookPanel()
		{
			panel = 0;
		}

		lookPanel(V3DPluginCallback2 &_v3d, QWidget *parent) : QDialog(parent),
		v3d(_v3d)
		{

			panel = this;
			win_list = v3d.getImageWindowList();
			QStringList items;
			for (int i=0; i<win_list.size(); i++) items << v3d.getImageName(win_list[i]);

			//QDialog d(parent);
			combo1 = new QComboBox(); combo1->addItems(items);
			combo2 = new QComboBox(); combo2->addItems(items);
			label1 = new QLabel(QObject::tr("Source: "));
			label2 = new QLabel(QObject::tr("Target: "));
			check_rotation = new QCheckBox(); check_rotation->setText(QObject::tr("Rotation "));check_rotation->setChecked(true);
			check_shift = new QCheckBox(); check_shift->setText(QObject::tr("Shift"));check_shift->setChecked(true);
			check_zoom = new QCheckBox(); check_zoom->setText(QObject::tr("Zoom"));check_zoom->setChecked(true);
			QPushButton* ok     = new QPushButton("Sync");
			QPushButton* cancel = new QPushButton("Close");
			QPushButton* syncAuto     = new QPushButton("Sync Auto");
			

			gridLayout = new QGridLayout();
			gridLayout->addWidget(label1, 1,0,1,6);
			gridLayout->addWidget(combo1,1,1,1,6);
			gridLayout->addWidget(label2, 2,0,1,6);
			gridLayout->addWidget(combo2,2,1,1,6);
			gridLayout->addWidget(check_rotation, 4,0,1,1);
			gridLayout->addWidget(check_shift,4,1,1,1);
			gridLayout->addWidget(check_zoom, 4,2,1,1);
			gridLayout->addWidget(ok, 5,0);
			gridLayout->addWidget(cancel,5,1);
			gridLayout->addWidget(syncAuto,5,2);
			setLayout(gridLayout);
			setWindowTitle(QString("Synchronize"));

			connect(ok,     SIGNAL(clicked()), this, SLOT(accept()));
			connect(cancel, SIGNAL(clicked()), this, SLOT(reject()));
			connect(syncAuto, SIGNAL(clicked()), this, SLOT(_slot_syncAuto()));
			connect(check_rotation, SIGNAL(stateChanged(int)), this, SLOT(update()));
			connect(check_shift, SIGNAL(stateChanged(int)), this, SLOT(update()));
			connect(check_zoom, SIGNAL(stateChanged(int)), this, SLOT(update()));
		}
		virtual void accept()
		{
			int i1 = combo1->currentIndex();
			int i2 = combo2->currentIndex();

			Image4DSimple* image1 = v3d.getImage(win_list[i1]);
			Image4DSimple* image2 = v3d.getImage(win_list[i2]);
			
			if (win_list[i1]&& win_list[i2])//ensure the 3d viewer window is open; if not, then open it
			{
			 	v3d.open3DWindow(win_list[i1]);
				View3DControl *view1 = v3d.getView3DControl(win_list[i1]);
				v3d.open3DWindow(win_list[i2]);
				View3DControl *view2 = v3d.getView3DControl(win_list[i2]);
				clock_t this_time = clock();
    				clock_t last_time = this_time;
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
							//v3d.updateImageWindow(win_list[i1]);
							v3d.updateImageWindow(win_list[i2]);
			 	}
						
				
			}
		}
		/*virtual void reject()
		{
				int i1 = combo1->currentIndex();
				int i2 = combo2->currentIndex();

				Image4DSimple* image1 = v3d.getImage(win_list[i1]);
				Image4DSimple* image2 = v3d.getImage(win_list[i2]);
				v3dhandle curwin = v3d.currentImageWindow();
				if (win_list[i1]&& win_list[i2])//ensure the 3d viewer window is open; if not, then open it
				{
				 	
					clock_t this_time = clock();
	    				clock_t last_time = this_time;
					while(true)
					{					
							double time_counter = 0;						
							this_time = clock(); 
							time_counter += (double)(this_time - last_time);
							last_time = this_time;
							if(time_counter > (double)(0.2 * CLOCKS_PER_SEC))	
							{
								time_counter -= (double)(0.2 * CLOCKS_PER_SEC);
								
								printf("time is %f\n\n\n",time_counter);	
								
							}
					}	
					
				}
		}*/	
	};

lookPanel* lookPanel::panel = 0;

V3DLONG panel(V3DPluginCallback2 &v3d, QWidget *parent)
{
	if (lookPanel::panel)
	{
		lookPanel::panel->show();
		return 1;
	}

	lookPanel* p = new lookPanel(v3d, parent);
	if (p)	p->show();
	return (V3DLONG)p;
}





//void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent);
 
QStringList sync3D::menulist() const
{
	return QStringList() 
		<<tr("synchronize")
		<<tr("about");
}

QStringList sync3D::funclist() const
{
	return QStringList()
		<<tr("synchronize")
		<<tr("help");
}

void sync3D::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("synchronize"))
	{
		//SynTwoImage(callback, parent);
		SynTwoImage(callback, parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi Zhou, 2013-07-09"));
	}
}

bool sync3D::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("synchronize"))
	{
	}
	else if (func_name == tr("help"))
	{
	}
	else return false;
}

void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent)
{
	v3dhandleList win_list = v3d.getImageWindowList();
	if (win_list.size()<1)
	{
		QMessageBox::information(0, "Sync3D",QObject::tr("Need at least 1 images."));
		return;
	}

	panel(v3d, parent);
}

void lookPanel::_slot_syncAuto()
{
	printf("this is zhi zhou\n\n\n\n\n");
}

