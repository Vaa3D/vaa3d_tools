/* segment_block_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2018-7-1 : by Yongzhang
 */
 
#include "v3d_message.h"
#include <vector>
#include "basic_surf_objs.h"
#include "my_surf_objs.h"
#include "../../../released_plugins/v3d_plugins/sort_neuron_swc/openSWCDialog.h"

#include "segment_block_plugin.h"
#include "segment_block.h"
Q_EXPORT_PLUGIN2(segment_block, TestPlugin);
extern V3DLONG o_x;
extern V3DLONG o_y;
extern V3DLONG o_z;

using namespace std;

struct input_PARA
{
    QString inimg_file;
    V3DLONG channel;
};

 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
        <<tr("segment_block")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
        <<tr("segment_block")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("segment_block"))
	{
        OpenSWCDialog * openDlg=new OpenSWCDialog(0,&callback);
        if(!openDlg->exec())
            return;
        NeuronTree nt,nt_p;
        nt_p = openDlg->nt;
        QString filename = openDlg->file_name;
        cout<<filename.toStdString()<<endl;


        //NeuronTree input,result;
        Tree tree;
        for(V3DLONG i=0;i<nt_p.listNeuron.size();i++)
        {
            NeuronSWC S=nt_p.listNeuron[i];
            Point* pt= new Point;
            pt->x = S.x;
            pt->y = S.y;
            pt->z = S.z;
            pt->r = S.r;
            pt->type = S.type;
            pt->p = NULL;
            pt->childNum = 0;
            tree.push_back(pt);

        }
        cout<<"tree.size="<<tree.size()<<endl;

        for(V3DLONG i=0;i<nt_p.listNeuron.size();i++)
        {
            if (nt_p.listNeuron[i].pn<0) continue;
            V3DLONG pid = nt_p.hashNeuron.value(nt_p.listNeuron[i].pn);
            tree[i]->p = tree[pid];
            tree[pid]->childNum++;
        }
        //	printf("tree constructed.\n");
        vector<Segment*> seg_list;
        for(V3DLONG i=0;i<nt_p.listNeuron.size();i++)
        {
            if (tree[i]->childNum!=1)//tip or branch point
            {
                Segment* seg = new Segment;
                Point* cur = tree[i];
                do
                {
                    seg->push_back(cur);
                    cur = cur->p;
                }
                while(cur && cur->childNum==1);
                seg_list.push_back(seg);
            }
        }
        cout<<"seg_list.size="<<seg_list.size()<<endl;
        cout<<"tree="<<tree.size()<<endl;
        PARA PA;
        PA.data1d = 0;

        //Check segments
//        QList<NeuronSWC> seg_result;
//        for(V3DLONG i=0;i<seg_list.size();i++)
//        {
//            seg_result.clear();
//            for(V3DLONG j=0;j<seg_list[i]->size();j++)
//            {
//                NeuronSWC S;
//                Point* pt= new Point;
//                pt=seg_list[i]->at(j);
//                S.n=j;
//                S.parent=j-1;
//                S.x=pt->x;
//                S.y=pt->y;
//                S.z=pt->z;
//                S.r=pt->r;
//                S.type=pt->type;

//                seg_result.push_back(S);

//            }
//            NeuronTree seg_nt;
//            seg_nt.listNeuron=seg_result;
//            filename=QString("segment"+QString::number(i)+".swc");
//            writeSWC_file(filename,seg_nt);

//        }
        segment_block(callback,seg_list,PA);

	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
            "Developed by Yongzhang, 2018-7-1"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    vector<char*> infiles, inparas, outfiles;
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("func1"))
    {
        v3d_msg("To be implemented.");
    }
    if (func_name == tr("func2"))
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

