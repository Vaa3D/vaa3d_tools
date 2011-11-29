/*
 *  add_roi.cpp
 *  add_roi
 *
 *  Created by Yang, Jinzhu on 11/24/10.
 *
 */

#include "roi_editor.h"
#include "v3d_message.h"

//Q_EXPORT_PLUGIN2 ( PluginName, ClassName )
//The value of PluginName should correspond to the TARGET specified in the plugin's project file.
Q_EXPORT_PLUGIN2(roi_editor, ROI_Editor_Plugin);

void do_computation(V3DPluginCallback2 &callback, QWidget *parent, int method_code);

//plugin funcs
const QString title = "ROI editor";
QStringList ROI_Editor_Plugin::menulist() const
{
    return QStringList() 
	<< tr("copy ROIs from another image")
	<< tr("paste ROIs to another image")
	<< tr("delete ROIs in all tri-view planes")
	<< tr("delete xy-plane ROI")
	<< tr("delete yz-plane ROI")
	<< tr("delete zx-plane ROI")
	<< tr("Help");
}

void ROI_Editor_Plugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("paste ROIs to another image"))
	{
    	do_computation(callback, parent,1 );
    }
	else if (menu_name == tr("delete ROIs in all tri-view planes"))
	{
		do_computation(callback, parent,2);
	}
	else if (menu_name == tr("delete xy-plane ROI"))
	{
		do_computation(callback, parent,3);
	}
	else if (menu_name == tr("delete yz-plane ROI"))
	{
		do_computation(callback, parent,4);
	}
	else if(menu_name == tr("delete zx-plane ROI"))
	{
		do_computation(callback, parent,5);
	}
	else if(menu_name == tr("copy ROIs from another image"))
	{
		do_computation(callback, parent,6);
	}
	else if (menu_name == tr("Help"))
	{
		v3d_msg("Edit Regions of Interest (ROIs) of the image of the current window: copy ROIs from another image or [paste] them to another image; or delete some of them.");		
		return;
	}
}

void do_computation(V3DPluginCallback2 &callback, QWidget *parent, int method_code)
{
	V3DLONG h;
	V3DLONG d;
	ROIList pRoiList;
	
	v3dhandleList win_list = callback.getImageWindowList();
	if(win_list.size()<1) 
	{
		QMessageBox::information(0, title, QObject::tr("No image is open."));
		return;
	}
	
	v3dhandle curwin = callback.currentImageWindow();
	
	if (method_code == 1)
	{
		ParameterDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(curwin);
		
		printf("%d %d \n",pRoiList[0].size(),win_list.size());
		
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
		}		
	}
	else if(method_code == 2)
	{
		ParameterDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[0].size(),win_list.size());
		
		//pRoiList=callback.getROI(curwin);
		
		//pRoiList.clear();
		for(int i=0;i<3;i++)
		{
			pRoiList[i].clear();
		}
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
		}
	}
	else if (method_code == 3)	
	{
		ParameterDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[0].size(),win_list.size());
		
		pRoiList[0].clear();
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
		}
		
	}
	else if (method_code == 4)
	{
		ParameterDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[1].size(),win_list.size());
		
		pRoiList[1].clear();
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
		}		
		
	}
	else if (method_code == 5)
	{
		ParameterDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[2].size(),win_list.size());
		
		pRoiList[2].clear();
		
		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
		}				
	}
	else if (method_code == 6)
	{
		ParameterDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;	
		
		long ind=dialog.combo_channel->currentIndex();
		printf("ind=%d\n",ind);
		
		pRoiList=callback.getROI(win_list[ind]);
		
		printf("%d %d \n",pRoiList[0].size(),win_list.size());
		
		
		if(callback.setROI(curwin,pRoiList))
		{
			callback.updateImageWindow(curwin);
		}		
	}
}

