/* iBioformatIO.cpp
 * 2011-12-02: create this program by Yang Yu
 */

// add dofunc() by Jianlong Zhou, 2012-04-08

//

#ifndef __IBIOFORMATIO_CPP__
#define __IBIOFORMATIO_CPP__

#include <QtGui>
#include <QFileInfo>

//#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "iBioformatIO.h"

//#include "basic_surf_objs.h"
//#include "stackutil.h"
//#include "volimg_proc.h"
//#include "img_definition.h"
//#include "basic_landmark.h"

//#include "mg_utilities.h"
//#include "mg_image_lib.h"

//#include "basic_landmark.h"
//#include "basic_4dimage.h"

#include <iostream>
using namespace std;

#include <string.h>


//plugin interface
const QString title = "Image IO Using Bioformat";
static QString pluginRootPath;

Q_EXPORT_PLUGIN2(imageIO_Bioformat, IBioformatIOPlugin);

QStringList IBioformatIOPlugin::menulist() const
{
    return QStringList() << tr("load an image using Bioformats Java library")
                         << tr("click me if you are unhappy with the loading result...")
                         << tr("About");
}

void IBioformatIOPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("load an image using Bioformats Java library"))
    {

        // input
        QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open An Image"),
                                                          QDir::currentPath(),
                                                          QObject::tr("Image File (*.*)"));

        if(m_FileName.isEmpty())
        {
             printf("\nError: Your image does not exist!\n");
             return;
        }

        // temp
        QString baseName = QFileInfo(m_FileName).baseName();
        QString tmpfile = QDir::tempPath().append("/").append(baseName).append(".tif");

        //
        QFile tmpqfile(tmpfile);
        if (tmpqfile.exists()) system(qPrintable(QString("rm -f \"%1\"").arg(tmpfile)));

        //look for loci_tools.jar
        QString lociDir = ("loci_tools.jar");
        if (!QFile(lociDir).exists())
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

        QString cmd_loci = QString("java -cp %1 loci.formats.tools.ImageConverter \"%2\" \"%3\"").arg(lociDir.toStdString().c_str()).arg(m_FileName.toStdString().c_str()).arg(tmpfile.toStdString().c_str());
        v3d_msg(cmd_loci, 0);

        system(qPrintable(cmd_loci));

        if (!tmpqfile.exists())
        {
            v3d_msg("The temprary file does not exist. The conversion of format using Bioformats has failed. Please sue another way to convert and load using Vaa3D.\n");
            return;
        }


        // load
        V3DLONG sz_relative[4];
        int datatype_relative = 0;
        unsigned char* relative1d = 0;

        if (simple_loadimage_wrapper(callback, const_cast<char *>(tmpfile.toStdString().c_str()), relative1d, sz_relative, datatype_relative)!=true)
        {
             fprintf (stderr, "Error happens in reading the subject file [%s]. Exit. \n",tmpfile.toStdString().c_str());
             return;
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
    else if (menu_name == tr("click me if you are unhappy with the loading result..."))
    {
       v3d_msg("This program is designed to use a system call to invoke the LOCI Bioformats Image IO Java library loci_tools.jar"
               " to load an image. It first calls bioformats library to generate a temporary 3D TIF file on your harddrive"
               " and then uses Vaa3D to load that temporary file. Therefore, if you see some wrong loading result using this plugin, it is"
               " likely that you will get the same thing if you run the bioformats library directly. Of course, you may find a newer"
               " version of the loci_tools.jar at the LOCI website http://loci.wisc.edu/bio-formats/downloads; we encourage you"
               " to copy the latest version to the Vaa3D executable folder and try if it would fix your problem.", 1);
       return;
    }
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Simple image reading using Bioformats library %1 (2011-2012) developed by Yang Yu, Yinan Wan, and Hanchuan Peng. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()).append("\n"));
        return;
    }
}

// plugin func
QStringList IBioformatIOPlugin::funclist() const
{
    return QStringList() << tr("imgiobiof")
                         << tr("help");
}

bool IBioformatIOPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
     if (func_name == tr("imgiobiof"))
	{
          cout<<"Welcome to imageIOBioformat loader"<<endl;
          if(input.size() != 1 || output.size() != 1) return false;

          char * inimg_file = ((vector<char*> *)(input.at(0).p))->at(0);
          char * outimg_file = ((vector<char*> *)(output.at(0).p))->at(0);
          cout<<"inimg_file = "<<inimg_file<<endl;
          cout<<"outimg_file = "<<outimg_file<<endl;

          QString m_FileName(inimg_file); // use inimge_file name

          if(m_FileName.isEmpty())
          {
               printf("\nError: Your image does not exist!\n");
               return false;
          }

          // temp
          QString baseName = QFileInfo(m_FileName).baseName();
          //QString tmpfile = QDir::tempPath().append("/").append(baseName).append(".tif");
          QString tmpfile(outimg_file); //use outimg_file name

          //
          QFile tmpqfile(tmpfile);
          if (tmpqfile.exists()) system(qPrintable(QString("rm -f \"%1\"").arg(tmpfile)));

          //look for loci_tools.jar
          QString lociDir = ("loci_tools.jar");
          if (!QFile(lociDir).exists())
          {
               printf("loci_tools.jar is not in current directory, search v3d app path.\n");
               lociDir = getAppPath().append("/loci_tools.jar");
               printf(qPrintable(lociDir));
               printf("\n");
               if (!QFile(lociDir).exists())
               {
                    v3d_msg("Cannot find loci_tools.jar, please download it and make sure it is put under the Vaa3D executable folder, parallel to the Vaa3D executable and the plugins folder.", 0);
                    return false;
               }
          }

          QString cmd_loci = QString("java -cp %1 loci.formats.tools.ImageConverter \"%2\" \"%3\"").arg(lociDir.toStdString().c_str()).arg(m_FileName.toStdString().c_str()).arg(tmpfile.toStdString().c_str());
          v3d_msg(cmd_loci, 0);

          system(qPrintable(cmd_loci));

          if (!tmpqfile.exists())
          {
              v3d_msg("The temprary file does not exist. The conversion of format using Bioformats has failed. Please use another way to convert and load using Vaa3D.\n");
              return false;
          }

          return true;
	}
	else if(func_name == tr("help"))
	{
		cout<<"Usage : v3d -x imageIO_Bioformat -f imgiobiof -i <inimg_file> -o <outimg_file>"<<endl;
		cout<<"e.g. v3d -x imageIO_Bioformat -f imgiobiof -i input.raw -o output.raw"<<endl;
		cout<<endl;
		return true;
	}

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

#endif


