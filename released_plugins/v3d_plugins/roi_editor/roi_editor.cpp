/*
 *  add_roi.cpp
 *  add_roi
 *
 *  Created by Yang, Jinzhu on 11/24/10.
 *  Add "save roi" and "load roi" functions by Jianlong Zhou. 2012-04-17
 *
 */

//log file IO
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <stdio.h>

#include "roi_editor.h"
#include "v3d_message.h"

using namespace std;


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
     << tr("Save ROI")
     << tr("Load ROI")
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
     else if (menu_name == tr("Save ROI"))
	{
          do_computation(callback, parent, 7);
     }
     else if (menu_name == tr("Load ROI"))
	{
          do_computation(callback, parent, 8);
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
     else if (method_code == 7)
	{
		pRoiList=callback.getROI(curwin);
		printf("%d %d \n",pRoiList[0].size(),win_list.size());

          // Save ROI
          QString filename = QFileDialog::getSaveFileName(
               parent,
               QObject::tr("Save ROI to file"),
               QDir::currentPath(),
               QObject::tr("text file (*.txt)") );
          if( !filename.isNull() )
          {
               // typedef QList<QPolygon>        ROIList;
               // ROI list format is
               // ROI_number: ROIList.size()
               // foreach  ith ROI (QPolygon):
               // PointNumber of QPolygon: ROI.size()
               // foreach QPoint: x y

               FILE * fp=0;
               fp = fopen((char *)qPrintable(filename), "wt");
               if (!fp)
               {
                    v3d_msg(QString("Fail to open file %1 to write.").arg(filename));
                    return;
               }

               //now save
               long  num_roi = pRoiList.size();
               fprintf(fp, "%d\n", num_roi); // num_roi

               for(long ith_roi=0; ith_roi<num_roi; ith_roi++)
               {
                    long num_pt=pRoiList[ith_roi].size(); // QPolygon have num_pt points
                    fprintf(fp, "%d\n", num_pt); // num_roi

                    for(long ith_pt=0; ith_pt<num_pt; ith_pt++)
                    {
                         fprintf(fp, "%d %d\n", pRoiList[ith_roi].at(ith_pt).x(), pRoiList[ith_roi].at(ith_pt).y() ); // x, y
                    }
               }

               //finish up
               if (fp) fclose(fp);
          }
          else
          {
               printf("Error in creating save file.");
               return;
          }

	}
     else if (method_code == 8)
	{
          QString filename = QFileDialog::getOpenFileName(
               parent,
               QObject::tr("Load ROI File"),
               QDir::currentPath(),
               QObject::tr("ROI file (*.txt);;All files (*.*)") );
          if( !filename.isEmpty() )
          {
               string file_name = filename.toStdString();

               //
               ifstream pFile(file_name.c_str());
               string str;
               long num_roi=0, num_pt=0;

               if(pFile.is_open())
               {
                    if( !pFile.eof() )
                    {
                         getline(pFile, str);
                         istringstream iss(str);
                         iss >> num_roi; //get num_roi
                         cout<< "num_roi="<<num_roi<<endl;

                         pRoiList.clear(); // prepare for loading ROI
                         for(long ith_roi=0; ith_roi<num_roi; ith_roi++)
                         {
                              cout<<"ith_roi="<<ith_roi<<endl;
                              getline(pFile, str);
                              istringstream iss_npt(str);
                              iss_npt >> num_pt; // ith ROI QPolygon have num_pt points
                              if(num_pt<=0)
                              {
                                   pRoiList.append(NULL);
                              }
                              else if(num_pt>0)
                              {
                                   QPolygon roi_poly;
                                   roi_poly.clear();
                                   for(long ith_pt=0; ith_pt<num_pt; ith_pt++)
                                   {
                                        int x, y;
                                        getline(pFile, str);
                                        istringstream iss_pt(str);
                                        iss_pt >> x >> y; // ROI QPolygon points
                                        cout<<"x, y"<< x << ","<< y<<endl;

                                        // add poly points
                                        roi_poly.push_back( QPoint(x, y) );
                                   }

                                   // add roi to roilist
                                   pRoiList.append( roi_poly );
                              }
                         }
                    }
               }
               pFile.close();
          }
          else
          {
               printf("Error in openning file.");
               return;
          }

          // select the image to be set roi
		ParameterDialog dialog(callback, parent);
		if (dialog.exec()!=QDialog::Accepted)
			return;

		long ind=dialog.combo_channel->currentIndex();
		printf("ind=%d\n",ind);


		if(callback.setROI(win_list[ind],pRoiList))
		{
			callback.updateImageWindow(win_list[ind]);
		}
	}


}

