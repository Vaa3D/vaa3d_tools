#ifndef CELL_TRACK_EX_H_H
#define CELL_TRACK_EX_H_H

#include <QObject>
#include "../../CT3D/cell_track.h"

class CellTrackEX :public QObject, public CellTrack
{
	Q_OBJECT
public:
	CellTrackEX();
	~CellTrackEX();
	 /*static*/ bool createFramesFromTrees(ComponentTree* tree1, ComponentTree* tree2,vector<Frame*> &frames );
	 /*static*/ bool createFramesFromTrees(vector<char*> tree_files,vector<Frame*> &frames);
	virtual /*static*/ bool createFramesFromImages(vector<char*> img_files,vector<Frame*> &frames );
signals:
	void setProgressValue(int);
};

#endif
