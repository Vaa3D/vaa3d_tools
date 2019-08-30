/* test2_plugin.cpp
 * This is a test plugin, you can use it as a demo.
 * 2019-4-26 : by YourName
 */
 
#include "v3d_message.h"
#include <iostream>
#include <vector>
#include "test2_plugin.h"
using namespace std;
Q_EXPORT_PLUGIN2(test2, TestPlugin);

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

bool TestPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("func1"))
	{
        cout<<"Welcome to image_threshold function"<<endl;
        if(input.size()!=2||output.size()!=1)
        {
            cout<<"illegal input!"<<endl;
            return -1;
        }

        // 1 - Read input image
        vector<char*>* inlist=(vector<char*>*)(input.at(0).p);
        if(inlist->size()!=1)
        {
            cout<<"You must specify 1 input file!"<<endl;
            return -1;
        }
        char* infile=inlist->at(0);
        cout<<"input file: "<<infile<<endl;
        unsigned char* inimg1d=NULL;
        V3DLONG* sz=NULL;
        int datatype=0;
        if(!simple_loadimage_wrapper(callback,infile,inimg1d,sz,datatype)) return -1;
    //    if(!loadImage(infile,inimg1d,sz,datatype))
    //        return -1;

        // 2 - Read color channel and threshold parameter
        //cout<<"weeeeeeeeeeeeeeeeeeeeeeeeeeeee"<<endl;
        v3d_msg("111");
        vector<char*>* paralist=(vector<char*>*)(input.at(1).p);
        if(paralist->size()!=2)
        {
            cout<<"Illegle parameter!"<<endl;
            return-1;
        }
        int c=atoi(paralist->at(0));
        int thres=atoi(paralist->at(1));
        cout<<"color channel: "<<c<<endl;
        cout<<"threshold: "<<thres<<endl;
        if(c<0||c>sz[3])
        {
            cout<<"The color channel does not exist!"<<endl;
            return -1;
        }
        //loadImage(infile,inimg1d,sz,datatype,c);
        simple_loadimage_wrapper(callback,infile,inimg1d,sz,datatype);

        // 3 - Read output fileName
        vector<char*>* outlist=(vector<char*>*)(output.at(0).p);
        if(outlist->size()!=1)
        {
            cout<<"You must specify 1 output file!"<<endl;
            return -1;
        }
        char* outfile=outlist->at(0);
        cout<<"output file: "<<outfile<<endl;

        // 4 - Do binary segmentation
        V3DLONG tb=sz[0]*sz[1]*sz[2]*datatype;
        unsigned char* nm=NULL;
        try{
            nm=new unsigned char[tb];
        }catch(...){
            throw("Fail to allocate memory in Image Thresholding plugin");
        }
        for(V3DLONG i=0;i<tb;++i)
        {
            if(inimg1d[i]>=thres)
                nm[i]=255;
            else
                nm[i]=0;
        }

        // 5 - Save file and free memory
        sz[3]=1;
        //saveImage(outfile,nm,sz,datatype)

        simple_saveimage_wrapper(callback,outfile,inimg1d,sz,datatype);
        if(nm)
        {
            delete[] nm;
            nm=NULL;
        }
        return 1;
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

