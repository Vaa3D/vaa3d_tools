/* marker_minspanningtree_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2013-08-03 : by Zhi Zhou
 */

 
#include "v3d_message.h"
#include <vector>
#include "marker_minspanningtree_plugin.h"
#include "basic_surf_objs.h"
#include "../../../hackathon/xiaoxiaol/consensus_skeleton_2/mst_boost_prim.h"

#include <iostream>


#include <fstream>
#include <sstream>
//#ifndef test_main_cpp


#define INF 1E9
using namespace std;
using namespace boost;

Q_EXPORT_PLUGIN2(marker2mst, markertree);
void processImage(V3DPluginCallback2 &callback, QWidget *parent);
void processImage_siamese(V3DPluginCallback2 &callback, QWidget *parent);


QStringList markertree::menulist() const
{
	return QStringList() 
        <<tr("generate a MST for all the markers")
     //   <<tr("generate a MST based on siamese network")
		<<tr("about");
}

QStringList markertree::funclist() const
{
	return QStringList()
		<<tr("help");
}

template <class T> T pow2(T a)
{
    return a*a;

}


void markertree::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("generate a MST for all the markers"))
	{
		processImage(callback,parent);
    }else if (menu_name == tr("generate a MST based on siamese network"))
    {
        processImage_siamese(callback,parent);
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

    UndirectedGraph g(marknum);

    for (int i=0;i<marknum;i++)
    {
        tmpLocation = listLandmarks.at(i);
        tmpLocation.getCoord(tmpx,tmpy,tmpz);
        x1 = tmpx;
        y1 = tmpy;
        z1 = tmpz;
        for (int j=0;j<marknum;j++)
        {
            EdgeQuery edgeq = edge(i, j, *&g);
            if (!edgeq.second && i!=j)
            {
                tmpLocation = listLandmarks.at(j);
                tmpLocation.getCoord(tmpx,tmpy,tmpz);
                double Vedge = sqrt(double(x1-tmpx)*double(x1-tmpx) + double(y1-tmpy)*double(y1-tmpy) + double(z1-tmpz)*double(z1-tmpz));
                add_edge(i, j, LastVoted(i, Weight(Vedge)), *&g);
            }
        }
    }

 //   property_map<UndirectedGraph, edge_weight_t>::type weightmap = get(edge_weight, *&g);
    vector < graph_traits < UndirectedGraph >::vertex_descriptor > p(num_vertices(*&g));
    prim_minimum_spanning_tree(*&g, &p[0]);

    NeuronTree marker_MST;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    for (std::size_t i = 0; i != p.size(); ++i)
    {
        NeuronSWC S;
        tmpLocation = listLandmarks.at(i);
        tmpLocation.getCoord(tmpx,tmpy,tmpz);
        int pn;
        if(p[i] == i)
            pn = -1;
        else
            pn = p[i] + 1;

        S.n 	= i+1;
        S.type 	= 7;
        S.x 	= tmpx;
        S.y 	= tmpy;
        S.z 	= tmpz;
        S.r 	= 1;
        S.pn 	= pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);
    }

    marker_MST.n = -1;
    marker_MST.on = true;
    marker_MST.listNeuron = listNeuron;
    marker_MST.hashNeuron = hashNeuron;

    /*
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

*/
    QString outfilename = imgname + "_boost_marker.swc";
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

