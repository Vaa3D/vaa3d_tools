#include <iostream>

#include <qsettings.h>

#include "FragTraceControlPanel.h"

using namespace std;

FragTraceControlPanel::FragTraceControlPanel()
{

}

FragTraceControlPanel::FragTraceControlPanel(QWidget* parent, V3DPluginCallback2* callback) : uiPtr(new Ui::FragmentedTraceUI), QDialog(parent)
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

FragTraceControlPanel::~FragTraceControlPanel()
{

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
	cout << "haha" << endl;
	
}