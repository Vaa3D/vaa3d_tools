#ifndef PROGRESSMONITOR_H
#define PROGRESSMONITOR_H

#include <iostream>

using namespace std;

int percentage;

class ProgressMonitor
{
public:
	void operator()()
	{
		while (1)
		{
			if (percentage == 100) break;
		}
	}
};

#endif