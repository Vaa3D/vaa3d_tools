#ifndef _V3D_MONITOR_H__
#define _V3D_MONITOR_H__
#include <QThread>
#include "v3d_interface.h"

class MouseMonitor : public QThread
{
	Q_OBJECT
public:
	MouseMonitor(V3DPluginCallback2 * callback, v3dhandle curwin)
	{
		this->callback = callback;
		this->curwin = curwin;
		landmarks = callback->getLandmark(curwin);
		qRegisterMetaType<LocationSimple>("LocationSimple");
		setTerminationEnabled(true);
	}
	void run()
	{
		while(1)
		{
			// check whether closed ?
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
				LandmarkList new_landmarks = callback->getLandmark(curwin);
				if(new_landmarks.size() != landmarks.size())
				{
					qDebug("mark changed !");
					if(new_landmarks.size() > landmarks.size())
					{
						emit mark_changed(new_landmarks.last());
					}
					else emit mark_changed(landmarks.last());
				}
				landmarks = new_landmarks;
			}
			usleep(1000); // sleep 1ms
			//delay(1); //delay 4ms
		}
		exec();

	}
signals:
	void mark_changed(LocationSimple loc);
	void win_closed();
private:
	V3DPluginCallback2 * callback;
	v3dhandle curwin;
	LandmarkList landmarks;
};

#endif
