#include <iostream>

#include <qsettings.h>

#include "FragTraceControlPanel.h"

using namespace std;

FragTraceControlPanel::FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback, bool showMenu) : uiPtr(new Ui::FragmentedTraceUI), thisCallback(callback), QDialog(parent)
{
	if (showMenu)
	{
		uiPtr->setupUi(this);

		QSettings callOldSettings("SEU-Allen", "Fragment tracing");
		if (callOldSettings.value("wholeBlock") == true)
		{
			uiPtr->radioButton->setChecked(true);
			uiPtr->radioButton_2->setChecked(false);
		}
		else if (callOldSettings.value("withSeed") == true)
		{
			uiPtr->radioButton->setChecked(false);
			uiPtr->radioButton_2->setChecked(true);
		}

		this->show();
	}
	else this->traceButtonClicked();
}

void FragTraceControlPanel::saveSettingsClicked()
{
	QSettings settings("SEU-Allen", "Fragment tracing");
	if (uiPtr->radioButton->isChecked())
	{
		settings.setValue("wholeBlock", true);
		settings.setValue("withSeed", false);
	}
	else if (uiPtr->radioButton_2->isChecked())
	{
		settings.setValue("wholeBlock", false);
		settings.setValue("withSeed", true);
	}
}

void FragTraceControlPanel::traceButtonClicked()
{
	QSettings currSettings("SEU-Allen", "Fragment tracing");
	cout << "Fragment tracing procedure initiated." << endl;
	if (currSettings.value("wholeBlock") == true && currSettings.value("withSeed") == false)
	{
		cout << " whole block tracing, acquiring image information.." << endl;
		const Image4DSimple* currImgPtr = thisCallback->getImageTeraFly();
		cout << " -- Current image block dimensions: " << currImgPtr->getXDim() << " " << currImgPtr->getYDim() << " " << currImgPtr->getZDim() << endl;
		
		this->traceManagerPtr = new FragTraceManager(currImgPtr);
		connect(this, SIGNAL(switchOnSegPipe()), this->traceManagerPtr, SLOT(imgProcPipe_wholeBlock()));
		emit switchOnSegPipe();
	}
	else if (currSettings.value("wholeBlock") == false && currSettings.value("withSeed") == true)
	{
		cout << " trace with given seed point, acquiring image information.." << endl;
	}

}