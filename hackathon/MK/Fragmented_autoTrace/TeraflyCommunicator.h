#ifndef TERAFLYCOMMUNICATOR_H
#define TERAFLYCOMMUNICATOR_H

#include <QtCore>

class TeraflyCommunicator
{

public:
	
	virtual void getCViewerInstanceTest(TeraflyCommunicator*& myCommunicator) = 0;

	

};

Q_DECLARE_INTERFACE(TeraflyCommunicator, "MK.teraflyInterfaceTest1/1.0");

#endif