/* test3_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-4-26 : by YourName
 */
 
#include "v3d_message.h"
#include <iostream>
#include <vector>
#include "test3_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(test3, TestPlugin);
 
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
        unsigned char * aa = 0;
        V3DLONG dr[4]={0,0,0,0};
        int datatype=0;
        const char * filename="C:\\Users\\BrainCenter2\\Desktop\\tt2.tif";
        cout<<"777"<<endl;
        if(!simple(callback,filename,aa,dr,datatype))
        {
            cout<<"123"<<endl;
        };
        for (int i=0;i<4;i++)
        {
            cout<<dr[i]<<endl;
        }
        for (int i=0;i<dr[0]*dr[1]*dr[2]*dr[3];i++)
        {
            cout<<static_cast<int>(aa[i])<<" ";
        }
        cout<<datatype<<endl;
	}
	else if (menu_name == tr("menu2"))
	{
		v3d_msg("To be implemented.");
	}
	else
	{
		v3d_msg(tr("This is a test plugin, you can use it as a demo.. "
			"Developed by YourName, 2019-4-26"));
	}
}
bool simple(V3DPluginCallback & cb, const char * filename, unsigned char * & pdata, V3DLONG sz[4], int & datatype)
{
    if (!filename || !sz)
    {
        cout<<"xxx"<<endl;
        return false;
    }


    Image4DSimple *inimg = 0;
    inimg = cb.loadImage((char *)filename);
    if (!inimg || !inimg->valid())
        return false;

    if (pdata) {delete []pdata; pdata=0;}

    V3DLONG totalbytes = inimg->getTotalBytes();
    try
    {
        pdata = new unsigned char [totalbytes];
        if (!pdata)
            goto Label_error_simple_loadimage_wrapper;

        memcpy(pdata, inimg->getRawData(), totalbytes);
        datatype = inimg->getUnitBytes(); //1,2,or 4
        sz[0] = inimg->getXDim();
        sz[1] = inimg->getYDim();
        sz[2] = inimg->getZDim();
        sz[3] = inimg->getCDim();
        if (inimg) {delete inimg; inimg=0;} //delete "outimg" pointer, added by Z.ZHOU 06122014

    }
    catch (...)
    {
        goto Label_error_simple_loadimage_wrapper;
    }

    return true;

Label_error_simple_loadimage_wrapper:
    if (inimg) {delete inimg; inimg=0;}
    return false;
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

