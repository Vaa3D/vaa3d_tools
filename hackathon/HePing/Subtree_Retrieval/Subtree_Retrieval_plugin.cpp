/* Subtree_Retrieval_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2020-4-7 : by heping
 */
 
#include "v3d_message.h"
#include <vector>
#include<Windows.h>
#include<io.h>
#include<direct.h>
#include "Subtree_Retrieval_plugin.h"
#include "split_subtree.h"
using namespace std;
Q_EXPORT_PLUGIN2(Subtree_Retrieval, SubtreeRetrieval);
 
QStringList SubtreeRetrieval::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList SubtreeRetrieval::funclist() const
{
	return QStringList()
        <<tr("subtree_split")
        <<tr("kmeans_tree")
		<<tr("help");
}

void SubtreeRetrieval::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by heping, 2020-4-7"));
	}
}

bool SubtreeRetrieval::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("subtree_split"))
	{
        QString datafiles=(infiles.size()>=1)?infiles[0]:"";
        QString paramater=(infiles.size()>=2)?infiles[1]:"";
        QString outdir=(outfiles.size()>=1)?outfiles[0]:"";
        //判断输出目录是否存在，不存在则创建
        if(_access(outdir.toStdString().c_str(),0)==-1){
            _mkdir(outdir.toStdString().c_str());
        }

        vector<QString> swcfiles;
        //找到目录下所有的swc文件，文件名保持在swcfiles中
        find_all_files(datafiles,swcfiles);

        //遍历所有文件，并将每个神经元进行分割
        for(int i=0;i<swcfiles.size();i++){
            QString filePath=datafiles+"\\"+swcfiles[i];
            NeuronTree nt=readSWC_file(filePath);
            vector<bool> select_flag=vector<bool>(nt.listNeuron.size(),false);
            qDebug()<<filePath;
            vector<Subtree> sbtrees;
            int level=1;
            bool flag=SplitSubtree(nt,select_flag,paramater,level,sbtrees);
            while(flag==true){//存在没有被分割的结点
                qDebug()<<"*****************************";
                order++;
                flag=SplitSubtree(nt,select_flag,paramater,level,sbtrees);
            }
            NeuronTree nt1;
            nt1.listNeuron.clear();

            for(int j=0;j<sbtrees.size();j++){
                //  每个subtree单独写成一个swc文件，方便统计每个subtree的特征
                nt1.listNeuron=sbtrees[j].listNeuron;
                qDebug()<<outdir+"\\"+swcfiles[i]+"_"+QString::number(j)+".swc";
                writeESWC_file(outdir+"\\"+swcfiles[i]+"_"+QString::number(j)+".swc",nt1);

                //将每个subtree写入一个文件中
//                for(int k=0;k<sbtrees[j].listNeuron.size();k++){
//                    sbtrees[j].listNeuron[k].type=(j)%4+1;
//                    nt1.listNeuron.push_back(sbtrees[j].listNeuron[k]);
//                }




            }
            //将一个神经元的subtree用不同颜色表示，保存在一个文件中，是为了方便观察实验结果subtree的分割是否正确
            //writeESWC_file(outdir+"\\"+swcfiles[i]+"sbtree.swc",nt1);
        }



	}
    else if (func_name == tr("kmeans_tree"))
	{
        //利用subtree的特征进行聚类分析
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}

