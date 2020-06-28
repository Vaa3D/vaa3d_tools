/* Retrace_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-6-25 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "Retrace_plugin.h"

#include "retracedialog.h"
#include "app2.h"

using namespace std;
Q_EXPORT_PLUGIN2(Retrace, RetracePlugin);
 
QStringList RetracePlugin::menulist() const
{
	return QStringList() 
        <<tr("Retrace")
        <<tr("app2Convenient")
		<<tr("about");
}

QStringList RetracePlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void RetracePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Retrace"))
	{
        RetraceDialog dlg = RetraceDialog(parent,callback);
        dlg.exec();
	}
    else if (menu_name == tr("app2Convenient"))
	{
        paraApp2 p = paraApp2();
        p.initialize(callback);
        p.bkg_thresh = -1;
        proc_app2(p);
        QString rootposstr="", tmps;
        if(p.landmarks.size()>0){
            tmps.setNum(int(p.landmarks[0].x-1+0.5)).prepend("_x"); rootposstr += tmps;
            tmps.setNum(int(p.landmarks[0].y-1+0.5)).prepend("_y"); rootposstr += tmps;
            tmps.setNum(int(p.landmarks[0].z-1+0.5)).prepend("_z"); rootposstr += tmps;
        }
        QList<ImageMarker> markers;
        for(int i=0; i<p.landmarks.size(); i++){
            ImageMarker m = ImageMarker(p.landmarks[i].x,p.landmarks[i].y,p.landmarks[i].z);
            m.color = p.landmarks[i].color;
            m.radius = p.landmarks[i].radius;
            markers.push_back(m);
        }


        QString swcFile = QString(p.p4dImage->getFileName()) + rootposstr + "_app2.swc";
        QString markerFile = QString(p.p4dImage->getFileName()) + rootposstr + "_app2.marker";

        writeSWC_file(swcFile,p.result);
        writeMarker_file(markerFile,markers);

//        p.result.color.r = p.result.color.g = p.result.color.b = 0;
        NeuronTree nt = readSWC_file(swcFile);
        v3dhandle curwin = callback.currentImageWindow();
        callback.setSWC(curwin,nt);
//        QString imageName = callback.getImageName(curwin);
        callback.open3DWindow(curwin);
        callback.getView3DControl(curwin)->updateWithTriView();
//        V3dR_MainWindow* cur3d =  callback.find3DViewerByName(imageName);
//        QList <NeuronTree> * trees = callback.getHandleNeuronTrees_Any3DViewer(cur3d);
//        trees->clear();
//        for(int i=0; i<p.result.listNeuron.size(); i++){
//            qDebug()<<"before i: "<<i<<" type: "<<p.result.listNeuron[i].type;
//        }
//        trees->append(p.result);
//        for(int i=0; i<trees->at(0).listNeuron.size(); i++){
//            qDebug()<<"after i: "<<i<<" type: "<<p.result.listNeuron[i].type;
//        }
//        callback.update_3DViewer(cur3d);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by ZX, 2020-6-25"));
	}
}

bool RetracePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

