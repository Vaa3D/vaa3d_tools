/* Retrace_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-6-25 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "Retrace_plugin.h"

#include "retracedialog.h"
#include "app2.h"
//#include "../../../../released_plugins/v3d_plugins/sort_neuron_swc/sort_swc.h"

using namespace std;
Q_EXPORT_PLUGIN2(Retrace, RetracePlugin);
 
QStringList RetracePlugin::menulist() const
{
	return QStringList() 
        <<tr("Retrace")
        <<tr("app2Convenient")
        <<tr("app2Terafly")
        <<tr("app2MultiTerafly")
        <<tr("app2TeraflyWithPara")
        <<tr("app2MultiTeraflyWithPara")
        <<tr("ultratracerTerafly")
        <<tr("saveMarkerWithBoundingBox")
		<<tr("about");
}

QStringList RetracePlugin::funclist() const
{
	return QStringList()
        <<tr("ultratracingInBox")
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
        callback.open3DWindow(curwin);
        callback.getView3DControl(curwin)->updateWithTriView();

	}
    else if (menu_name == "app2Terafly") {
        app2Terafly(2,false,-1,-1,callback,parent);
    }
    else if (menu_name == "app2MultiTerafly") {
        app2MultiTerafly(2,false,-1,-1,callback,parent);
    }
    else if (menu_name == "app2TeraflyWithPara") {

        QDialog* dlg = new QDialog(parent);

        QLineEdit* typeEdit = new QLineEdit("2");
        QLineEdit* app2ThEdit = new QLineEdit("-1");
        QLineEdit* contrastEdit = new QLineEdit("1");
        QCheckBox* thresholdBox = new QCheckBox();
        QGridLayout* layout = new QGridLayout;
        layout->addWidget(new QLabel("type: "),1,1);
        layout->addWidget(typeEdit,1,2);
        layout->addWidget(new QLabel("app2 threshold: "),2,1);
        layout->addWidget(app2ThEdit,2,2);
        layout->addWidget(new QLabel("contrast ratio: "),3,1);
        layout->addWidget(contrastEdit,3,2);
        layout->addWidget(new QLabel("SimpleThreshold "),4,1);
        layout->addWidget(thresholdBox,4,2);

        QPushButton* start = new QPushButton("Start");
        QPushButton* cancel = new QPushButton("Cancel");

        connect(start, SIGNAL(clicked()), dlg, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), dlg, SLOT(reject()));
        layout->addWidget(cancel,5,1);
        layout->addWidget(start,5,2);

        dlg->setLayout(layout);
        int type;
        bool threshold;
        int app2Th;
        double contrastT;

        if(dlg->exec() != QDialog::Accepted) return;


        type = typeEdit->text().toInt();
        app2Th = app2ThEdit->text().toInt();
        threshold = thresholdBox->isChecked();
        contrastT = contrastEdit->text().toFloat();
        app2Terafly(type,threshold,app2Th,contrastT,callback,parent);
    }
    else if (menu_name == "app2MultiTeraflyWithPara") {
        QDialog* dlg = new QDialog(parent);

        QLineEdit* typeEdit = new QLineEdit("2");
        QLineEdit* app2ThEdit = new QLineEdit("-1");
        QLineEdit* contrastEdit = new QLineEdit("1");
        QCheckBox* thresholdBox = new QCheckBox();
        QGridLayout* layout = new QGridLayout;
        layout->addWidget(new QLabel("type: "),1,1);
        layout->addWidget(typeEdit,1,2);
        layout->addWidget(new QLabel("app2 threshold: "),2,1);
        layout->addWidget(app2ThEdit,2,2);
        layout->addWidget(new QLabel("contrast ratio: "),3,1);
        layout->addWidget(contrastEdit,3,2);
        layout->addWidget(new QLabel("SimpleThreshold "),4,1);
        layout->addWidget(thresholdBox,4,2);

        QPushButton* start = new QPushButton("Start");
        QPushButton* cancel = new QPushButton("Cancel");

        connect(start, SIGNAL(clicked()), dlg, SLOT(accept()));
        connect(cancel, SIGNAL(clicked()), dlg, SLOT(reject()));
        layout->addWidget(cancel,5,1);
        layout->addWidget(start,5,2);

        dlg->setLayout(layout);
        int type;
        bool threshold;
        int app2Th;
        double contrastT;

        if(dlg->exec() != QDialog::Accepted) return;


        type = typeEdit->text().toInt();
        app2Th = app2ThEdit->text().toInt();
        contrastT = contrastEdit->text().toFloat();
        threshold = thresholdBox->isChecked();
        app2MultiTerafly(type,threshold,app2Th,contrastT,callback,parent);
    }
    else if (menu_name == "ultratracerTerafly") {
        ultratracerTerafly(2,callback,parent);
    }
    else if (menu_name == "saveMarkerWithBoundingBox") {
        writeBlock(callback,parent);
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

    if (func_name == tr("ultratracingInBox"))
    {
        QString imageBlockPath = (inparas.size()>=1) ? inparas[0] : "";
        QString swcPath = (inparas.size()>=2) ? inparas[1] : "";
        qDebug()<<"imageBlockPath: "<<imageBlockPath.toStdString().c_str();
        qDebug()<<"swcPath: "<<swcPath.toStdString().c_str();
        tracingPipeline(imageBlockPath,swcPath,callback);
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

