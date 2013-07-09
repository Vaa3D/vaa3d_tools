/* sync3D_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-07-09 : by Zhi Zhou
 */
 
#include "v3d_message.h"
#include <vector>
#include "sync3D_plugin.h"
#include <QtGui>
#include <stdlib.h>

using namespace std;
Q_EXPORT_PLUGIN2(sync3D, sync3D);
void SynTwoImage(V3DPluginCallback2 &v3d, QWidget *parent);
 
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

	QStringList items;
	for (int i=0; i<win_list.size(); i++) items << v3d.getImageName(win_list[i]);

	QDialog d(parent);
	QComboBox* combo1 = new QComboBox(); combo1->addItems(items);
	QComboBox* combo2 = new QComboBox(); combo2->addItems(items);
	QPushButton* ok     = new QPushButton("OK");
	QPushButton* cancel = new QPushButton("Cancel");
	QFormLayout *formLayout = new QFormLayout;
	formLayout->addRow(QObject::tr("Source: "), combo1);
	formLayout->addRow(QObject::tr("Target: "), combo2);
	formLayout->addRow(ok, cancel);
	d.setLayout(formLayout);
	d.setWindowTitle(QString("Synchronize"));

	d.connect(ok,     SIGNAL(clicked()), &d, SLOT(accept()));
	d.connect(cancel, SIGNAL(clicked()), &d, SLOT(reject()));
	if (d.exec()!=QDialog::Accepted)
		return;

	int i1 = combo1->currentIndex();
	int i2 = combo2->currentIndex();

	Image4DSimple* image1 = v3d.getImage(win_list[i1]);
	Image4DSimple* image2 = v3d.getImage(win_list[i2]);
	v3dhandle curwin = v3d.currentImageWindow();
	if (win_list[i1]&& win_list[i2])//ensure the 3d viewer window is open; if not, then open it
	{
	 	v3d.open3DWindow(win_list[i1]);
		View3DControl *view1 = v3d.getView3DControl(win_list[i1]);
		
		v3d.open3DWindow(win_list[i2]);
		View3DControl *view2 = v3d.getView3DControl(win_list[i2]);
		if (view1 && view2)
		{  
			view1->lookAlong(100,100,100);
			view2->lookAlong(100,100,100);
			v3d.updateImageWindow(win_list[i1]);
			v3d.updateImageWindow(win_list[i2]);
		}
	}

}

