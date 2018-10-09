/*
 *  marker2others.cpp
 *  save an image's markers to other formats
 *
 *  Created by Hanchuan Peng, 2012-07-02
 */
 
#include "v3d_message.h"
#include "marker2others_plugin.h"
#include <vector>
#include <iostream>

using namespace std;

Q_EXPORT_PLUGIN2(marker2others, Marker2OthersPlugin);
 
//
void marker2others(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
    LandmarkList mlist = callback.getLandmark(curwin);
    QString imgname = callback.getImageName(curwin);
    if (mlist.isEmpty())
    {
        v3d_msg(QString("The marker list of the current image [%1] is empty. Do nothing.").arg(imgname));
        return;
    }
    
    NeuronTree nt;
    QList <NeuronSWC> & listNeuron = nt.listNeuron;
    
    for (int i=0;i<mlist.size();i++)
    {
        NeuronSWC n;
        n.x = mlist[i].x-1;
        n.y = mlist[i].y-1;
        n.z = mlist[i].z-1;
        n.n = i;
        n.type = 2;
        n.r = 1;
        n.pn = -1; //so the first one will be root
        listNeuron << n;
    }
    
    QString outfilename = imgname + "_marker.swc";
    if (outfilename.startsWith("http", Qt::CaseInsensitive))
    {
        QFileInfo ii(outfilename);
        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
    }
    
    QStringList infostr;
    
    writeSWC_file(outfilename, nt, &infostr);
    v3d_msg(QString("The SWC file [%1] has been saved.").arg(outfilename));

	return;
}

bool marker2apo(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.empty())
    {
        cerr<<"Need input marker file"<<endl;
        return false;
    }

    if(outfiles.empty())
    {
        cerr<<"Need output apo file"<<endl;
        return false;
    }

    QString  inmarker_file =  infiles[0];
    QString  outapo_file =  outfiles[0];

    int k=0;
    double scale;
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        scale = paras.empty() ? 1.0 : atof(paras[k]);
    }

    QList <ImageMarker> listLandmarks = readMarker_file(inmarker_file);
    QList<CellAPO> file_inmarkers;
    for(int i=0; i<listLandmarks.size();i++)
    {
        CellAPO t;
        t.x = listLandmarks[i].x*scale;
        t.y = listLandmarks[i].y*scale;
        t.z = listLandmarks[i].z*scale;
        t.color.r = 255;
        t.color.g = 0;
        t.color.b = 0;

        file_inmarkers.push_back(t);
    }

    writeAPO_file(outapo_file,file_inmarkers);
    return true;
}

void printHelp(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("This plugin converts and saves an image's marker to the SWC format. ");
}

void printHelp(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    v3d_msg("Not implemented yet", 0);
    return;
}

//


QStringList Marker2OthersPlugin::menulist() const
{
	return QStringList()
		<<tr("Save markers to SWC format")
		<<tr("about");
}

QStringList Marker2OthersPlugin::funclist() const
{
	return QStringList()
        <<tr("marker2apo")
		<<tr("help");
}

void Marker2OthersPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Save markers to SWC format"))
	{
		marker2others(callback,parent);
	}
	else if (menu_name == tr("help"))
	{
		printHelp(callback,parent);
	}
	else
	{
		printHelp(callback,parent);
	}
}

bool Marker2OthersPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("marker2apo"))
	{
        marker2apo(input, output);
        return true;
	}
	else if (func_name == tr("help"))
	{
		printHelp(input,output);
        return true;
	}
	return false;
}

