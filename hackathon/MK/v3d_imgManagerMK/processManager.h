#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <iostream>

#include "ImgManager.h"
#include "ImgAnalyzer.h"
#include "ImgProcessor.h"

class ProcessManager
{
public:
	ProcessManager(const int& inputReading) : readingFromClient(inputReading) {};

	void operator()()
	{
		while (1)
		{
			if (readingFromClient > 0) cout << readingFromClient << endl;
			else if (readingFromClient == 100) break;
		}
	}

	const int& readingFromClient;
};

#endif