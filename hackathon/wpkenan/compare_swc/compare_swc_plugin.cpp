/* compare_swc_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-6-24 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "compare_swc_plugin.h"
#include "n_class.h"
#include <fstream>

using namespace std;
Q_EXPORT_PLUGIN2(compare_swc, TestPlugin);
 
QStringList TestPlugin::menulist() const
{
	return QStringList() 
		<<tr("menu1")
		<<tr("menu2")
		<<tr("about");
}

QStringList TestPlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void TestPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
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
			"Developed by YourName, 2019-6-24"));
	}
}

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	cout << "wp_debug:" << __LINE__ << endl;
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
    {
        cout<<"111"<<endl;
        const char* f=(infiles.size()>=1)?infiles[0]:"";
        const QString file(f);
        NeuronTree nt=readSWC_file(file);
        SwcTree tree;
        tree.initialize(nt);
        cout<<"segments size:"<<tree.branchs.size()<<endl;
        //tree.segments_level_size();
        tree.display();

//        tree.find_big_turn();

//        cout<<"111"<<endl;
//        tree.cut_cross();
//        cout<<"222"<<endl;
//        tree.branchs_to_nt();
//        cout<<"!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<endl;
//        cout<<"segments size:"<<tree.branchs.size()<<endl;
//        //tree.segments_level_size();
//        tree.display();
//        writeESWC_file("D://1.eswc",tree.nt);



	}
	else if (func_name == tr("func2"))
    {
        const QString swc1=(infiles.size()>=1)?infiles[0]:"";
        const QString swc2=(infiles.size()>=2)?infiles[1]:"";
        NeuronTree nt1=readSWC_file(swc1);
        NeuronTree nt2=readSWC_file(swc2);
        SwcTree a,b;
        a.initialize(nt1);
        b.initialize(nt2);
        //a.display();
        //b.display();
        Swc_Compare c;
        vector<int> a_false,b_false,a_more,b_more;
//        int mode=0;

        QString dir_a=(outfiles.size()>=1)?outfiles[0]:"";
        QString dir_b=(outfiles.size()>=2)?outfiles[1]:"";
        QString file=(outfiles.size()>=3)?outfiles[2]:"";

        QString apofile=dir_a+"/mmm.apo";
        QString braindir=(infiles.size()>=3)?infiles[2]:"";

        ofstream out;
        out.open(outfiles[2],ios::app);

        c.compare_two_swc(a,b,a_false,b_false,a_more,b_more,nt1,nt2,dir_a,dir_b,braindir,callback);

        cout<<"finish compare_two_swc"<<endl;

        // added 15 Jul
        c.get_false_point_image(dir_a,a_more,a,b,callback,braindir,true);
        c.get_false_point_image(dir_b,b_more,b,a,callback,braindir,false);

        cout<<"1111111111"<<endl;
        a.branchs_to_nt(a_more);
        b.branchs_to_nt(b_more);

        cout<<"2222222222"<<endl;

        QString a_swc=dir_a+"/manual.eswc";
        QString b_swc=dir_b+"/auto.eswc";

        for(V3DLONG i=0;i<a.nt.listNeuron.size();++i)
        {

            if(a.nt.listNeuron[i].type!=5)
            {
                a.nt.listNeuron[i].type=3;
            }
            else
            {
                a.nt.listNeuron[i].type=6;
            }

        }

        writeSWC_file(a_swc,a.nt);
        writeSWC_file(b_swc,b.nt);

        cout<<"33333333333"<<endl;

        QList<CellAPO> markers;
        for(int i=0;i<a_more.size();++i)
        {
            NeuronSWC tmp=a.branchs[a_more[i]].head_point;
            CellAPO marker;
            marker.x=tmp.x;
            marker.y=tmp.y;
            marker.z=tmp.z;
            marker.color.r=255;
            marker.color.g=0;
            marker.color.b=0;
            marker.volsize=5;
            markers.push_back(marker);
        }
        for(int i=0;i<b_more.size();++i)
        {
            NeuronSWC tmp=b.branchs[b_more[i]].head_point;
            CellAPO marker;
            marker.x=tmp.x;
            marker.y=tmp.y;
            marker.z=tmp.z;
            marker.color.r=0;
            marker.color.g=255;
            marker.color.b=0;
            marker.volsize=5;
            markers.push_back(marker);
        }
        cout<<"44444444444"<<endl;

        writeAPO_file(apofile,markers);

        out<<"manual_more:"<<endl;
        out<<"manual_max_level: "<<a.get_max_level()<<endl;
        out<<"index"<<" branch_index"<<" level"<<" length"<<endl;

        for(int i=0;i<a_more.size();++i)
            out<<i<<" "<<a_more[i]<<" "<<a.branchs[a_more[i]].level<<" "<<a.branchs[a_more[i]].length<<endl;

        out<<"auto_more:"<<endl;
        out<<"auto_max_level: "<<b.get_max_level()<<endl;
        out<<"index"<<" branch_index"<<" level"<<" length"<<endl;
        for(int i=0;i<b_more.size();++i)
            out<<i<<" "<<b_more[i]<<" "<<b.branchs[b_more[i]].level<<" "<<b.branchs[b_more[i]].length<<endl;
//        mode=2;

//        c.get_sub_image(dir_a,a_more,a,b,braindir,callback,mode,);
//        mode=1;
//        c.get_sub_image(dir_b,b_more,b,braindir,callback,mode);
//        c.get_sub_false_trunk_image(dir,a_false,b_false,a,b,braindir,callback);


	}
    else if (func_name== tr("globalcompare"))
    {
        NeuronTree nt0= readSWC_file(infiles[0]);//auto
        NeuronTree nt1= readSWC_file(infiles[1]);//manual
        QString braindir=inparas[0];
        QString outdir=outfiles[0];

        //qDebug()<<"reach here";
        Swc_Compare c;
        SwcTree a,b;
        a.initialize(nt0);
        b.initialize(nt1);
        c.global_compare(a,b,braindir,outdir,callback);
    }
    else if(func_name==tr("func3"))
    {
        if (infiles.size()!=3) fprintf(stderr,"please specify three inputs swc1 swc2 brainfolder");
        QString swc1= infiles[0];
        QString swc2= infiles[1];
        QString braindir=infiles[2];
        NeuronTree nt1=readSWC_file(swc1);
        NeuronTree nt2=readSWC_file(swc2);
        SwcTree a,b;// e.g. a for manual swc, b for auto swc
        a.initialize(nt1);
        b.initialize(nt2);

        Swc_Compare c;

        if( outfiles.size()!=3) fprintf(stderr, "please specify three outputs folder1 folder2 folder3");
        QString dir_a=outfiles[0];
        QString dir_b=outfiles[1];
        QString dir_ab=outfiles[2];
        //QString txtfile=outfiles[3];

        vector<int> a_false, b_false,a_more,b_more;
        c.compare_two_swc(a,b,a_false,b_false,a_more,b_more,nt1,nt2,dir_a,dir_b,braindir,callback);

        //c.get_accurate_false_point_image(dir_a,a_false,a,b,callback,braindir,true);
        c.get_false_point_image(dir_a,a_more,a,b,callback,braindir,true);
        c.get_false_point_image(dir_b,b_more,b,a,callback,braindir,false);
        c.get_accurate_false_point_image(dir_ab,b_false,b,a,callback,braindir,false);
    }
    else if(func_name==tr("refine"))
    {
		cout << "wp_debug:" << __LINE__ << endl;
        NeuronTree nt1=readSWC_file(infiles[0]);
        QString braindir= infiles[1];
        SwcTree a;
        a.initialize(nt1);
		cout << "wp_debug: " << __LINE__ << endl;
        NeuronTree refinetree = a.refine_swc_by_bdb(braindir,callback);
        QString eswcfile = (outfiles.size()>=1) ? outfiles[0] : "";
        writeESWC_file(eswcfile,refinetree);

    }

    else if (func_name == tr("help"))
    {
        cout<<"usage:"<<endl;
        cout<<"v3d -x compare_swc -f func2 -i [file_manual_swc] [file_auto_swc] [brain_path] -o [manual_more_folder] [auto_more_folder] [txt_file]"<<endl;

	}
	else return false;

	return true;
}

