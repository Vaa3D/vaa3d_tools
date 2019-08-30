/* re_xtracing_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-6-18 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "re_xtracing_plugin.h"
#include "some_function.h"
using namespace std;
Q_EXPORT_PLUGIN2(re_xtracing, TestPlugin);
 
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
        if(callback.getImageWindowList().empty())
        {
            v3d_msg("Please open an image first");
            return;
        }
        PARA_T p;
        if(!p.initialize(callback))
            return;
        re_xtrace(callback,p);
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-6-18"));
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
        cout<<"start"<<endl;
        unsigned char* pdata=0;
        V3DLONG sz[4]={0,0,0,0};
        int datatype=0;
        simple_loadimage_wrapper(callback,infiles[0],pdata,sz,datatype);
        cout<<"11"<<endl;
        double thres=(inparas.size()>=1)?(double)atoi(inparas[0]):25.0;
        bool is_gsdt=(inparas.size()>=2)?(bool)atoi(inparas[1]):true;
        float* phi=0;
        long* plist=0;
        int cnn_type=3;
        cout<<"222"<<endl;
        if(is_gsdt)
        {
            cout<<"in is_gsdt"<<endl;
            fastmarching_dt(pdata,phi,sz[0],sz[1],sz[2],cnn_type,thres);

            cout<<"end fastmarching_dt"<<endl;

            fm_find_segs(phi,plist,sz[0],sz[1],sz[2],cnn_type,thres);

            cout<<"end fm_find_segs"<<endl;
        }
        else
        {
            cout<<"in not"<<endl;
            fm_find_segs(pdata,plist,sz[0],sz[1],sz[2],cnn_type,thres);
        }
        vector<unit_seg> segs;

        cout<<"before test........"<<endl;

        for(long i=0;i<sz[0]*sz[1]*sz[2];++i)
        {
            if(plist[i]>=sz[0]*sz[1]*sz[2])
                cout<<plist[i];
        }
        cout<<"test......"<<endl;

        trans_segs(plist,segs,sz[0],sz[1],sz[2]);
        cout<<"end trans_segs"<<endl;
        calculate_segs_radius(pdata,sz,segs,thres);
        cout<<"end calculate_segs_radius"<<endl;
        NeuronTree nt;
        for(long i=0;i<segs.size();++i)
        {
            for(long j=0;j<segs[i].seg_tree.size();++j)
            {
                NeuronSWC tmp;
                tmp.n=segs[i].seg_tree[j].n;
                tmp.x=segs[i].seg_tree[j].x;
                tmp.y=segs[i].seg_tree[j].y;
                tmp.z=segs[i].seg_tree[j].z;
                tmp.parent=segs[i].seg_tree[j].parent;
                tmp.r=segs[i].seg_tree[j].r;
                nt.listNeuron.push_back(tmp);
            }
        }
        writeESWC_file("D://test.eswc",nt);
        for(long i=0;i<segs.size();++i)
        {
            for(long j=0;j<segs[i].seg_tree.size();++j)
            {
                segs[i].seg_tree.clear();
            }
        }
        segs.clear();
        nt.listNeuron.clear();
        if(pdata) delete[] pdata;
        if(plist) delete[] plist;
        if(phi) delete[] phi;
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

