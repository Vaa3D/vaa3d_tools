#ifndef TERAFLYCOMMUNICATOR_H
#define TERAFLYCOMMUNICATOR_H

#include "CViewer.h"
#include "CPlugin.h"
#include "PMain.h"

class TeraflyCommunicator : public QWidget
{
	Q_OBJECT

public:
	static terafly::CViewer* currCViewerPtr;


	void test();

};



#endif