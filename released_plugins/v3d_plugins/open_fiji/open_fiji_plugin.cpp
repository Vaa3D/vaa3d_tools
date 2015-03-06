/* open_fiji_plugin.cpp
 * Uses Fiji to save v3draw file and open
 * 2013-12-12 : by Brian Long
 */
 

//  BRL making separate plugin based on iBioformatIO.cpp
//  but using a modified system call to use Fiji

//  added these includes from the original iBioformatIO.cpp
#include <QtGui>
#include <QFileInfo>
#include <QtGlobal>
#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"
#include "basic_4dimage.h"
#include <string.h>

#include <iostream>

#include "v3d_message.h"
#include <vector>
#include "open_fiji_plugin.h"
#include "../plugin_loader/v3d_plugin_loader.h"


using namespace std;

void call_open_using_imagej();


Q_EXPORT_PLUGIN2(open_fiji, open_fiji);
 
QStringList open_fiji::menulist() const
{
	return QStringList() 
        <<tr("Import using ImageJ and save as .v3draw")
        <<tr("About");
}

QStringList open_fiji::funclist() const
{
	return QStringList()
        <<tr("openfiji")
		<<tr("help");
}


void call_open_using_imagej(bool ismenu, QString inputfile, QString savefile,V3DPluginCallback2 &callback)
{
    QSettings setting("Vaa3D_tools", "open_imagej");
    QString imagej_binary_file = "non-existing"; //setting.value("imagej_binary_path").toByteArray();

    v3d_msg(QString("The default path of imagej is [%1]").arg(imagej_binary_file), 0);
   // v3d_msg(QString("The default temporary location for saving intermediate conversion files is [%1]").arg(tmp_conversion_folder), 0);

    QFile f_imagej_binary_file(imagej_binary_file);
    if (!f_imagej_binary_file.exists())
    {

        v3d_msg("Cannot find the specified default ImageJ location.");
#if defined(Q_OS_MAC)
         imagej_binary_file = getAppPath().append("/Fiji.app/Contents/MacOS/ImageJ-macosx");
#elif defined(Q_OS_LINUX)
        imagej_binary_file = getAppPath().append("/Fiji.app/ImageJ-linux64");
#elif defined(Q_OS_WIN32)
        imagej_binary_file = getAppPath().append("/Fiji.app/ImageJ-win32.exe");
#elif defined(Q_OS_WIN64)
        imagej_binary_file = getAppPath().append("/Fiji.app/ImageJ-win64.exe");
#else
        v3d_msg(tr("Currently only available for Linux, Mac OSX 10.5+ and Windows"));
        return;
#endif

        v3d_msg(QString("Now set the new imagej path to [%1]").arg(imagej_binary_file), 0);
        f_imagej_binary_file.setFileName(imagej_binary_file);
    }

    if (!f_imagej_binary_file.exists())
    {
        v3d_msg("Cannot locate the executable of ImageJ/Fiji program. Please specify Fiji location.");
//        v3d_msg("Cannot locate the executable of ImageJ/Fiji program. Now you can specify where it is.",0);
        // use this code above for dofunc version- prints to command line instead of window

        imagej_binary_file = QFileDialog::getOpenFileName(0, QObject::tr("Locate your ImageJ/Fiji application"),
                                                          QDir::currentPath(),
                                                          QObject::tr(" *"));

        if(imagej_binary_file.isEmpty())
        {
            return;
        }

        // the user will select the .app location, not the the command line executable location, which is OS-dependent

        //  need these for WIN32, WIN64, LINUX and MAC
        #if defined(Q_OS_MAC)
        // mac

                 QString fijiPath = imagej_binary_file.append("/Contents/MacOS/ImageJ-macosx");
        #elif defined(Q_OS_LINUX)
        // linux
                 QString fijiPath = imagej_binary_file.append("/ImageJ-linux64");
        #elif defined(Q_OS_WIN32)
               //32 bit windows
                 QString fijiPath = imagej_binary_file.append("/ImageJ-win32.exe");
        #elif defined(Q_OS_WIN64)
        // 64 bit windows
                 QString fijiPath = imagej_binary_file.append("/ImageJ-win64.exe");
        #else
                v3d_msg(tr("Currently only available for Linux, Mac OSX 10.5+ and Windows"));
                return;
        #endif

        f_imagej_binary_file.setFileName(imagej_binary_file);
    }

    // the user will select the .app location on mac and linux

    //now have found the ImageJ location. thus save it for future use

    setting.setValue("imagej_binary_path", qPrintable(imagej_binary_file));

    //now call ImageJ

    QString v3dAppPath = getAppPath();

    QString cmd_Fiji = QString("%1  --headless -batch  %2/brl_FijiConvert.js %3:%4").arg(imagej_binary_file.toStdString().c_str()).arg(v3dAppPath.toStdString().c_str()).arg(inputfile.toStdString().c_str()).arg(savefile.toStdString().c_str());
    v3d_msg(cmd_Fiji, 0);

    //the folowing direct call work nicely: //20141004, PHC
    //  /Users/pengh/work/v3d_external/bin/Fiji.app/Contents/MacOS/ImageJ-macosx  --headless -batch  /Users/pengh/work/v3d_external/bin/brl_FijiConvert.js /workdata/v3d_demo_data/single_neuron/Twin-Spot_GH146_lAL_39-2.lsm:/Users/pengh/Downloads/44/11.avi
    //
    //Thus it seems the problem is "Unrecognized command: "raw writer""

    system(qPrintable(cmd_Fiji));

    if (!QFile(savefile).exists()) v3d_msg("File conversion failed.\n"); //need change later

    if (ismenu) { // display image if it's being called from the menu. otherwise don't!
        // load
        V3DLONG sz_relative[4];
        int datatype_relative = 0;
        unsigned char* relative1d = 0;

        if (simple_loadimage_wrapper(callback, const_cast<char *>(savefile.toStdString().c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
             fprintf (stderr, "Error happens while reading the subject file [%s]. Exit. \n",savefile.toStdString().c_str());
             //return;
        }


        // visualize
        Image4DSimple p4DImage;

        if(datatype_relative == V3D_UINT8)
        {
             p4DImage.setData((unsigned char*)relative1d, sz_relative[0], sz_relative[1], sz_relative[2], sz_relative[3], V3D_UINT8);
        }
        else if(datatype_relative == V3D_UINT16)
        {
             p4DImage.setData((unsigned char*)relative1d, sz_relative[0], sz_relative[1], sz_relative[2], sz_relative[3], V3D_UINT16);
        }
        else if(datatype_relative == V3D_FLOAT32)
        {
             p4DImage.setData((unsigned char*)relative1d, sz_relative[0], sz_relative[1], sz_relative[2], sz_relative[3], V3D_FLOAT32);
        }
        else
        {
             printf("\nError: The program only supports UINT8, UINT16, and FLOAT32 datatype.\n");
             return;
        }

        v3dhandle newwin = callback.newImageWindow();
        callback.setImage(newwin, &p4DImage);
        callback.setImageName(newwin, savefile.toStdString().c_str());
        callback.updateImageWindow(newwin);
    }
}


void open_fiji::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    printf(menu_name.toStdString().c_str());
    if (menu_name == tr("Import using ImageJ and save as .v3draw"))
    {


        // input image file
        QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Choose file to import"),
                                                          QDir::currentPath(),
                                                          QObject::tr("Image File (*.*)"));

        if(m_FileName.isEmpty())
        {
             printf("\nError: Your image does not exist!\n");
             return;
        }
        // target directory for v3draw file
        QFileDialog d;
         d.setWindowTitle(tr("Select Save Directory:"));
         d.setFileMode(QFileDialog::Directory);
         d.exec();

              QString m_SaveDir=(d.selectedFiles())[0]; // on some machines, selecting a save directory seems to double up a single directory.
            //print some output to the commandline for informational purposes
          printf("filename [%s]\n",m_FileName.toStdString().c_str());
          printf("save target [%s]\n",m_SaveDir.toStdString().c_str());

        // temp directory
        QString baseName = QFileInfo(m_FileName).baseName();
        QString savefile = m_SaveDir.append("/").append(baseName).append(".raw");

        call_open_using_imagej(true, m_FileName, savefile, callback);
    }
	else
	{
        v3d_msg(tr("Uses Fiji to save .v3draw file and open in vaa3d. "
			"Developed by Brian Long, 2013-12-12"));
	}
}

