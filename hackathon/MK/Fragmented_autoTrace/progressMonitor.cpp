#include "progressMonitor.h"

ProgressMonitor::ProgressMonitor(bool& terminationSwitch) : progressDlgPtr(new Ui::progressDlg)
{
	progressDlgPtr->setupUi(this);
	this->show();
}