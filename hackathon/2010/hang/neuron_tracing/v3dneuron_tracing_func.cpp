/* v3dneuron_tracing_func.cpp
 * This plugin will call v3dneuron_tracing command in dofunc
 * 2011-07-07 : by xiaoh10
 */

#include <v3d_interface.h>
#include "v3d_message.h"
#include "v3dneuron_tracing_func.h"
#include <vector>
#include <iostream>

#include "basic_surf_objs.h"
using namespace std;

const QString title = QObject::tr("V3dneuron_tracing Plugin");

int split(const char *paras, char ** &args)
{
    int argc = 0;
    int len = strlen(paras);
    int posb[200];
    char * myparas = new char[len];
    strcpy(myparas, paras);
    for(int i = 0; i < len; i++)
    {
        if(i==0 && myparas[i] != ' ' && myparas[i] != '\t')
        {
            posb[argc++]=i;
        }
        else if((myparas[i-1] == ' ' || myparas[i-1] == '\t') &&
                (myparas[i] != ' ' && myparas[i] != '\t'))
        {
            posb[argc++] = i;
        }
    }

    args = new char*[argc];
    for(int i = 0; i < argc; i++)
    {
        args[i] = myparas + posb[i];
    }

    for(int i = 0; i < len; i++)
    {
        if(myparas[i]==' ' || myparas[i]=='\t')myparas[i]='\0';
    }
    return argc;
}
int v3dneuron_tracing(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandleList win_list = callback.getImageWindowList();

	if(win_list.size()<1)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return -1;
	}
	v3dhandle curwin = callback.currentImageWindow();
	LandmarkList landmarks = callback.getLandmark(curwin);
	if(landmarks.empty())
	{
		v3d_msg("Please set a landmark!");
		return 0;
	}
	QList <ImageMarker> imagemarks;
	for(int i = 0; i < landmarks.size(); i++)
	{
		ImageMarker m;
		LocationSimple l = landmarks.at(i);
		m.x = l.x;
		m.y = l.y;
		m.z = l.z;
		imagemarks.push_back(m);
	}
	system("rm -f /tmp/mymarks.marker");
	system("rm -f /tmp/tmp_out*");
	writeMarker_file("/tmp/mymarks.marker",imagemarks);
	QString img_file = callback.getImageName(curwin);
	bool ok;
	QString nt_path = QInputDialog::getText(0, QObject::tr("Set path"), QObject::tr("v3dneuron_tracing path : "), QLineEdit::Normal, "~/Local/bin/v3dneuron_tracing", &ok);
	//QString paras = QObject::tr("v3dneuron_tracing -s %1 -S /tmp/mymarks.marker -o /tmp/tmp_out").arg(img_file);
	QString paras = QObject::tr("%1 -s \"%2\" -S /tmp/mymarks.marker -o /tmp/tmp_out").arg(nt_path).arg(img_file);
	qDebug(paras.toStdString().c_str());
	//QMessageBox::information(0,"",paras);
	system(paras.toStdString().c_str());
	NeuronTree nt = readSWC_file("/tmp/tmp_out_0.swc");
	//nt.editable = false;
	callback.setSWC(curwin, nt);
	callback.updateImageWindow(curwin);
	callback.open3DWindow(curwin);
	//callback.getView3DControl(curwin)->setShowSurfObjects(2);
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
	//callback.setImageName(newwin, QObject::tr("v3dneuron_tracing"));
	//callback.updateImageWindow(newwin);
	return 1;
}

bool v3dneuron_tracing(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"Welcome to v3dneuron_tracing"<<endl;
	if(input.size() != 2 || output.size() != 1) return false;
	char * paras = 0;
	if(((vector<char*> *)(input.at(1).p))->empty()){paras = new char[1]; paras[0]='\0';}
	else paras = (*(vector<char*> *)(input.at(1).p)).at(0);
	cout<<"paras : "<<paras<<endl;

	for(int i = 0; i < strlen(paras); i++)
	{
		if(paras[i] == '#') paras[i] = '-';
	}
	cout<<"paras : "<<paras<<endl;

	cout<<string("v3dneuron_tracing ").append(paras).c_str()<<endl;
	system(string("v3dneuron_tracing ").append(paras).c_str());
	return true;
}


