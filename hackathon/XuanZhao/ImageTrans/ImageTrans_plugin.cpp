/* ImageTrans_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-7-5 : by YourName
 */
 
#include "v3d_message.h"
#include <vector>
#include "ImageTrans_plugin.h"
#include "trans_class.h"
#include <basic_surf_objs.h>
using namespace std;
Q_EXPORT_PLUGIN2(ImageTrans, TestPlugin);
 
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
			"Developed by YourName, 2019-7-5"));
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
        const char* file=(infiles.size()>=1)?infiles[0]:"";
        unsigned char* pdata=0;
        V3DLONG sz[4]={0,0,0,0};
        int datatype=0;
        simple_loadimage_wrapper(callback,file,pdata,sz,datatype);
        cout<<"put image..."<<endl;
        ImageCtrl c(pdata,sz);

        vector<vector<vector<unsigned char>>> image;
        c.Data1d_to_3d(image);
        //c.display(image);

        vector<vector<vector<unsigned char>>> image_new;
        cout<<"prepare draw..."<<endl;
        int times=(inparas.size()>=1)?atoi(inparas[0]):5;
        int mode=(inparas.size()>=2)?atoi(inparas[1]):0;
        c.Draw_S(image_new,times,mode);
        cout<<"draw end..."<<endl;
        c.Data3d_to_1d(image_new);
        cout<<"start save..."<<endl;
        QString outfile(outfiles[0]);
        c.SaveImage(outfile,callback);
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

