/* bioformats_loader_plugin.cpp
 * Uses bioformats to save v3draw file and open
 * 2013-12-12 : by Brian Long
 */


// 2015.03.11  BRL modified this to use the bioformats tool bfconvert instead of dragging around Fiji everywhere.
// CRITICAL NOTE FROM BRL:  the bioformats tools package included in the src folder includes my v3draw writer- other versions may not.



//  based on iBioformatIO.cpp



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
#include "bioformats_loader_plugin.h"
#include "../plugin_loader/v3d_plugin_loader.h"


using namespace std;

void call_open_using_imagej();


Q_EXPORT_PLUGIN2(bioformats_loader, bioformats_loader);
 
QStringList bioformats_loader::menulist() const
{
	return QStringList() 
        <<tr("Convert to .v3draw with Bioformats and import")
        <<tr("About");
}

QStringList bioformats_loader::funclist() const
{
	return QStringList()
        <<tr("bioformatsLoader")
		<<tr("help");
}


void call_open_using_imagej(bool ismenu, QString inputfile, QString savefile,V3DPluginCallback2 &callback)
{
    QSettings setting("Vaa3D_tools", "open_imagej");
    QString bfconvert = "undefined";
   // v3d_msg(QString("The default temporary location for saving intermediate conversion files is [%1]").arg(tmp_conversion_folder), 0);
    QFile f_bioformats(bfconvert);
    if (!f_bioformats.exists())
    {
#if defined(Q_OS_MAC)
         bfconvert = getAppPath().append("/bioformats_tools/bfconvert");
#elif defined(Q_OS_LINUX)
        bfconvert = getAppPath().append("/bioformats_tools/bfconvert");
#elif defined(Q_OS_WIN32)
        bfconvert = getAppPath().append("\bioformats_tools\bfconvert");
#elif defined(Q_OS_WIN64)
        bfconvert = getAppPath().append("\bioformats_tools\bfconvert");
#else
        v3d_msg(tr("Currently only available for Linux, Mac OSX 10.5+ and Windows"));
        return;
#endif

        f_bioformats.setFileName(bfconvert);
    }

    if (!f_bioformats.exists())
    {
        v3d_msg("Cannot locate the Bioformats 'bfconvert' program. Please specify its location.");
        // use this code above for dofunc version- prints to command line instead of window

        bfconvert = QFileDialog::getOpenFileName(0, QObject::tr("Locate Bioformats bfconvert tool"),
                                                          QDir::currentPath(),
                                                          QObject::tr(" *"));

        if(bfconvert.isEmpty())
        {
            return;
        }


        f_bioformats.setFileName(bfconvert);
    }

    // the user will select the .app location on mac and linux

    //now have found the bioformats location. thus save it for future use

    setting.setValue("bfconvert", qPrintable(bfconvert));

    QString v3dAppPath = getAppPath();

    QString cmd_bfconvert = QString("%1 %2 %3").arg(bfconvert.toStdString().c_str()).arg(inputfile.toStdString().c_str()).arg(savefile.toStdString().c_str());

    v3d_msg(cmd_bfconvert, 0);
    system(qPrintable(cmd_bfconvert));

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


void bioformats_loader::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    printf(menu_name.toStdString().c_str());
    if (menu_name == tr("Convert to .v3draw with Bioformats and import"))
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
        QString savefile = m_SaveDir.append("/").append(baseName).append(".v3draw");

        call_open_using_imagej(true, m_FileName, savefile, callback);
    }
	else
	{
        v3d_msg(tr("Uses Bioformats bfconvert to save .v3draw file and open in vaa3d. "
            "Brian Long and Vaa3D team"));
	}
}

bool bioformats_loader::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    if (func_name == tr("bioformatsLoader"))
    {
    vector<char*> infiles, outfiles;
    if(input.size()==0) {
        cout<<"for usage type: vaa3d -x bioformats_loader -f help"<<endl;
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
        cout<<"Usage : vaa3d -x bioformats_loader -f bioformatsLoader -i <input_file_name> -o <output_file_name>  "<<endl;
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