bool open_fiji::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("openfiji"))
    {
    vector<char*> infiles, outfiles;
    if(input.size()==0) {
        cout<<"for usage type: vaa3d -x open_fiji -f help"<<endl;
        return false;
    }
    if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
    if(input.size() >= 2)
    {
    if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);
    }
    char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
    char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);


        call_open_using_imagej(false, QString(inimg_file    ),QString(outimg_file), callback);

    }
	else if (func_name == tr("help"))
	{
        cout<<"Usage : vaa3d -x open_fiji -f openfiji -i <input_file_name> -o <output_file_name>  "<<endl;
    }
	else return false;

	return true;
}

QString getAppPath()
{
    QString v3dAppPath("~/Work/v3d_external/v3d");
    QDir testPluginsDir = QDir(qApp->applicationDirPath());

#if defined(Q_OS_WIN)
    if (testPluginsDir.dirName().toLower() == "debug" || testPluginsDir.dirName().toLower() == "release")
        testPluginsDir.cdUp();
#elif defined(Q_OS_MAC)
    // In a Mac app bundle, plugins directory could be either
    //  a - below the actual executable i.e. v3d.app/Contents/MacOS/plugins/
    //  b - parallel to v3d.app i.e. foo/v3d.app and foo/plugins/
    if (testPluginsDir.dirName() == "MacOS") {
        QDir testUpperPluginsDir = testPluginsDir;
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp();
        testUpperPluginsDir.cdUp(); // like foo/plugins next to foo/v3d.app
        if (testUpperPluginsDir.cd("plugins")) testPluginsDir = testUpperPluginsDir;
        testPluginsDir.cdUp();
    }
#endif

    v3dAppPath = testPluginsDir.absolutePath();
    return v3dAppPath;
}
