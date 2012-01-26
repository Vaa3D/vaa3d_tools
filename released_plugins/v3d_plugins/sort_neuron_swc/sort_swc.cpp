/*
 *  sort_swc.cpp
 *  sort_swc 
 *
 *  Created by Wan, Yinan, on 06/20/11.
 *  Last change: Wan, Yinan, on 06/23/11.
 */

#include <QtGlobal>
#include <math.h>
#include "sort_swc.h"
#include "v3d_message.h" 
#include <unistd.h>
#include <string.h>
#include <iostream>
using namespace std;


#define VOID 1000000000

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(sort_swc, SORT_SWCPlugin);


//plugin funcs
const QString title = "sort_swc";
QStringList SORT_SWCPlugin::menulist() const
{
    return QStringList() 
	<<tr("sort_swc")
	<<tr("Help");
}

QStringList SORT_SWCPlugin::funclist() const
{
	return QStringList()
		<<tr("sort_swc")
		<<tr("Help");
}

QHash<V3DLONG, V3DLONG> ChildParent(const NeuronTree &neurons, const QList<V3DLONG> & idlist, const QHash<V3DLONG,V3DLONG> & LUT) 
{
	QHash<V3DLONG, V3DLONG> cp;
	for (V3DLONG i=0;i<neurons.listNeuron.size(); i++)
		if (neurons.listNeuron.at(i).pn==-1) cp.insertMulti(idlist.indexOf(LUT.value(neurons.listNeuron.at(i).n)), -1);
		else cp.insertMulti(idlist.indexOf(LUT.value(neurons.listNeuron.at(i).n)), idlist.indexOf(LUT.value(neurons.listNeuron.at(i).pn))); 
	return cp;
}

QHash<V3DLONG, V3DLONG> getUniqueLUT(const NeuronTree &neurons)
{
	QHash<V3DLONG,V3DLONG> LUT;
	for (V3DLONG i=0;i<neurons.listNeuron.size();i++)
	{
		V3DLONG j;
		for (j=0;j<i;j++)
		{
			if (neurons.listNeuron.at(i).x==neurons.listNeuron.at(j).x && neurons.listNeuron.at(i).y==neurons.listNeuron.at(j).y && neurons.listNeuron.at(i).z==neurons.listNeuron.at(j).z)		break;
		}
		
		LUT.insertMulti(neurons.listNeuron.at(i).n,j);
	}
	return (LUT);
}


void DFS(bool** matrix, V3DLONG* neworder, V3DLONG node, V3DLONG* id, V3DLONG siz, int* numbered, int *group)
{
	numbered[node] = *group;
	neworder[*id] = node;
	(*id)++;
	for (V3DLONG v=0;v<siz;v++)
		if (!numbered[v] && matrix[v][node])
		{
			DFS(matrix, neworder, v, id, siz,numbered,group);
		}
}

double computeDist(const NeuronSWC & s1, const NeuronSWC & s2)
{
	double xx = s1.x-s2.x;
	double yy = s1.y-s2.y;
	double zz = s1.z-s2.z;
	return (sqrt(xx*xx+yy*yy+zz*zz));
}
	
