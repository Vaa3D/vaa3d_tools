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
        QString braindir=(infiles.size()>=3)?infiles[2]:"";

        ofstream out;
        out.open(outfiles[2],ios::app);

        c.compare_two_swc(a,b,a_false,b_false,a_more,b_more,nt1,nt2,dir_a,dir_b,braindir,callback);

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
	else if (func_name == tr("help"))
	{
        cout<<"usage:"<<endl;
        cout<<"v3d -x compare_swc -f func2 -i [file_manual_swc] [file_auto_swc] [brain_path] -o [manual_more_folder] [auto_more_folder] [txt_file]"<<endl;

	}
	else return false;

	return true;
}

