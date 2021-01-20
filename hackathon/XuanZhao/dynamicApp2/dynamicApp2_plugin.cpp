/* dynamicApp2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-10-21 : by ZX
 */
 
#include "v3d_message.h"
#include <vector>
#include "dynamicApp2_plugin.h"

#include "vn_app2.h"
#include "axontrace.h"

#include "swc_convert.h"


#include "dlog.h"

using namespace std;
Q_EXPORT_PLUGIN2(dynamicApp2, dynamicApp2Plugin);

QString versionStr = "v0.001";
 
QStringList dynamicApp2Plugin::menulist() const
{
	return QStringList() 
        <<tr("ultratracerAxonTerafly")
		<<tr("menu2")
		<<tr("about");
}

QStringList dynamicApp2Plugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void dynamicApp2Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("ultratracerAxonTerafly"))
	{
        qInstallMsgHandler(messageOutput);
        qDebug()<<"---------test----------";
        ultratracerAxonTerafly(callback,parent);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by ZX, 2020-10-21"));
	}
}

bool dynamicApp2Plugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("app2"))
	{
        PARA_APP2 p;

        if (!p.fetch_para_commandline(input, output, callback, parent))
            return false;

        if (!proc_app2(callback, p, versionStr))
            return false;
	}
    else if (func_name == tr("dynamicApp2"))
	{
        qInstallMsgHandler(messageOutput);
        PARA_APP2 p;

        if (!p.fetch_para_commandline(input, output, callback, parent))
            return false;

        if (!proc_app2_dynamic2(callback, p, versionStr))
            return false;
	}
    else if (func_name == tr("ultratracerAxonTerafly"))
    {
        qInstallMsgHandler(messageOutput);
        QString swcPath = infiles.size()>=1 ? infiles[0] : "";
        QString brainPath = infiles.size()>=2 ? infiles[1] : "";
        QString outSwcPath = swcPath + "_result.swc";

        NeuronTree ori = readSWC_file(swcPath);
        NeuronTree resultTree = ultratracerAxonTerafly(brainPath,ori,callback);
        writeSWC_file(outSwcPath,resultTree);

    }
    else if (func_name == tr("ultratracerLine")) {
        qInstallMsgHandler(messageOutput);
        QString imgPath = infiles.size()>=1 ? infiles[0] : "";
        QString swcPath = infiles.size()>=2 ? infiles[1] : "";
        double length_thresh = inparas.size()>=1 ? atof(inparas[0]) : 30;
        PARA_APP2 p;
        cout<<"-------------start--------------"<<endl;
        p.p4dImage = callback.loadImage((char *)(qPrintable(imgPath)));
//        unsigned char* pdata = 0;
//        V3DLONG sz[4];
//        int dataType = 1;
//        if(!simple_loadimage_wrapper(callback,imgPath.toStdString().c_str(),pdata,sz,dataType)){
//            cout<<"load image failed"<<endl;
//            return false;
//        }

//        Image4DSimple* p4dImage = new Image4DSimple();
//        p4dImage->setData(pdata,sz[0],sz[1],sz[2],sz[3],V3D_UINT8);
//        p.p4dImage = p4dImage;

        p.xc0 = p.yc0 = p.zc0 = 0;
        p.xc1 = p.p4dImage->getXDim()-1;
        p.yc1 = p.p4dImage->getYDim()-1;
        p.zc1 = p.p4dImage->getZDim()-1;
        qDebug()<<"load image end"<<endl;

        NeuronTree nt = readSWC_file(swcPath);
        vector<MyMarker*> markers = swc_convert(nt);
        map<MyMarker*,int> markersMap;
        for(int i=0; i<markers.size(); i++){
            markersMap[markers[i]] = i;
        }
        qDebug()<<"markers size: "<<markers.size();

        vector<int> markersChildren = vector<int>(markers.size(), 0);
        MyMarker* rootMarker;
        MyMarker* leafMarker;
        for(int i=0; i<markers.size(); i++){
            if(markers[i]->parent == 0){
                rootMarker = markers[i];
                continue;
            }
            int pIndex = markersMap[markers[i]->parent];
            markersChildren[pIndex]++;
        }
        vector<MyMarker*> leafMarkers;
        for(int i=0; i<markers.size(); i++){
            if(markersChildren[i] == 0){
                leafMarkers.push_back(markers[i]);
            }
        }
        if(leafMarkers.size() != 1){
            cout<<"the leafMarker size is wrong"<<endl;
            if(p.p4dImage){
                delete p.p4dImage;
                p.p4dImage = 0;
            }
            return false;
        }
        leafMarker = leafMarkers[0];
        p.root = leafMarker;
        p.rootFore = rootMarker;
        p.f_length = length_thresh;
        proc_app2_line(callback,p,versionStr);



        if(p.p4dImage){
            delete p.p4dImage;
            p.p4dImage = 0;
        }

    }
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

