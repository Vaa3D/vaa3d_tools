/* convert_img_to_atlasview.cpp
 * extract the channels of an image and convert it as an atlas-viewer vieweable format
 * 2011-02-28: by Hanchuan Peng
 */

#include "convert_img_to_atlasview.h"
#include "v3d_message.h"

#include "stackutil.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(convert_img_to_atlasview, ConvertImg2AtlasPlugin);

void convertImg2Atlas(V3DPluginCallback2 &callback, QWidget *parent);

//plugin funcs
const QString title = "Distance transform";
QStringList ConvertImg2AtlasPlugin::menulist() const
{
    return QStringList() 
		<< tr("Convert the current active (selected) image")
		<< tr("About");
}

void ConvertImg2AtlasPlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
    if (menu_name == tr("Convert the current active (selected) image"))
    {
    	convertImg2Atlas(callback, parent); 
    }
	else if (menu_name == tr("About"))
	{
		v3d_msg(QString("This plugin extracts an image's channels and save each of them as a separate image file, "
						"and then generates a linker file (.atlas) to link all of them, so that one can use V3D's "
						"AtlasViewer to blend/see the data in a more flexible way. Developed by Hanchuan Peng. Version (%1)").arg(getPluginVersion()));
	}
}

void convertImg2Atlas(V3DPluginCallback2 &callback, QWidget *parent)
{
	v3dhandle curwin = callback.currentImageWindow();
	if (!curwin)
	{
		v3d_msg("You don't have any image open in the main window.");
		return;
	}
	
	Image4DSimple* subject = callback.getImage(curwin);
	QString m_InputFileName = callback.getImageName(curwin);
	
	if (!subject)
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	
	Image4DProxy<Image4DSimple> pSub(subject);
	
	V3DLONG sz[4];
	sz[0] = subject->getXDim();
    sz[1] = subject->getYDim();
    sz[2] = subject->getZDim();
	sz[3] = 1;

	//now write files
	
	QString folderName = QFileDialog::getSaveFileName(0, QString("Select a folder to save the v3d-atlas files"),
													QString(subject->getFileName())+"_atlas",
													QString("Atlas files (*.atlas)"),
													0,
													QFileDialog::ShowDirsOnly);
	
	QDir d(folderName);
	if (!d.exists(folderName))
		d.mkdir(folderName);
	

	unsigned char simple_cmap[14*3]  = {0,   0,   0,
		255, 0,   0,
		0,   255, 0,
		0,   0,   255,
		255, 255, 0,
		0,   255, 255,
		255, 0,   255,
		255, 128,   0,
		128, 255, 0,
		0,   128, 255,
		255, 255, 128,
		128, 255, 255,
		255, 128, 255,
		255, 255, 255
	};
	
	QString fileName = folderName + "/c.atlas";
	FILE *fp = fopen(qPrintable(fileName), "wt");
	if (!fp)
		return;
	
	for (V3DLONG i=0;i<subject->getCDim();i++)
	{
		QString tag = QString("ch") + QString("").setNum(i+1); 
		QString curfile = folderName + "/" + tag + ".raw";
		
		int j = i%13+1;
		fprintf(fp, "%d, %s, %d, %d, %d, %s\n", i, qPrintable(tag), simple_cmap[j*3+0], simple_cmap[j*3+1], simple_cmap[j*3+2],  qPrintable(tag+".raw"));
				
		saveImage(qPrintable(curfile), subject->getRawDataAtChannel(i), sz, subject->getDatatype());
	}

	if (fp) fclose(fp);

	//display a message
	v3d_msg(QString("The data have been converted and saved to the atlas file [%1]. You can drag and drop it into V3D to use AtlasViewer to blend different channels.").arg(fileName));
}
