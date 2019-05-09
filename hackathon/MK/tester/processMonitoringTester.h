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
			if (clientClass.progressReading < 100) cout << clientClass.progressReading << endl;
			else if (clientClass.progressReading == 100) break;
		}
	}
};


#endif