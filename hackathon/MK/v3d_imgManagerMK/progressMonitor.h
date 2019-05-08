#ifndef PROGRESSMONITOR_H
#define PROGRESSMONITOR_H

#include <iostream>

using namespace std;

class ProgressMonitor
{
public:
	int percentage;

	void operator()(int& inputPercentage)
	{
		//while (1)
		//{
			//if (inputPercentage == 100) break;
			cout << inputPercentage << endl;
		//}
	}
};

#endif