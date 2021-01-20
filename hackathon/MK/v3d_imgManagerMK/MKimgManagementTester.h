#ifndef MKIMGMANAGEMENTTESTER_H
#define MKIMGMANAGEMENTTESTER_H

#include "ImgProcessor.h"
#include "ImgAnalyzer.h"
#include "ImgManager.h"

namespace v3d_imgManagerMK
{
	class Tester
	{
	public:
		static Tester* testerInstance;
		static Tester* instance();
		static Tester* instance(const ImgAnalyzer* imgAnalyzerPtr);
		static Tester* getInstance();
		static void uninstance();
		static bool isInstantiated() { return testerInstance != nullptr; }

		shared_ptr<const ImgAnalyzer*> sharedImgAnalyzerPtr;

	private:
		Tester() {};
		Tester(const ImgAnalyzer* imgAnalyzerPtr);
	};
}

#endif