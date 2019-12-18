#ifndef FRAGMENTEDITOR_H
#define FRAGMENTEDITOR_H

#include <iostream>

#include <v3d_interface.h>

using namespace std;

class FragmentEditor : public QWidget
{
	Q_OBJECT;

public:
	FragmentEditor(QWidget* parent, V3DPluginCallback2* callback);

	void test(string testString) { cout << testString << endl; }

private:
	V3DPluginCallback2* thisCallback;
};



#endif