bool SortSWC(const NeuronTree & neurons, QList<NeuronSWC> & lN, V3DLONG newrootid)
{
	//create a LUT, from the original id to the position in the listNeuron, different neurons with the same x,y,z & r are merged into one position
	QHash<V3DLONG, V3DLONG> LUT = getUniqueLUT(neurons);

	//create a new id list to give every different neuron a new id		
	QList<V3DLONG> idlist = ((QSet<V3DLONG>)LUT.values().toSet()).toList();

	//create a child-parent table, both child and parent id refers to the index of idlist
	QHash<V3DLONG, V3DLONG> cp = ChildParent(neurons,idlist,LUT);

	
	V3DLONG siz = idlist.size();

	
	bool** matrix = new bool*[siz];
	for (V3DLONG i = 0;i<siz;i++)
	{
		matrix[i] = new bool[siz];
		for (V3DLONG j = 0;j<siz;j++) matrix[i][j] = false;
	}
	

	//generate the adjacent matrix for undirected matrix
	for (V3DLONG i = 0;i<siz;i++)
	{
		QList<V3DLONG> parentSet = cp.values(i); //id of the ith node's parents
		for (V3DLONG j=0;j<parentSet.size();j++)
		{
			V3DLONG v2 = (V3DLONG) (parentSet.at(j));
			if (v2==-1) continue;
			matrix[i][v2] = true;
			matrix[v2][i] = true;
		}
	}
	
	
	//do a DFS for the the matrix and re-allocate ids for all the nodes
	V3DLONG root;
	if (newrootid==VOID)
	{
		for (V3DLONG i=0;i<neurons.listNeuron.size();i++)
			if (neurons.listNeuron.at(i).pn==-1){
				root = idlist.indexOf(LUT.value(neurons.listNeuron.at(i).n));
				break;
			}
	}
	else{
		root = idlist.indexOf(LUT.value(newrootid));
	
		if (LUT.keys().indexOf(newrootid)==-1)
		{
			v3d_msg("The new root id you have chosen does not exist in the SWC file.");
			return(false);
		}
	}

	
	V3DLONG* neworder = new V3DLONG[siz];
	int* numbered = new int[siz];
	for (V3DLONG i=0;i<siz;i++) numbered[i] = 0;
	
	V3DLONG id[] = {0};

	int group[] = {1};
	DFS(matrix,neworder,root,id,siz,numbered,group);

	while (*id<siz)
	{
		V3DLONG iter;
		(*group)++;
		for (iter=0;iter<siz;iter++)
			if (numbered[iter]==0) break;
		DFS(matrix,neworder,iter,id,siz,numbered,group);
	}

	if ((*group)>1)
		v3d_msg(QString("Warning: The input neuron is not connected and broken into %1 parts. The program will automatically generate %2 links to merge it into one piece.").arg(*group).arg((*group)-1));

	//find the point in non-group 1 that is nearest to group 1, 
	//include the nearest point as well as its neighbors into group 1, until all the nodes are connected
	while((*group)>1)
	{
		double min = VOID;
		double dist = 0;
		int mingroup = 1;
		V3DLONG m1,m2;
		for (V3DLONG ii=0;ii<siz;ii++){
			if (numbered[ii]==1)
				for (V3DLONG jj=0;jj<siz;jj++)
					if (numbered[jj]!=1)
					{
						dist = computeDist(neurons.listNeuron.at(idlist.at(ii)),neurons.listNeuron.at(idlist.at(jj)));
						if (dist<min)
						{
							min = dist;
							mingroup = numbered[jj];
							m1 = ii;
							m2 = jj;
						}
					}
		}
		for (V3DLONG i=0;i<siz;i++)
			if (numbered[i]==mingroup)
				numbered[i] = 1;
		matrix[m1][m2] = true;
		matrix[m2][m1] = true;
		(*group)--;
	}

	id[0] = 0;
	for (int i=0;i<siz;i++)
		numbered[i] = 0;
	*group = 1;
	DFS(matrix,neworder,root,id,siz,numbered,group);

	if ((*id)<siz) {
		v3d_msg("Error!");
		return false;
	}
	

	NeuronSWC S;
	S.n = 1;
	S.pn = -1;
	V3DLONG oripos = idlist.at(root);
	S.x = neurons.listNeuron.at(oripos).x;
	S.y = neurons.listNeuron.at(oripos).y;
	S.z = neurons.listNeuron.at(oripos).z;
	S.r = neurons.listNeuron.at(oripos).r;
	S.type = neurons.listNeuron.at(oripos).type;
	lN.append(S);

	for (V3DLONG ii = 1;ii<siz;ii++)
	{
		for (V3DLONG jj=0;jj<ii;jj++) //after DFS the id of parent must be less than child's
		{
			if (matrix[neworder[ii]][neworder[jj]]) 
			{
				NeuronSWC S;
				S.n = ii+1;
				S.pn = jj+1;
				V3DLONG oripos = idlist.at(neworder[ii]);
				S.x = neurons.listNeuron.at(oripos).x;
				S.y = neurons.listNeuron.at(oripos).y;
				S.z = neurons.listNeuron.at(oripos).z;
				S.r = neurons.listNeuron.at(oripos).r;
				S.type = neurons.listNeuron.at(oripos).type;
				lN.append(S);
			}
		}
	}

	//free space by Yinan Wan, 2012-01-26
	if (neworder) {delete []neworder; neworder=NULL;}
	if (numbered) {delete []numbered; numbered=NULL;}
	if (matrix)
	{
		for (int i=0;i<siz;i++)
			if (matrix[i]) {delete matrix[i]; matrix[i]=NULL;}
		if (matrix) {delete []matrix; matrix=NULL;}
	}
	return(true);
}

void sort_swc(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	NeuronTree neuron;
	QString fileOpenName;
	if (method_code == 1)
	{
		fileOpenName = QFileDialog::getOpenFileName(0, QObject::tr("Open File"),
				"",
				QObject::tr("Supported file (*.swc)"
					";;Neuron structure	(*.swc)"
					));
		if(fileOpenName.isEmpty()) 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}


		if (fileOpenName.size()>0)
		{
			neuron = readSWC_file(fileOpenName);
			QList<NeuronSWC> lN;		
			int rootid;
			bool ok;
			rootid = QInputDialog::getInteger(parent, "Would you like to specify new root number?","New root number:(If you select 'cancel', the first root in file is set as default)",1,1,neuron.listNeuron.size(),1,&ok);
			if (!ok)
				rootid = VOID;
			if (SortSWC(neuron, lN,rootid)){

				//parse fileOpenName
				int parser;
				for (parser=fileOpenName.length()-1;parser>=0;parser--)
					if (fileOpenName.at(parser)==QChar('.')) break;
				QString fileDefaultName = fileOpenName;
				fileDefaultName.insert(parser,QString("_sorted"));


				//write new SWC to file
				QString fileSaveName = QFileDialog::getSaveFileName(0, QObject::tr("Save File"),
						fileDefaultName,
						QObject::tr("Supported file (*.swc)"
							";;Neuron structure	(*.swc)"
							));
				QFile file(fileSaveName);
				file.open(QIODevice::WriteOnly|QIODevice::Text);
				QTextStream myfile(&file);
				for (V3DLONG i=0;i<lN.size();i++)
					myfile << lN.at(i).n <<" " << lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

				file.close();
			}
		}
		else 
		{
			v3d_msg("You don't have any SWC file open in the main window.");
			return;
		}

	}
}

