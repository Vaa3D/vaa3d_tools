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
void others2marker(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    NeuronTree nt = callback.getSWC(curwin);
    QString imgname = callback.getImageName(curwin);
    if (!nt.listNeuron.size())
    {
        v3d_msg(QString("The SWC of the current image [%1] is empty. Do nothing.").arg(imgname));
        return;
    }
    QList <NeuronSWC> & listNeuron = nt.listNeuron;
    QList <ImageMarker> listLandmarks;
    for (int i=0;i<listNeuron.size();i++)
    {
        ImageMarker m;
        m.x=listNeuron[i].x;
        m.y=listNeuron[i].y;
        m.z=listNeuron[i].z;
        m.n=listNeuron[i].n;
        m.name="";
        m.type=0;
        m.shape=0;
        m.radius=1;
        m.color.r = 255;
        m.color.g = 0;
        m.color.b = 0;
        listLandmarks.append(m);
    }

    QString outfilename = imgname + ".marker";
    if (outfilename.startsWith("http", Qt::CaseInsensitive))
    {
        QFileInfo ii(outfilename);
        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
    }
    writeMarker_file(outfilename, listLandmarks);
    v3d_msg(QString("The Marker file [%1] has been saved.").arg(outfilename));

    return;
}
bool others2marker(QString inswc_file)
{
    QString  outmarker_file = inswc_file+".marker";
    NeuronTree nt=readSWC_file(inswc_file);
    QList <ImageMarker> listLandmarks;
    if(!nt.listNeuron.size())
        return false;

    double scale=1;

    for(int i=0; i<nt.listNeuron.size();i++)
    {
        ImageMarker m;
        m.x=nt.listNeuron[i].x*scale;
        m.y=nt.listNeuron[i].y*scale;
        m.z=nt.listNeuron[i].z*scale;
        m.n=nt.listNeuron[i].n;
        m.name="";
        m.type=0;
        m.shape=0;
        m.radius=1;
        m.color.r = 255;
        m.color.g = 0;
        m.color.b = 0;
        listLandmarks.append(m);
    }

    writeMarker_file(outmarker_file,listLandmarks);
    QString FinishMsg = QString("Marker file [") + outmarker_file + QString("] has been generated.");
    qDebug()<<FinishMsg;
    return true;
}
bool others2marker(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.empty())
    {
        cerr<<"Need input swc file"<<endl;
        return false;
    }

    QString  inswc_file =  infiles[0];
    QString  outmarker_file = inswc_file+".marker";
    if(outfiles.size())
        outmarker_file =  outfiles[0];

    int k=0;
    double scale;
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        scale = paras.empty() ? 1.0 : atof(paras[k]);
    }

    NeuronTree nt=readSWC_file(inswc_file);
    QList <ImageMarker> listLandmarks;
    if(!nt.listNeuron.size())
        return false;
    for(int i=0; i<nt.listNeuron.size();i++)
    {
        ImageMarker m;
        m.x=nt.listNeuron[i].x*scale;
        m.y=nt.listNeuron[i].y*scale;
        m.z=nt.listNeuron[i].z*scale;
        m.n=nt.listNeuron[i].n;
        m.name="";
        m.type=0;
        m.shape=0;
        m.radius=1;
        m.color.r = 255;
        m.color.g = 0;
        m.color.b = 0;
        listLandmarks.append(m);
    }

    writeMarker_file(outmarker_file,listLandmarks);
    return true;
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
bool marker2apo(QString markerFileName)
{
	QList <ImageMarker> listLandmarks = readMarker_file(markerFileName);
	QList<CellAPO> file_inmarkers;
	for (int i = 0; i<listLandmarks.size(); i++)
	{
		CellAPO t;
		t.x = listLandmarks[i].x;
		t.y = listLandmarks[i].y;
		t.z = listLandmarks[i].z;
		t.color.r = 255;
		t.color.g = 0;
		t.color.b = 0;
		t.volsize = 314.159;

		file_inmarkers.push_back(t);
	}

	QString saveName = markerFileName + ".apo";
	writeAPO_file(saveName, file_inmarkers);

	QString FinishMsg = QString("An apo file [") + saveName + QString("] has been generated.");
	v3d_msg(FinishMsg);
	
	return true;
}
bool apo2marker(const V3DPluginArgList & input, V3DPluginArgList & output)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if(infiles.empty())
    {
        cerr<<"Need input apo file"<<endl;
        return false;
    }

    QString  inapo_file =  infiles[0];
    QString  outmarker_file = inapo_file+".marker";
    if(outfiles.size())
        outmarker_file =  outfiles[0];

    QList <CellAPO> inapolist=readAPO_file(inapo_file);
    QList <ImageMarker> listLandmarks;
    if(!inapolist.size())
        return false;

    int k=0;
    double scale;
    if (input.size()>=2)
    {
        vector<char*> paras = (*(vector<char*> *)(input.at(1).p));
        scale = paras.empty() ? 1.0 : atof(paras[k]);
    }

    for(int i=0; i<inapolist.size();i++)
    {
        ImageMarker m;
        m.x=inapolist[i].x*scale;
        m.y=inapolist[i].y*scale;
        m.z=inapolist[i].z*scale;
        m.n=inapolist[i].n;
        m.name=inapolist[i].name;
        m.color=inapolist[i].color;
        m.comment=inapolist[i].comment;
        m.radius=inapolist[i].volsize;
        m.type=0;
        m.shape=0;

        listLandmarks.append(m);
    }

    writeMarker_file(outmarker_file,listLandmarks);
    return true;
}
bool apo2marker(QString inapo_file)
{
    QString  outmarker_file = inapo_file+".marker";

    QList <CellAPO> inapolist=readAPO_file(inapo_file);
    QList <ImageMarker> listLandmarks;
    if(!inapolist.size())
        return false;

    double scale=1;

    for(int i=0; i<inapolist.size();i++)
    {
        ImageMarker m;
        m.x=inapolist[i].x*scale;
        m.y=inapolist[i].y*scale;
        m.z=inapolist[i].z*scale;
        m.n=inapolist[i].n;
        m.name=inapolist[i].name;
        m.color=inapolist[i].color;
        m.comment=inapolist[i].comment;
        m.radius=inapolist[i].volsize;
        m.type=0;
        m.shape=0;

        listLandmarks.append(m);
    }

    writeMarker_file(outmarker_file,listLandmarks);
    QString FinishMsg = QString("Marker file [") + outmarker_file + QString("] has been generated.");
    qDebug()<<FinishMsg;
    return true;
}
void printHelp(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3d_msg("This plugin converts and saves an image's marker to the SWC format. ");
    qDebug()<<"This plugin also can convert apo or swc file to marker format";
}

