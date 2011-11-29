#ifndef _V3D_MONITOR_H__
#define _V3D_MONITOR_H__
#include <QThread>
#include "v3d_interface.h"

class V3dMonitor : public QThread
{
	Q_OBJECT
public:
	V3dMonitor(V3DPluginCallback2 * callback, v3dhandle curwin)
	{
		this->callback = callback;
		this->curwin = curwin;
		LandmarkList empty_marklist;
		callback->setLandmark(curwin, empty_marklist);
		qRegisterMetaType<LocationSimple>("LocationSimple");
		setTerminationEnabled(true);
	}
	~V3dMonitor()
	{
		if(isRunning()) this->terminate();
	}
	void run()
	{
		while(1)
		{
			v3dhandleList win_list = callback->getImageWindowList();
			if(win_list.empty())
			{
				emit win_closed();
				break;
			}
			bool is_legal = false;
			for(int i = 0; i < win_list.size(); i++)
			{
				if(curwin == win_list[i]) {is_legal = true; break;}
			}
			if(!is_legal)
			{
				emit win_closed();
				break;
			}

			if(curwin==callback->currentImageWindow())
			{
				LandmarkList landmarks = callback->getLandmark(curwin);
				if(!landmarks.empty())
				{
					emit mark_changed(landmarks.last());
				}
				LandmarkList empty_marklist;
				callback->setLandmark(curwin, empty_marklist);
			}
			usleep(5000); // sleep 1ms
		}
		exec();

	}
signals:
	void mark_changed(LocationSimple loc);
	void win_closed();
private:
	V3DPluginCallback2 * callback;
	v3dhandle curwin;
};

#endif
