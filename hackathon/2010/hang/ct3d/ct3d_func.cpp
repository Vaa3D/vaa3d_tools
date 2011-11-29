/* ct3d_func.cpp
 * This plugin will invoke ct3d program, which track and segment cells from more than two images
 * 2011-06-27 : by Hang Xiao & Axel Mosig
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "ct3d_func.h"
#include "ct3d_gui.h"

//#include "mythread.h"

const QString title = QObject::tr("Ct3d Plugin");
int open_ct3d(V3DPluginCallback2 &callback, QWidget *parent)
{
	Ct3dWidget * widget = new Ct3dWidget(callback, parent);
	widget->show();
	//MyThread * mythread = new MyThread(&callback,callback.currentImageWindow());
	//TestDialog dialog(callback, parent);

	//if (dialog.exec()!=QDialog::Accepted) return -1;

	//dialog.update();
	//int i = dialog.i;
	//int c = dialog.channel;
	//Image4DSimple *p4DImage = callback.getImage(win_list[i]);
	//if(p4DImage->getCDim() <= c) {v3d_msg(QObject::tr("The channel isn't existed.")); return -1;}
	//V3DLONG sz[3];
	//sz[0] = p4DImage->getXDim();
	//sz[1] = p4DImage->getYDim();
	//sz[2] = p4DImage->getZDim();

	//unsigned char * inimg1d = p4DImage->getRawDataAtChannel(c);

	//v3dhandle newwin;
	//if(QMessageBox::Yes == QMessageBox::question(0, "", QString("Do you want to use the existing windows?"), QMessageBox::Yes, QMessageBox::No))
		//newwin = callback.currentImageWindow();
	//else
		//newwin = callback.newImageWindow();

	//p4DImage->setData(inimg1d, sz[0], sz[1], sz[2], sz[3]);
	//callback.setImage(newwin, p4DImage);
	//callback.setImageName(newwin, QObject::tr("catch_mouse_event"));
	//callback.updateImageWindow(newwin);
	return 1;
}

