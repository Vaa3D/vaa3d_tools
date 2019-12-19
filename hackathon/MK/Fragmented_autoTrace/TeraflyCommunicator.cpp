#include "TeraflyCommunicator.h"

terafly::CViewer* TeraflyCommunicator::currCViewerPtr = nullptr;

void TeraflyCommunicator::test()
{
	terafly::PluginInterface::getCViewerInstance(TeraflyCommunicator::currCViewerPtr);
}

void terafly::PluginInterface::getCViewerInstance(CViewer*& currCViewer_teraflyCommunicator)
{
	currCViewer_teraflyCommunicator = terafly::CViewer::getCurrent();
}