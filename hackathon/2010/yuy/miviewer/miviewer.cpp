/* miviewer.cpp
 * 2010-07-28: the program is created by Yang Yu
 */

#include "miviewer.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(miviewer, MIViewerPlugin);

//func
int iViewer(V3DPluginCallback &callback, QWidget *parent);

//plugin funcs
const QString title = "Microscopic Image Viewer";
QStringList MIViewerPlugin::menulist() const
{
    return QStringList() << tr("Microscopic Image Viewer")
						 << tr("about this plugin");
}

void MIViewerPlugin::domenu(const QString &menu_name, V3DPluginCallback &callback, QWidget *parent)
{
    if (menu_name == tr("Microscopic Image Viewer"))
    {
    	iViewer(callback, parent);
    }
	else if (menu_name == tr("about this plugin"))
	{
		QMessageBox::information(parent, "Version info", "MIViewer Plugin 1.0 (July 28, 2010) developed by Yang Yu. (Peng Lab, Janelia Research Farm Campus, HHMI)");
		return;
	}
}

//
int iViewer(V3DPluginCallback &callback, QWidget *parent)
{
	// load indexed data file
	QString m_FileName = QFileDialog::getOpenFileName(parent, QObject::tr("Open A Virtual Image"), "/Documents", QObject::tr("Image Configuration (*)"));
	
	if(m_FileName.isEmpty())	return false;
	
	// tiled images path
	QString curFilePath = QFileInfo(m_FileName).path();
	curFilePath.append("/");
	
	// compress image to make it fit in control window	
	// showing initial tri-view of control window
	float scaleFactor = 10;
	
	ImageNavigatingWidget *inw = new ImageNavigatingWidget(callback, m_FileName, curFilePath, scaleFactor);
	inw->show();
	
	long cur_status = inw->flag_changed;
	long pre_status = inw->flag_changed;
	
	while (1) 
	{
		cur_status = inw->flag_changed;
		
		if(cur_status != pre_status && inw->flag_syn)
		{
			pre_status = cur_status;
			
			inw->update_v3dviews(callback, inw->roi_start_x*scaleFactor, inw->roi_start_y*scaleFactor, inw->roi_start_z*scaleFactor, inw->roi_end_x*scaleFactor, inw->roi_end_y*scaleFactor, inw->roi_end_z*scaleFactor);
			
			qDebug()<<"ROI ..."<<inw->roi_start_x*scaleFactor<<inw->roi_start_y*scaleFactor<<inw->roi_start_z*scaleFactor<<inw->roi_end_x*scaleFactor<<inw->roi_end_y*scaleFactor<<inw->roi_end_z*scaleFactor;
			
			inw->flag_syn = false;
		}
		
		QCoreApplication::processEvents();
	}

	//
	return true;
}



