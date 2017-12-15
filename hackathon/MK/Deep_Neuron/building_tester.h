#include "Deep_Neuron_plugin.h"
#include "v3d_interface.h"

using namespace std;

class BuildingTester
{
public:
	QStringList inputFileStrings;
	QStringList inputStrings;
	QStringList outputStrings;
	void test1();
	void cropStack(unsigned char InputImagePtr[], unsigned char OutputImagePtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb, int imgX, int imgY, int imgZ);
	void maxIPStack(unsigned char inputVOIPtr[], unsigned char OutputImage2DPtr[],
		int xlb, int xhb, int ylb, int yhb, int zlb, int zhb);

	V3DPluginCallback2* theCallbackPtr;

private:
	




};