void printHelp()
{
    qDebug()<<"vaa3d -x <libname:marker2others> -f marker2apo -i <input_marker_file> -o <out_apo_file>";
    qDebug()<<"vaa3d -x <libname:marker2others> -f apo2marker -i <input_apo_file> -o <out_marker_file>";
    qDebug()<<"vaa3d -x <libname:marker2others> -f swc2marker -i <input_swc_file> -o <out_marker_file>";
    return;
}
//
QStringList Marker2OthersPlugin::menulist() const
{
	return QStringList()
		<< tr("Save markers to SWC format")
		<< tr("Save markers to apo format")
        << tr("Save apo to marker format")
        <<tr("Save  SWC to markers format")
        <<tr("Convert  SWC to markers format")
		<<tr("about");
}
QStringList Marker2OthersPlugin::funclist() const
{
	return QStringList()
        <<tr("marker2apo")
       <<tr("apo2marker")
      <<tr("swc2marker")
		<<tr("help");
}

void Marker2OthersPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("Save markers to SWC format"))
	{
		marker2others(callback,parent);
	}
	else if (menu_name == tr("Save markers to apo format"))
	{
		QString markerFileName = QFileDialog::getOpenFileName(0, QObject::tr("Select marker file"), "",
			QObject::tr("Supported file extension (*.marker)"));
		marker2apo(markerFileName);
	}
    else if (menu_name == tr("Save apo to marker format"))
    {
        QString apoFileName = QFileDialog::getOpenFileName(0, QObject::tr("Select apo file"), "",
            QObject::tr("Supported file extension (*.apo)"));
        apo2marker(apoFileName);
    }
    else if (menu_name == tr("Save  SWC to markers format"))
    {
        others2marker(callback,parent);
    }
    else if (menu_name == tr("Convert  SWC to markers format"))
    {
        QString inswcFile = QFileDialog::getOpenFileName(0, QObject::tr("Select swc file"), "",
            QObject::tr("Supported file extension (*.swc)"));
        others2marker(inswcFile);
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
    else if (func_name == tr("apo2marker"))
    {
        apo2marker(input,output);
        return true;
    }
    else if (func_name == tr("swc2marker"))
    {
        others2marker(input,output);
        return true;
    }
	else if (func_name == tr("help"))
	{
        printHelp();
        return true;
	}
	return false;
}

