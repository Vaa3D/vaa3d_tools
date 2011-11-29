#ifndef GLWIDGETEX_H
#define GLWIDGETEX_H

#include <vector>

#include <QObject>
#include <QtGui>
#include "../widgets/glwidget.h"

using namespace std;

class GLWidgetEX : public GLWidget
{
	//Q_OBJECT
public:
	GLWidgetEX(QWidget* parent);
	~GLWidgetEX();
	void setWidgetSize(int width, int height, int depth);
	void addTrajectory(vector<float>& trajectory, int color); 
protected:
	void paintGL();
private:
	vector<vector<float> > m_trajectorys; // m * 3*ni
	vector<int> m_colors;
};

#endif
