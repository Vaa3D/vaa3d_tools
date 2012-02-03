/* iBioformatIO.cpp
 * 2011-12-02: create this program by Yang Yu
 */


// 

#ifndef __IBIOFORMATIO_CPP__
#define __IBIOFORMATIO_CPP__

#include <QtGui>
#include <QFileInfo>

#include <cmath>
#include <stdlib.h>
#include <ctime>

#include "iBioformatIO.h"

#include "basic_surf_objs.h"
#include "stackutil.h"
#include "volimg_proc.h"
#include "img_definition.h"
#include "basic_landmark.h"

#include "mg_utilities.h"
#include "mg_image_lib.h"

#include "basic_landmark.h"
#include "basic_4dimage.h"

#include <iostream>
using namespace std;

#include <string.h>


//plugin interface
const QString title = "Image IO Using Bioformat";
static QString pluginRootPath;

Q_EXPORT_PLUGIN2(imageIO_Bioformat, IBioformatIOPlugin);

QStringList IBioformatIOPlugin::menulist() const
{
    return QStringList() << tr("ImageIO_Bioformat")
                         << tr("About");
}

void IBioformatIOPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("ImageIO_Bioformat"))
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
	if (tmpqfile.exists()) system(qPrintable(QString("rm -f ")+tmpfile));
	
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
			v3d_msg("cannot find loci_tools.jar, please download it.");
			return;
		}
	}
	
        QString cmd_loci = QString("java -cp %1 loci.formats.tools.ImageConverter %2 %3").arg(lociDir.toStdString().c_str()).arg(m_FileName.toStdString().c_str()).arg(tmpfile.toStdString().c_str());
        
        system(qPrintable(cmd_loci));

	if (!tmpqfile.exists()) v3d_msg("The format is not supported, or something is wrong in your file\n");

        
        // load
        V3DLONG *sz_relative = 0;
        int datatype_relative = 0;
        unsigned char* relative1d = 0;
        
        if (loadImage(const_cast<char *>(tmpfile.toStdString().c_str()), relative1d, sz_relative, datatype_relative)!=true)
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
    else if (menu_name == tr("About"))
    {
        QMessageBox::information(parent, "Version info", QString("Image IO using Bioformat Plugin %1 (Dec. 2, 2011) developed by Yang Yu and Hanchuan Peng. (Janelia Research Farm Campus, HHMI)").arg(getPluginVersion()).append("\n"));
        return;
    }
}

// plugin func
QStringList IBioformatIOPlugin::funclist() const
{
    return QStringList() << tr("imageIOBioformat");
}

bool IBioformatIOPlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
    //
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
	}
#endif
	v3dAppPath=testPluginsDir.absolutePath();

	return v3dAppPath;
}

#endif


