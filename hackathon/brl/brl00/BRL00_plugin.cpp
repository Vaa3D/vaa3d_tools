/* BRL00_plugin.cpp
 * BRL test plugin 00
 * 2013-12-20 : by BRL
 */
 
#include "v3d_message.h"
#include <vector>
#include "BRL00_plugin.h"
#include <QtGui>
#include <fstream>
#include <iostream>
#include <math.h>
#define WANT_STREAM       // include iostream and iomanipulators



using namespace std;
Q_EXPORT_PLUGIN2(BRL00, brl00);
 

// function declaration here. definition below
void processImage(V3DPluginCallback2 &callback);



QStringList brl00::menulist() const
{
	return QStringList() 
        <<tr("Select ROI")
        <<tr("Modify voxels")
		<<tr("about");
}

QStringList brl00::funclist() const
{
	return QStringList()
        <<tr("selectroi")
        <<tr("modifyvoxels")
		<<tr("help");
}

void brl00::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Select ROI"))
    {
        // check what's up with the current window

        v3dhandle curwin = callback.currentImageWindow();
        if (!curwin)
        {
            QMessageBox::information(0, "", "You don't have an image open in the main window.");
            return;
        }

        Image4DSimple* p4DImage = callback.getImage(curwin);

        if (!p4DImage)
        {
            QMessageBox::information(0, "", "The image pointer is invalid.");
            return;
        }

        // first need to select an ROI... this should be an existing command with the callback as an argument
// apparently there's a lot of overhead for this...

        // declearing stuff...
        V3DPluginArgItem arg;
        V3DPluginArgList input;
        V3DPluginArgList output;
        unsigned char * data1d_loaded = 0; // this is a pointer to the data loaded from a file
        int datatype;  // these are defined by load_image_wrapper?
        V3DLONG * in_zz = 0;
        unsigned char*  data1d_image = p4DImage->getRawData(); //this is for the data from the current window

        //and this thing that defines what channel we're looking at
        char c=1;


        arg.type = "random";std::vector<char*> args1;
        args1.push_back("/Users/brl/dump/ex_.v3draw"); arg.p = (void *) & args1; input<< arg;
        arg.type = "random";std::vector<char*> args;
        char channel = '0' + c;
        args.push_back("3");args.push_back("3");args.push_back("3");args.push_back(&channel); args.push_back("1.0"); arg.p = (void *) & args; input << arg;
        arg.type = "random";std::vector<char*> args2;args2.push_back("/Users/brl/dump/gfImage.v3draw"); arg.p = (void *) & args2; output<< arg;

        QString full_plugin_name = "gaussian";
        QString func_name = "gf";

        callback.callPluginFunc(full_plugin_name,func_name, input,output);
        char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
        v3d_msg(outimg_file);
        simple_loadimage_wrapper(callback, outimg_file, data1d_loaded, in_zz, datatype);
       // remove("temp.v3draw");
       // remove("gfImage.v3draw");
    }
    else if (menu_name == tr("Modify voxels"))
    {
        //  this will be my processImage function which will be defined below
        processImage(callback);

    }
else
    {
        v3d_msg(tr("BRL test plugin 00 "
                   "Developed by BRL 2013.12.20"));
    }
}
bool brl00::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
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

// processImage function cribbed from ZZ's local enhancement plugin

void processImage(V3DPluginCallback2 &callback)
{
    v3dhandle curwin = callback.currentImageWindow();
    if (!curwin)
    {
        QMessageBox::information(0, "", "You don't have any image open in the main window.");
        return;
    }

    Image4DSimple* p4DImage = callback.getImage(curwin);

    if (!p4DImage)
    {
        QMessageBox::information(0, "", "The image pointer is invalid. Ensure your data is valid and try again!");
        return;
    }

    unsigned char* data1d = p4DImage->getRawData();

    V3DLONG N = p4DImage->getXDim();
    V3DLONG M = p4DImage->getYDim();
    V3DLONG P = p4DImage->getZDim();
    ImagePixelType pixeltype = p4DImage->getDatatype();


    // display new  this was copied from plug_watershed and is substantially different
    // from ZZs local enhancement code.
    Image4DSimple new4DImage;
    new4DImage.setData((unsigned char *)data1d,N, M, P, 1, pixeltype);
    v3dhandle newwin = callback.newImageWindow();
    callback.setImage(newwin, &new4DImage);
    callback.setImageName(newwin, QString("Local_adaptive_enhancement_result"));
    callback.updateImageWindow(newwin);
}

