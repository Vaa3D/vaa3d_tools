/* MarkerSize_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-26 : by Zhi ZHou
 */
 
#include "v3d_message.h"
#include <vector>
#include "MarkerSize_plugin.h"
#include "basic_surf_objs.h"
#include <iostream>


#include <fstream>
#include <sstream>
using namespace std;
Q_EXPORT_PLUGIN2(MarkerSize, MarkerSize);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);
 
QStringList MarkerSize::menulist() const
{
	return QStringList() 
		<<tr("GetMarkerSize")
		<<tr("about");
}

QStringList MarkerSize::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void MarkerSize::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("GetMarkerSize"))
	{
        processImage(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by Zhi ZHou, 2013-08-26"));
	}
}

bool MarkerSize::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d = p4DImage->getRawData();
    QString imgname = callback.getImageName(curwin);
    //V3DLONG totalpxls = p4DImage->getTotalBytes();
    V3DLONG pagesz = p4DImage->getTotalUnitNumberPerChannel();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    V3DLONG sc = p4DImage->getCDim();

    V3DLONG in_sz[4];
    in_sz[0] = N; in_sz[1] = M; in_sz[2] = P; in_sz[3] = sc;


    int tmpx,tmpy,tmpz,x1,y1,z1;
    LandmarkList listLandmarks = callback.getLandmark(curwin);
    LocationSimple tmpLocation(0,0,0);
    int marknum = listLandmarks.count();
    if(marknum ==0)
    {
        v3d_msg("No markers in the current image, please double check.");
        return;
    }

    NeuronTree marker_windows;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();
    int index = 1;
    for (int i=0;i<marknum;i++)
    {
        tmpLocation = listLandmarks.at(i);
        tmpLocation.getCoord(tmpx,tmpy,tmpz);
        V3DLONG ix = tmpx-1;
        V3DLONG iy = tmpy-1;
        V3DLONG iz = tmpz-1;
        V3DLONG offsetk = iz*M*N;
        V3DLONG offsetj = iy*N;
        V3DLONG PixelValue = data1d[offsetk + offsetj + ix];
        int Ws = 2*(int)round((log(PixelValue)/log(2)));
        //int Ws = 2*PixelValue;
        printf("window size is %d %d (%d %d %d)\n", Ws,PixelValue,ix,iy,iz);
        NeuronSWC S;
        if(Ws>=0)
        {

            S.n     = index;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= -1;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+1;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+2;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index+1;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+3;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index+2;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+4;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+3;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+5;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+4;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+6;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+5;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+7;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+6;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+8;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+1;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+9;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+2;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+10;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+3;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);


            S.n     = index+11;
            S.type 	= 7;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz-Ws;
            S.r 	= 1;
            S.pn 	= index;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);

            S.n     = index+12;
            S.type 	= 7;
            S.x 	= tmpx+Ws;
            S.y 	= tmpy-Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index+4;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);


            S.n     = index+13;
            S.x 	= tmpx-Ws;
            S.y 	= tmpy+Ws;
            S.z 	= tmpz+Ws;
            S.r 	= 1;
            S.pn 	= index;
            listNeuron.append(S);
            hashNeuron.insert(S.n, listNeuron.size()-1);


             index += 14;

       }
        marker_windows.n = -1;
        marker_windows.on = true;
        marker_windows.listNeuron = listNeuron;
        marker_windows.hashNeuron = hashNeuron;

    }


    QString outfilename = imgname + "_marker.swc";
    if (outfilename.startsWith("http", Qt::CaseInsensitive))
    {
        QFileInfo ii(outfilename);
        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
    }
    //v3d_msg(QString("The anticipated output file is [%1]").arg(outfilename));
    writeSWC_file(outfilename,marker_windows);
    NeuronTree nt = readSWC_file(outfilename);
    callback.setSWC(curwin, nt);
    callback.open3DWindow(curwin);
    callback.getView3DControl(curwin)->updateWithTriView();
    v3d_msg(QString("You have totally [%1] markers for the file [%2] and the computed MST has been saved to the file [%3]").arg(marknum).arg(imgname).arg(outfilename));

    return;
}
