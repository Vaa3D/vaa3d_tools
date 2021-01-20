#include "MKimgManagementTester.h"

using namespace std;

// singleton testing class
v3d_imgManagerMK::Tester* v3d_imgManagerMK::Tester::testerInstance = nullptr;

v3d_imgManagerMK::Tester::Tester(const ImgAnalyzer* imgAnalyzerPtr)
{
	this->sharedImgAnalyzerPtr = make_shared<const ImgAnalyzer*>(imgAnalyzerPtr);
}

v3d_imgManagerMK::Tester* v3d_imgManagerMK::Tester::instance()
{
	v3d_imgManagerMK::Tester::testerInstance = new v3d_imgManagerMK::Tester();
	return v3d_imgManagerMK::Tester::testerInstance;
}

v3d_imgManagerMK::Tester* v3d_imgManagerMK::Tester::instance(const ImgAnalyzer* imgAnalyzerPtr)
{
	v3d_imgManagerMK::Tester::testerInstance = new v3d_imgManagerMK::Tester(imgAnalyzerPtr);
	return v3d_imgManagerMK::Tester::testerInstance;
}

v3d_imgManagerMK::Tester* v3d_imgManagerMK::Tester::getInstance()
{
	if (v3d_imgManagerMK::Tester::isInstantiated()) return v3d_imgManagerMK::Tester::testerInstance;
	else return nullptr;
}

void v3d_imgManagerMK::Tester::uninstance()
{
	if (v3d_imgManagerMK::Tester::testerInstance != nullptr) delete v3d_imgManagerMK::Tester::testerInstance;
	v3d_imgManagerMK::Tester::testerInstance = nullptr;
}