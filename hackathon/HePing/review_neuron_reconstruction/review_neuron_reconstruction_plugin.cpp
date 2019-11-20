/* review_neuron_reconstruction_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-9-21 : by heping
 */
 
#include "v3d_message.h"
#include <vector>
#include "review_neuron_reconstruction_plugin.h"
#include "review.h"
using namespace std;
Q_EXPORT_PLUGIN2(review_neuron_reconstruction, reviewNReconstruction);

QStringList reviewNReconstruction::menulist() const
{
	return QStringList() 
        <<tr("segment sequence")
		<<tr("show segment")
		<<tr("about");
}

QStringList reviewNReconstruction::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void reviewNReconstruction::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("segment sequence"))
	{
        review_neuron(callback,parent);
	}
	else if (menu_name == tr("show segment"))
	{
        show_dialog(callback,parent);
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by heping, 2019-9-21"));
	}
}

bool reviewNReconstruction::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
    {   V3DLONG sz0=512,sz1=512,sz2=256;
        vector<NeuronSWC> candidate_point;
        QList<CellAPO> markers;
        NeuronTree new_sequence_tree;
        QString swcfile=(infiles.size()>=1)?infiles[0]:"";//输入swc文件
        NeuronTree t = readSWC_file(swcfile);
        QString input_path = (infiles.size()>=2)?infiles[1]:"";//输入脑的位置
        QString save_folder = "C://Users//penglab//Desktop//17302-00001//review_test";
        SWCTreeSeg swcTree;
        //获取孩子节点
        swcTree.initialize(t);
        //在整个树中移动块，并在每个块中完成分段和平均灰度值的计算
        move_block(input_path,callback,new_sequence_tree,markers,t,swcTree,sz0,sz1,sz2,candidate_point);//移动块获取候选点分段
        new_sequence_tree.listNeuron.push_back(t.listNeuron[t.hashNeuron.value(swcTree.root.n)]);
        //写文件
        QFileInfo eswcfileinfo;
        eswcfileinfo=QFileInfo(swcfile);
        QString eswcfile=eswcfileinfo.fileName();
        eswcfile.mid(0,eswcfile.indexOf("."));
        //nt1.listNeuron.push_back(orig.listNeuron[orig.hashNeuron.value(swcTree.root.n)]);
        writeESWC_file(save_folder+"//"+eswcfile+".eswc",new_sequence_tree);
        //writeAPO_file("C://Users//penglab//Desktop//17302-00001//review_test1//marker.apo",markers);
        writeAPO_file(save_folder+"//"+eswcfile+".apo",markers);

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

