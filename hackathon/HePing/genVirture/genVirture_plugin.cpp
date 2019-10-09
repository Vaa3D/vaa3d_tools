/*
genVirture_plugin.cpp
Tue Mar 26 14:52:35 2019 by heping
*/

#include "v3d_message.h"
#include <vector>
#include "genVirture_plugin.h"
#include<iostream>
#include <QString>
#include <QtCore/QFile>

using namespace std;

Q_EXPORT_PLUGIN2(genVirture,genVirturePlugin);
void function(V3DPluginCallback2 &callback,QWidget *parent);
bool function(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent);
const QString title = QObject::tr("genVirture");
void generate_virture();
void configure();
bool pruning_false_branch(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output);//用于修剪重建结果的cross-by中的false positive
bool generate_virture1(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output, QWidget *parent);//暂时不用这个，命令行运行时使用的函数
const QString filename1 = "D:/gen_vir_experiment_code";
const QString filename2 = "D:/test.swc";
const QStringList *infostring;
QStringList genVirturePlugin::menulist() const{//菜单
	return QStringList()
		<< tr("generate_virture")//生成神经元菜单
		<< tr("pruning_false_branch")
		<< tr("configure");//用户用来输入一些自定义参数的调整，目前暂无 date 3/26
}

QStringList genVirturePlugin::funclist() const{//函数
	return QStringList()
		<< tr("generate_virture")
		<< tr("pruning_false_branch");
}

void genVirturePlugin::domenu(const QString &menu_name,V3DPluginCallback2 &callback,QWidget *parent){
   if(menu_name==tr("generate_virture")){
       generate_virture();
   }
   else if (menu_name == tr("configure")){
	   configure();
   }
   else{
       v3d_msg(tr("developing...Tue Mar 26 14:52:35 2019 by heping"));
   }
}

bool genVirturePlugin::dofunc(const QString &func_name,const V3DPluginArgList &input,V3DPluginArgList &output,V3DPluginCallback2 &callback,QWidget *parent){
   if(func_name==tr("generate_virture")){
	   return generate_virture1(callback,input, output, parent);
   }
   else if(func_name==tr("pruning_false_branch")){
	   return pruning_false_branch(callback,input,output);
   }
   return false;
}

bool pruning_false_branch(V3DPluginCallback2 &callback, const V3DPluginArgList &input, V3DPluginArgList &output){//pruning false positive branch in cross_by
	
}


void generate_virture(){//调用生成虚拟神经元的函数	
	QStringList filenames = QFileDialog::getOpenFileNames(0, 0, "", "support file(*.swc)", 0, 0);
	vector<NeuronTree> ntrees;
	QList <NeuronSWC> new_listNeuron_denr;
	QList <NeuronSWC> new_listNeuron_axon;
	if (filenames.isEmpty()){
		v3d_msg("You don't chose any swc file!");
		return;
	}
	for (int j = 0; j < filenames.size(); j++){
		NeuronTree ntree = readSWC_file(filenames[j]);
		V3DLONG neuron_size = ntree.listNeuron.size();
		vector <V3DLONG> nodes = vector<V3DLONG>(neuron_size, 0);
		//将轴突和树突分开，保存节点的孩子节点数量
		calculate_nodes(ntree, nodes);
		QList <NeuronSWC> new_listNeuron;
		save_point_nodes(ntree, nodes, new_listNeuron);
		ntree.listNeuron = new_listNeuron;
		QString file = filename1 + "/" + j + ".swc";
		writeSWC_file(file, ntree, infostring);
		cout << "write a swc file" << endl;
	}
	return;
	NeuronTree ntree = readSWC_file(filenames[0]);
	V3DLONG neuron_size = ntree.listNeuron.size();
	vector <V3DLONG> nodes=vector<V3DLONG>(neuron_size,0);
//	vector <V3DLONG> nodes_axon=vector<V3DLONG>(neuron_size,0);
	NeuronSWC seed;//SWC文件中第一个点就是soma
	seed = ntree.listNeuron[0];
	cout << "----------------------" << endl;
	
	//将轴突和树突分开，保存节点的孩子节点数量
	calculate_nodes(ntree, nodes);
	//将树突的关键节点保存下来,将轴突的关键节点保存下来
	//save_import_nodes(ntree,nodes,new_listNeuron_denr,new_listNeuron_axon);
	QList <NeuronSWC> new_listNeuron;
	save_point_nodes(ntree, nodes, new_listNeuron);
	cout << "save  finish!" << endl;
//	cout << new_listNeuron_axon.size() <<"--------" <<new_listNeuron_denr.size() << endl;
	cout << "--------------------------" << endl;
	//最小生成树算法prim，种子点的编号还是1
//	min_tree_prim(new_listNeuron_denr);
//	min_tree_prim(new_listNeuron_axon);
	//ntree.listNeuron = new_listNeuron_denr;
	ntree.listNeuron = new_listNeuron;
	cout << "wirte swc--------------------" << endl;
	
	if (writeSWC_file(filename1, ntree, infostring)){
		v3d_msg("finish it gendenrite");
	}
	else{
		cout << "write error!!!!" << endl;
	}
	cout << "start random algorithm!" << endl;

//	gen_axon(new_listNeuron_denr);
	ntree.listNeuron = new_listNeuron_axon;
	if (writeSWC_file(filename2, ntree, infostring)){
		v3d_msg("finish it");
	}
	else{
		cout << "write error!!!!" << endl;
	}
	//布线成本和总路径成本
	//其他参数影响：分子浓度、体积、竞争------------暂不考虑
	//段的直径逐渐减小-------暂不考虑
}

void configure(){};//暂时不管参数设置，先用固定输入某些参数

bool generate_virture1(V3DPluginCallback2 &callback,const V3DPluginArgList &input,V3DPluginArgList &output,QWidget *parent){//暂时不用
   vector<char*> *pinfiles=(input.size()>=1)?(vector<char*> *) input[0].p : 0;
   vector<char*> * poutfiles = (output.size() >= 1) ? (vector<char*> *) output[0].p : 0;
   vector<char*> * pparas = (input.size() >= 2) ? (vector<char*> *) input[1].p : 0;
   vector<char*> infiles = (pinfiles != 0) ? * pinfiles : vector<char*>();
   vector<char*> outfiles = (poutfiles != 0) ? * poutfiles : vector<char*>();
   vector<char*> paras = (pparas != 0) ? * pparas : vector<char*>();
   return true;
}

