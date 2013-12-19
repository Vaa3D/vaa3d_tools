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



Q_EXPORT_PLUGIN2(open_fiji, open_fiji);
 
QStringList open_fiji::menulist() const
{
	return QStringList() 
        <<tr("Import using ImageJ and save as .v3draw")
        <<tr("Convert Z sections in a .v3draw file to .avi")
        <<tr("About");
}

QStringList open_fiji::funclist() const
{
	return QStringList()
        <<tr("openfiji")
		<<tr("help");
}

void open_fiji::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    printf(menu_name.toStdString().c_str());
    if (menu_name == tr("Import using ImageJ and save as .v3draw"))
    {

        // input image file
        QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open An Image"),
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

              QString m_SaveDir=(d.selectedFiles())[0];
// on some machines, selecting a save directory seems to double up a single directory.

             // I wont to parse this string for di

          printf("filename [%s]\n",m_FileName.toStdString().c_str());
          printf("save target [%s]\n",m_SaveDir.toStdString().c_str());

        // temp
        QString baseName = QFileInfo(m_FileName).baseName();
        QString tmpfile = m_SaveDir.append("/").append(baseName).append(".v3draw");


        QFile tmpqfile(tmpfile);
  //      if (tmpqfile.exists()) system(qPrintable(QString("rm -f \"%1\"").arg(tmpfile)));


  //      QString lociDir = getAppPath().append("/mac_ffmpeg");

       //look for loci_tools.jar
  //      QString lociDir = ("loci_tools.jar");
 /*       if (!QFile(lociDir).exists())
        {
             printf("loci_tools.jar is not in current directory, search v3d app path.\n");
             lociDir = getAppPath().append("loci_tools.jar");
             printf(qPrintable(lociDir));
             printf("\n");
             if (!QFile(lociDir).exists())
             {
                  v3d_msg("Cannot find loci_tools.jar, please download it and make sure it is put under the Vaa3D executable folder, parallel to the Vaa3D executable and the plugins folder.");
                  return;
             }
        }
*/
        QDir AppDir = QDir(qApp->applicationDirPath());

        // I need to construct substrings for the various ImageJ executables...

// Actually I need these for WIN32, WIN64, LINUX and MAC
#if defined(Q_OS_MAC)
// mac
        AppDir.cdUp();
        AppDir.cdUp();
        AppDir.cdUp();
         QString fijiPath = "/Fiji.app/Contents/MacOS/ImageJ-macosx";
#elif defined(Q_OS_LINUX)
// linux
         QString fijiPath = "/Fiji.app/ImageJ-linux64";
#elif defined(Q_OS_WIN32)
       //32 bit windows
         QString fijiPath = "/Fiji.app/ImageJ-win32.exe";
#elif defined(Q_OS_WIN64)
// 64 bit windows
         QString fijiPath = "/Fiji.app/ImageJ-win64.exe";
#else
        v3d_msg(tr("Currently only available for Linux, Mac OSX 10.5+ and Windows"));
        return;
#endif

        QString appdirstring = AppDir.absolutePath();  // on a mac, you need to go 3 dir up from the app path to get to v3d_external/bin



   //     QString cmd_loci = QString("java -cp %1 loci.formats.tools.ImageConverter \"%2\" \"%3\"").arg(lociDir.toStdString().c_str()).arg(m_FileName.toStdString().c_str()).arg(tmpfile.toStdString().c_str());
        QString cmd_Fiji = QString("%1%2  --headless -batch  %1/brl_FijiConvert.js %3:%4").arg(appdirstring.toStdString().c_str()).arg(fijiPath.toStdString().c_str()).arg(m_FileName.toStdString().c_str()).arg(tmpfile.toStdString().c_str());
        system(qPrintable(cmd_Fiji));

        if (!tmpqfile.exists()) v3d_msg("The format is not supported, or something is wrong in your file\n");

        // load
        V3DLONG sz_relative[4];
        int datatype_relative = 0;
        unsigned char* relative1d = 0;

        if (simple_loadimage_wrapper(callback, const_cast<char *>(tmpfile.toStdString().c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
             fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",tmpfile.toStdString().c_str());
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
        callback.setImageName(newwin, tmpfile.toStdString().c_str());
        callback.updateImageWindow(newwin);

    }
    else if (menu_name == tr("Convert Z sections in a .v3draw file to .avi"))
	{

        // input image file
        QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Select .v3draw file"),
                                                          QDir::currentPath(),
                                                          QObject::tr("Image File (*.v3draw*)"));


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

              QString m_SaveDir=(d.selectedFiles())[0];


          printf("filename [%s]\n",m_FileName.toStdString().c_str());
          printf("save target [%s]\n",m_SaveDir.toStdString().c_str());

        // temp
        QString baseName = QFileInfo(m_FileName).baseName();
        QString tmpfile = m_SaveDir.append("/").append(baseName).append(".avi");


        QFile tmpqfile(tmpfile);


        QDir AppDir = QDir(qApp->applicationDirPath());
        AppDir.cdUp();
        AppDir.cdUp();
        AppDir.cdUp();
        QString appdirstring = AppDir.absolutePath();  // need to go 3 dir up from the app path to get to v3d_external/bin

        QString cmd_Fiji = QString("%1/Fiji.app/Contents/MacOS/ImageJ-macosx  --headless -batch  %1/brl_FijiConvert.js %2:%3").arg(appdirstring.toStdString().c_str()).arg(m_FileName.toStdString().c_str()).arg(tmpfile.toStdString().c_str());
        system(qPrintable(cmd_Fiji));	}
	else
	{
		v3d_msg(tr("Uses Fiji to save v3draw file and open. "
			"Developed by Brian Long, 2013-12-12"));
	}
}

bool open_fiji::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

    if (func_name == tr("openfiji"))
	{
        cout<<"to be implemented... "<<endl;
    }
    else if (func_name == tr("v3draw2avi"))
	{
        cout<<"to be implemented..."<<endl;
    }
	else if (func_name == tr("help"))
	{
        cout<<"Usage: "<<endl;
        cout<<"Usage : v3d -x open_fiji -f v3draw2avi -i <inimg_file> -o <outimg_file> -p "<<endl;
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