void processImage_siamese(V3DPluginCallback2 &callback, QWidget *parent)
{
    QString SWCfileName;
    SWCfileName = QFileDialog::getOpenFileName(0, QObject::tr("Open SWC File"),
            "",
            QObject::tr("Supported file (*.swc *.eswc)"
                ";;Neuron structure	(*.swc)"
                ";;Extended neuron structure (*.eswc)"
                ));
    if(SWCfileName.isEmpty())
        return;

    NeuronTree nt = readSWC_file(SWCfileName);
    ifstream ifs("/local4/Data/IVSCC_test/new_testing/tmp_NEUTUBE_v1/APP3/sim_full.txt");
    string info_sim;

    UndirectedGraph g(nt.listNeuron.size());

    V3DLONG d = 0;
    for (int i=0;i<nt.listNeuron.size()-1;i++)
    {
        for (int j=i+1;j<nt.listNeuron.size();j++)
        {
            V3DLONG x1 = nt.listNeuron.at(i).x;
            V3DLONG y1 = nt.listNeuron.at(i).y;
            V3DLONG z1 = nt.listNeuron.at(i).z;
            V3DLONG x2 = nt.listNeuron.at(j).x;
            V3DLONG y2 = nt.listNeuron.at(j).y;
            V3DLONG z2 = nt.listNeuron.at(j).z;
            double dis = sqrt(pow2(x1-x2) + pow2(y1-y2) + pow2(z1-z2));

            EdgeQuery edgeq = edge(i, j, *&g);
            if (!edgeq.second && i!=j)
            {
                double Vedge;
                if(dis<=80)
                {
                    getline(ifs, info_sim);
                    std::istringstream iss(info_sim);
                    iss >> Vedge;

                }
                if(dis>40)
                    Vedge = dis;
                add_edge(i, j, LastVoted(i, Weight(Vedge)), *&g);
                d++;
            }
        }


    }

    vector < graph_traits < UndirectedGraph >::vertex_descriptor > p(num_vertices(*&g));
    prim_minimum_spanning_tree(*&g, &p[0]);

    NeuronTree marker_MST;
    QList <NeuronSWC> listNeuron;
    QHash <int, int>  hashNeuron;
    listNeuron.clear();
    hashNeuron.clear();

    for (std::size_t i = 0; i != p.size(); ++i)
    {
        NeuronSWC S;
        int pn;
        if(p[i] == i)
            pn = -1;
        else
            pn = p[i] + 1;

        S.n 	= i+1;
        S.type 	= 7;
        S.x 	= nt.listNeuron.at(i).x;
        S.y 	= nt.listNeuron.at(i).y;
        S.z 	= nt.listNeuron.at(i).z;;
        S.r 	= 1;
        S.pn 	= pn;
        listNeuron.append(S);
        hashNeuron.insert(S.n, listNeuron.size()-1);
    }

    marker_MST.n = -1;
    marker_MST.on = true;
    marker_MST.listNeuron = listNeuron;
    marker_MST.hashNeuron = hashNeuron;


//    for (int i=1;i<marker_MST.listNeuron.size()-1;i++)
//    {
//        if(marker_MST.listNeuron.at(i).parent>0)
//        {
//            V3DLONG x1 = marker_MST.listNeuron.at(i).x;
//            V3DLONG y1 = marker_MST.listNeuron.at(i).y;
//            V3DLONG z1 = marker_MST.listNeuron.at(i).z;
//            V3DLONG x2 = marker_MST.listNeuron.at(marker_MST.listNeuron.at(i).parent).x;
//            V3DLONG y2 = marker_MST.listNeuron.at(marker_MST.listNeuron.at(i).parent).y;
//            V3DLONG z2 = marker_MST.listNeuron.at(marker_MST.listNeuron.at(i).parent).z;
//            double dis = sqrt(pow2(x1-x2) + pow2(y1-y2) + pow2(z1-z2));
//            if(dis>20)
//                marker_MST.listNeuron[i].parent = -1;
//        }
//    }

    QString outfilename = SWCfileName + "_boost_marker.swc";
    if (outfilename.startsWith("http", Qt::CaseInsensitive))
    {
        QFileInfo ii(outfilename);
        outfilename = QDir::home().absolutePath() + "/" + ii.fileName();
    }
    //v3d_msg(QString("The anticipated output file is [%1]").arg(outfilename));
    writeSWC_file(outfilename,marker_MST);
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

