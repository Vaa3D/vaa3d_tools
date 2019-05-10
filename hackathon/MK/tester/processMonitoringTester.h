#ifndef PROCESSMONITORINGTESTER_H
#define PROCESSMONITORINGTESTER_H

#include <iostream>

#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"

using namespace std;

class ProcessMonitoringTester
{
public:
	void operator()(ImgAnalyzer& clientClass)
	{
		while (1)
		{
			unique_lock<mutex> lk(clientClass.blobMergingMutex);
			if (clientClass.progressReading == 0) clientClass.monitorSwitch.wait(lk);
			if (clientClass.progressReading < 100) cout << clientClass.progressReading << endl;
			else if (clientClass.progressReading == 100) break;
			lk.unlock();
		}
	}

	void operator()(ImgTester& clientClass)
	{
		while (1)
		{
			if (clientClass.progressPercentage < 100) cout << clientClass.progressPercentage << endl;
			else if (clientClass.progressPercentage == 100) break;
		}
	}
};


#endif