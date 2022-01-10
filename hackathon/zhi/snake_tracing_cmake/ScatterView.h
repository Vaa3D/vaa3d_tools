/*=========================================================================
Copyright 2009 Rensselaer Polytechnic Institute
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. 
=========================================================================*/
#ifndef SCATTERVIEW_H
#define SCATTERVIEW_H

#include <QtGui>

#include "ObjectSelection.h"

class QRubberBand;
class PlotSettings;

class ScatterView : public QWidget
{
	Q_OBJECT

public:
	ScatterView(QWidget *parent = 0);
	void setModels(QStandardItemModel *tbl, ObjectSelection * sels = NULL);

	int ColForX(){ return columnNumForX; };
	int ColForY(){ return columnNumForY; };
	int ColForColor(){ return columnNumForColor; };
	//vtkSmartPointer<vtkTable> GetTable(){ return table; };

public slots:
	void clearSelections(void);
	void SetNormalize(bool val);
	void SetColForX(int x);
	void SetColForY(int y);
	void SetColForXY(int x, int y);
	void SetColForColor(int c);
	
	void Refresh();

protected slots:

protected:
    void paintEvent(QPaintEvent *event);
	void resizeEvent(QResizeEvent *event);
	void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent * event );
	void keyReleaseEvent(QKeyEvent *event );

private:
	void selectRegion(void);
	void refreshPixmap();
	void drawGrid(QPainter *painter);
	void drawCurves(QPainter *painter);
	void drawSelection(QPainter *painter);
	QRect getObjectRect(long int row);
	long int indexAt(QPoint &point);
	void updateAxis();
	enum {LMargin = 60, BMargin = 35, RMargin = 20, TMargin = 20};

	int columnNumForX;
	int columnNumForY;
	int columnNumForColor;

	QStandardItemModel *table;
	ObjectSelection * selection;

	QPixmap pixmap;
	PlotSettings *mySettings;

	QPoint origin;	//Set upon mouse press
	QVector<QPoint> selectionRegion;	//current boundary points for group selection
	int selMode;	//Set to 1 while control key is pressed (0 otherwise)
};


class PlotSettings
{
public:
	PlotSettings();
	void setRange(double x1, double x2, double y1, double y2);
	void adjust();

	//These are the values that the plot shows:
	double spanX() const { return maxX - minX; }
	double spanY() const { return maxY - minY; }
	double minX;
	double maxX;
	int numXTicks;
	double minY;
	double maxY;
	int numYTicks;

	bool normalize;

	//These are the values from in the actual data, needed for normalization:
	double d_minX;
	double d_minY;
	double d_maxX;
	double d_maxY;
	
private:
	static void adjustAxis(double &min, double &max, int &numTicks);
};

#endif 