void SORT_SWCPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("sort_swc"))
	{
		sort_swc(callback, parent,1 );
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("(version 0.15) Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's. If the original SWC has more than one connected components,  the plugin will automatically link the nearest points. It also includes a merging process of neuron segments, where neurons with the same x,y,z cooridinates are combined as one.");
		return;
	}
}

bool sort_func(const V3DPluginArgList & input, V3DPluginArgList & output)
{
	cout<<"==========Welcome to sort_swc function============="<<endl;
	vector<char*>* inlist = (vector<char*>*)(input.at(0).p);
	vector<char*>* outlist;
	vector<char*>* paralist;

	int rootid;
	bool hasPara, hasOutput;
	if (input.size()==1) 
	{
		cout<<"No new root id specified.\n";
		rootid = VOID;
		hasPara = false;
	}
	else {
		hasPara = true;
		paralist = (vector<char*>*)(input.at(1).p);
	}

	int neuronNum = inlist->size();

	cout<<"input size: "<<neuronNum<<"\n";
	if (output.size()==0){
		cout<<"No output file specified.\n";
		hasOutput = false;
		}
	else {
		hasOutput = true;
		 outlist = (vector<char*>*)(output.at(0).p);
		if (neuronNum!=outlist->size())
		{
			cout<<"input and output file number are not same, please check your input.\n";
			return false;
		}
	}


	if (hasPara)
		if (paralist->size()!=neuronNum)
		{
			cout<<"number of input files must match number of parameters, please check your input\n";
			return false;
		}

	for (int iter=0;iter<neuronNum;iter++)
	{
		QString fileOpenName = QString(inlist->at(iter));
		QString fileSaveName;

		cout<<"---------Neuron #1----------\n";
		cout<<"infile: "<<inlist->at(iter)<<endl;
		if (hasOutput){
			cout<<"outfile: "<<outlist->at(iter)<<endl;
			fileSaveName = QString(outlist->at(iter));
		}
		else {
				int parser;
				for (parser=fileOpenName.length()-1;parser>=0;parser--)
					if (fileOpenName.at(parser)==QChar('.')) break;
				fileSaveName = fileOpenName;
				fileSaveName.insert(parser,QString("_sorted"));
				v3d_msg(QString("outfile: %1").arg(fileSaveName),0);		
		}

		if (hasPara)
		{
			char * paras = paralist->at(iter);
			rootid = 0;
			for (int i=0;i<strlen(paras);i++)
			{
				rootid = rootid*10 + paras[i]-'0';
			}
			cout<<"new root id: "<<rootid<<endl;
		}

		if(fileOpenName.isEmpty()) 
		{
			cout<<"you specified a wrong file name."<<endl;
			return false;
		}

		NeuronTree neuron;
		if (fileOpenName.size()>0)
		{
			neuron = readSWC_file(fileOpenName);
			QList<NeuronSWC> lN;			
			if (SortSWC(neuron, lN,rootid)){
				//write new SWC to file
				QFile file(fileSaveName);
				file.open(QIODevice::WriteOnly|QIODevice::Text);
				QTextStream myfile(&file);
				for (V3DLONG i=0;i<lN.size();i++)
					myfile << lN.at(i).n <<" "<< lN.at(i).type << " "<< lN.at(i).x <<" "<<lN.at(i).y << " "<< lN.at(i).z << " "<< lN.at(i).r << " " <<lN.at(i).pn << "\n";

				file.close();
			}	
		}
		else 
		{
			cout<<"You don't have any SWC file open in the main window."<<endl;
			return false;
		}
	}
	return true;

}



bool SORT_SWCPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback, QWidget * parent)
{
	if (func_name==tr("sort_swc"))
	{
		sort_func(input,output);
		return true;
	}
	if (func_name==tr("Help"))
	{
		cout<<"(version 0.15) Set a new root and sort the SWC file into a new order, where the newly set root has the id of 1, and the parent's id is less than its child's. If the original SWC has more than one connected components, the plugin will automatically link the nearest points. It also includes a merging process of neuron segments, where neurons with the same x,y,z cooridinates are combined as one.\n";
		cout<<"usage:\n";
		cout<<"-i<file name>:\ta list of input swc file(s)\n";
		cout<<"-o<file name>:\t (not required) a list specifying output sorted swc file(s), corresponding to input file. If you don't specify this, the program will generate an swc file named 'inputName_sorted.swc' within the same directory as the input file\n";
		cout<<"-p<root number>: (not required) a list showing new root id. If you don't specify this, the plugin will set the first root in the input file as new root.\n";
		cout<<"Demo:\t ./v3d -x plugins/neuron_uitilities/sort_a_swc/libsort_swc.so -i test.swc -o test_sorted.swc -p 1\n";
		return true;
	}

	return false;
}

