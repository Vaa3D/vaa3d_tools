/* ex_push.cpp
 * an example program to test the push function in the plugin interface
 * 2010-08-3: by Hanchuan Peng
 */


#include "ex_push.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(ex_push, ExPushPlugin);

void dopush(V3DPluginCallback2 &v3d, QWidget *parent, int method_code);

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
const QString title = "Example for pushing image and objects";
QStringList ExPushPlugin::menulist() const
{
    return QStringList()
		<< tr("Close and Open 3D viewer and Push Image")
		<< tr("Push Object and Screenshot of global 3d viewer and also change 3d viewer")
		<< tr("Push time point of 3d viewer")
		<< tr("Look along direction in 3d viewer")
		<< tr("About");
}

void ExPushPlugin::domenu(const QString & menu_name, V3DPluginCallback2 & v3d, QWidget * parent)
{
    if (menu_name == tr("Close and Open 3D viewer and Push Image"))
    {
    	dopush(v3d, parent, 1);
    }
	else if (menu_name == tr("Push Object and Screenshot of global 3d viewer and also change 3d viewer"))
	{
    	dopush(v3d, parent, 2);
	}
	else if (menu_name == tr("Push time point of 3d viewer"))
	{
    	dopush(v3d, parent, 3);
	}
	else if (menu_name == tr("Look along direction in 3d viewer"))
	{
    	dopush(v3d, parent, 4);
	}
	else
	{
        QString msg = QString("Push Plugin version %1\ndeveloped by Hanchuan Peng & Zongcai Ruan. (Janelia Farm Research Campus, HHMI)")
                .arg(getPluginVersion(), 1, 'f', 1);
		QMessageBox::information(parent, "Version info", msg);
	}
}

//lookPanel* panel(V3DPluginCallback2 &v3d, QWidget *parent);


void dopush(V3DPluginCallback2 &v3d, QWidget *parent, int method_code)
{
	v3dhandle curwin = v3d.currentImageWindow();
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	Image4DSimple *oldimg = v3d.getImage(curwin);
	V3DLONG oldsz0=oldimg->getXDim(), oldsz1=oldimg->getYDim(), oldsz2=oldimg->getZDim();

	if (method_code==1) //push image
	{
		//close the current window
		v3d.close3DWindow(curwin);

//		//for (int j=1; j<1000; j++) //try to empty all existing events
//		{
//			QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
//		}

		//now should be able to open the new window
		v3d.open3DWindow(curwin);

		//now push the data to the 3d viewer's display
		for (int curloop=0; curloop<1; curloop++)
		{
//			Image4DSimple * oldimg = v3d.getImage(curwin);
//
			Image4DSimple p4DImage;
			unsigned char * pData = 0;
			V3DLONG sz0=oldsz0*0.9, sz1=oldsz1*.9, sz2=oldsz2*.9, sz3=oldimg->getCDim();
			V3DLONG totallen = sz0*sz1*sz2*sz3;
			pData = new unsigned char [totallen];
			memcpy(pData, oldimg->getRawData(), totallen);

			p4DImage.setData((unsigned char*)pData, sz0, sz1, sz2, sz3, V3D_UINT8);

			v3d.setImage(curwin, &p4DImage);
			v3d.setImageName(curwin, QString("push now %1").arg(curloop));
			v3d.updateImageWindow(curwin);

			v3d.pushImageIn3DWindow(curwin);
		}
	}
	else if (method_code==2) //push marker and swc
	{
		//ensure the 3d viewer window is open; if not, then open it
		v3d.open3DWindow(curwin);

		//now push the data to the 3d viewer's display
		for (int curloop=0; curloop<=100; curloop++)
		{
			LandmarkList curlist;
			for (int i=0;i<20; i++)
			{
				LocationSimple s;
				s.x = rand()%oldsz0;
				s.y = rand()%oldsz1;
				s.z = rand()%oldsz2;
				if (i<10)
					s.radius = 10*i;
				else {
					s.radius=s.radius*0.5;
				}

				curlist << s;
			}

			v3d.setLandmark(curwin, curlist);
			v3d.setImageName(curwin, QString("push now %1").arg(curloop));

			v3d.pushObjectIn3DWindow(curwin);

			//100812 RZC: put here in loop is more safe, every loop checking makes sure that the view3d is open.
			View3DControl *view = v3d.getView3DControl(curwin);
			if (view)  view->setXRotation(curloop*360/100);
			if (view)  view->setYRotation(curloop*360/100);
			if (view)  view->setZRotation(curloop*360/100);

			v3d.updateImageWindow(curwin);

//			QString BMPfilename = QString("aaa_%1").arg(curloop);
//			v3d.screenShot3DWindow(curwin, BMPfilename);
		}
	}
	else if (method_code==3) //push timepoint
	{
		V3DLONG szt = oldimg->getTDim();

		//ensure the 3d viewer window is open; if not, then open it
		v3d.open3DWindow(curwin);

		//now push the time point in the 3d viewer's display
		for (int t=0; t<szt; t++)
		{
			v3d.pushTimepointIn3DWindow(curwin, t);
			v3d.updateImageWindow(curwin);
		}
	}
	else if (method_code==4) //look along
	{
		panel(v3d, parent);
	}
}


