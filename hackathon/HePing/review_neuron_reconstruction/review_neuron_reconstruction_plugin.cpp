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
		<<tr("menu1")
		<<tr("menu2")
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
	if (menu_name == tr("menu1"))
	{
		v3d_msg("To be implemented.");
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
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
    {   V3DLONG sz0=512,sz1=256,sz2=128;
        vector<NeuronSWC> candidate_point;
        QString swcfile=(infiles.size()>=1)?infiles[0]:"";//输入swc文件
        NeuronTree t = readSWC_file(swcfile);
        QString input_path = (infiles.size()>=2)?infiles[1]:"";//输入脑的位置
        SWCTreeSeg swcTree;
        //获取孩子节点
        swcTree.initialize(t);
        //在整个树中移动块，并在每个块中完成分段和平均灰度值的计算
        move_block(input_path,callback,t,swcTree,sz0,sz1,sz2,candidate_point);//移动块获取候选点分段
        //对所有的段进行排序(目前只有一个块中的段)
        sequence_rule(swcfile,swcTree,t,swcTree.children);

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

