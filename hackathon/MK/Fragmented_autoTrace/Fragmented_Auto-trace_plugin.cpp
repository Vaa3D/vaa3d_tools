/* Fragmented_Auto-trace_plugin.cpp
 * This plugin can be called from v3dr_glwidget with hot key Alt+F. Thus it can work like a standard feature in Vaa3D.
 * 2018-12-3 : by MK
 */
 
#include <vector>

#include "v3d_message.h"

#include "Fragmented_Auto-trace_plugin.h"

using namespace std;

Q_EXPORT_PLUGIN2(Fragmented_Auto-trace, FragmentedAutoTracePlugin);
 
QStringList FragmentedAutoTracePlugin::menulist() const
{
	return QStringList() 
		<<tr("start_tracing")
		<<tr("settings")
		<<tr("about");
}

QStringList FragmentedAutoTracePlugin::funclist() const
{
	return QStringList()
		<<tr("func1")
		<<tr("func2")
		<<tr("help");
}

void FragmentedAutoTracePlugin::domenu(const QString &menu_name, V3DPluginCallback2 &callback, QWidget *parent)
{
	if (menu_name == tr("settings"))
	{
		FragTraceControlPanel* panelPtr = new FragTraceControlPanel(parent, &callback);
		this->UIinstancePtr = panelPtr;
		panelPtr->exec(); // This forces the dialog to stay. Note, it is still on the SAME THREAD.
		if (this->UIinstancePtr->CViewerPortal != nullptr) this->UIinstancePtr->CViewerPortal->changeFragTraceStatus(false);
	}
	else
	{
		v3d_msg(tr("This plugin can be called from v3dr_glwidget with hot key Alt+F. Thus it can work like a standard feature in Vaa3D."
			"\n\nDevelopment in progress, MK, 2018-12-3"));
	}
}

bool FragmentedAutoTracePlugin::dofunc(const QString & func_name, const V3DPluginArgList & input, V3DPluginArgList & output, V3DPluginCallback2 & callback,  QWidget * parent)
{
	vector<char*> infiles, inparas, outfiles;
	if(input.size() >= 1) infiles = *((vector<char*> *)input.at(0).p);
	if(input.size() >= 2) inparas = *((vector<char*> *)input.at(1).p);
	if(output.size() >= 1) outfiles = *((vector<char*> *)output.at(0).p);

	if (func_name == tr("hotKey"))
	{
		if (this->UIinstancePtr != nullptr)
		{
			string inputParam = input.at(1).type.toStdString();
			
			if (!inputParam.compare("shift_e"))
			{
				if (this->UIinstancePtr->uiPtr->pushButton_12->isChecked())
				{
					this->UIinstancePtr->uiPtr->pushButton_12->setChecked(false);
					this->UIinstancePtr->eraseButtonClicked();
				}
				else if (!this->UIinstancePtr->uiPtr->pushButton_12->isChecked())
				{
					this->UIinstancePtr->uiPtr->pushButton_12->setChecked(true);
					this->UIinstancePtr->eraseButtonClicked();
				}
			}

			cout << this->UIinstancePtr->tracedTree.listNeuron.size() << endl;
		}
	}
	else if (func_name == tr("3DViewer_marker_click"))
	{
		v3dhandle currImgWindow = callback.currentImageWindow();
		QString imageName = callback.getImageName(callback.currentImageWindow());
		V3dR_MainWindow* currMainWindow = callback.find3DViewerByName(imageName);

		if (callback.getSurfaceType(currMainWindow) == 1)
		{
			this->UIinstancePtr->surType = 1;
			QList<ImageMarker> currMarkerListFrom3Dviewer = callback.send3DviewerMarkerList(currMainWindow);
			QList<ImageMarker> filteredMarkerList;
			for (QList<ImageMarker>::iterator it = currMarkerListFrom3Dviewer.begin(); it != currMarkerListFrom3Dviewer.end(); ++it)
				if (it->selected) filteredMarkerList.push_back(*it);

			if (this->UIinstancePtr->selectedMarkerList != filteredMarkerList)
			{
				this->UIinstancePtr->selectedMarkerList = filteredMarkerList;
				this->UIinstancePtr->selectedLocalMarkerList = filteredMarkerList;
				this->UIinstancePtr->updateMarkerMonitor();
			}
		}
		else if (callback.getSurfaceType(currMainWindow) == 5)
		{
			this->UIinstancePtr->surType = 5;
			QList<CellAPO> currAPOlistFrom3Dviewer = callback.send3DviewerApoList(currMainWindow);
			QList<ImageMarker> currMarkerListFrom3Dviewer;
			for (QList<CellAPO>::iterator it = currAPOlistFrom3Dviewer.begin(); it != currAPOlistFrom3Dviewer.end(); ++it)
			{
				if (it->selected)
				{
					ImageMarker newMarker;
					newMarker.name = it->name;
					newMarker.x = it->x;
					newMarker.y = it->y;
					newMarker.z = it->z;
					currMarkerListFrom3Dviewer.push_back(newMarker);
				}
			}

			if (this->UIinstancePtr->selectedMarkerList != currMarkerListFrom3Dviewer)
			{
				this->UIinstancePtr->selectedMarkerList = currMarkerListFrom3Dviewer;
				this->UIinstancePtr->selectedLocalMarkerList = currMarkerListFrom3Dviewer;
				this->UIinstancePtr->updateMarkerMonitor();
			}
		}
	}
	else if (func_name == tr("mouse_click"))
	{
		if (this->UIinstancePtr != nullptr)
		{
			string inputParam = input.at(1).type.toStdString();
			//cout << inputParam << endl;
			//system("pause");
		}
	}
	else if (func_name == tr("help"))
	{
		v3d_msg("To be implemented.");
	}
	else return false;

	return true;
}