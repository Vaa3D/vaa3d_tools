/* marker_minspanningtree_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-03 : by Zhi Zhou
 */

 
#include "v3d_message.h"
#include <vector>
#include "marker_minspanningtree_plugin.h"
#include "basic_surf_objs.h"

#include <iostream>


#include <fstream>
#include <sstream>
//#ifndef test_main_cpp


#define INF 1E9
using namespace std;
Q_EXPORT_PLUGIN2(marker2mst, markertree);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);

QStringList markertree::menulist() const
{
	return QStringList() 
        <<tr("generate a MST for all the markers")
		<<tr("about");
}

QStringList markertree::funclist() const
{
	return QStringList()
		<<tr("help");
}

void markertree::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("generate a MST for all the markers"))
	{
		processImage(callback,parent);
	}
	else
	{
        v3d_msg(tr("Developed by Zhi Zhou, 2013-08-03"));
	}
}

void processImage(V3DPluginCallback2 &callback, QWidget *parent)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        v3d_msg("You don't have any image open in the main window.");
        return;
    }
	
    Image4DSimple* p4DImage = callback.getImage(curwin);
    QString imgname = callback.getImageName(curwin);
	
    if (!p4DImage)
    {
        v3d_msg("The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    int tmpx,tmpy,tmpz,x1,y1,z1;
	LandmarkList listLandmarks = callback.getLandmark(curwin);
	LocationSimple tmpLocation(0,0,0);
	int marknum = listLandmarks.count();
    if(marknum ==0)
    {
        v3d_msg("No markers in the current image, please double check.");
        return;
    }
	double** markEdge = new double*[marknum];
	for(int i = 0; i < marknum; i++)
	{
		markEdge[i] = new double[marknum];

	}

	
	for (int i=0;i<marknum;i++)
	{
		tmpLocation = listLandmarks.at(i);
		tmpLocation.getCoord(tmpx,tmpy,tmpz);
		x1 = tmpx;
		y1 = tmpy;
		z1 = tmpz;
		for (int j=0;j<marknum;j++)
		{
			tmpLocation = listLandmarks.at(j);
			tmpLocation.getCoord(tmpx,tmpy,tmpz);
            markEdge[i][j] = sqrt(double(x1-tmpx)*double(x1-tmpx) + double(y1-tmpy)*double(y1-tmpy) + double(z1-tmpz)*double(z1-tmpz));
		}
	}
	
    //NeutronTree structure
    NeuronTree marker_MST;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    //set node

    NeuronSWC S;
    tmpLocation = listLandmarks.at(0);
    tmpLocation.getCoord(tmpx,tmpy,tmpz);

    S.n 	= 1;
    S.type 	= 7;
    S.x 	= tmpx;
    S.y 	= tmpy;
    S.z 	= tmpz;
    S.r 	= 1;
    S.pn 	= -1;
    listNeuron.append(S);
    hashNeuron.insert(S.n, listNeuron.size()-1);

        int* pi = new int[marknum];
        for(int i = 0; i< marknum;i++)
            pi[i] = 0;
        pi[0] = 1;
        int indexi,indexj;
        for(int loop = 0; loop<marknum;loop++)
        {
            double min = INF;
              for(int i = 0; i<marknum; i++)
              {
                if (pi[i] == 1)
                {
                    for(int j = 0;j<marknum; j++)
                    {
                        if(pi[j] == 0 && min > markEdge[i][j])
                        {
                            min = markEdge[i][j];
                            indexi = i;
                            indexj = j;
                        }
                    }
                }

              }
              if(indexi>=0)
              {
                tmpLocation = listLandmarks.at(indexj);
                tmpLocation.getCoord(tmpx,tmpy,tmpz);
                S.n 	= indexj+1;
                S.type 	= 7;
                S.x 	= tmpx;
                S.y 	= tmpy;
                S.z 	= tmpz;
                S.r 	= 1;
                S.pn 	= indexi+1;
                listNeuron.append(S);
                hashNeuron.insert(S.n, listNeuron.size()-1);

              }else
              {
                  break;
              }
            pi[indexj] = 1;
            indexi = -1;
            indexj = -1;
        }
    marker_MST.n = -1;
    marker_MST.on = true;
    marker_MST.listNeuron = listNeuron;
    marker_MST.hashNeuron = hashNeuron;


    QString outfilename = imgname + "_marker.swc";
    if (outfilename.startsWith("http", Qt::CaseInsensitive))
    {
        QFileInfo ii(outfilename);
        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
    }
    //v3d_msg(QString("The anticipated output file is [%1]").arg(outfilename));
    writeSWC_file(outfilename,marker_MST);
    v3d_msg(QString("You have totally [%1] markers for the file [%2] and the computed MST has been saved to the file [%3]").arg(marknum).arg(imgname).arg(outfilename));
    return;
}



bool markertree::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("help"))
	{
        v3d_msg("To be implemented");
	}
	else return false;
    
    return true;
}

