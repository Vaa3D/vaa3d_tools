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
#include <math.h>

#include "ScatterView.h"

ScatterView::ScatterView(QWidget *parent) : QWidget(parent)
{
	mySettings = new PlotSettings();
	this->setFocusPolicy(Qt::StrongFocus);

	columnNumForX = 0;
	columnNumForY = 1;
	columnNumForColor = -1;

	table = NULL;
	selection = NULL;
	selMode = 0;
}

//*****************************************************************************************
// Add to the setModel function so that the axis are updated
//*****************************************************************************************
void ScatterView::setModels(QStandardItemModel *tbl, ObjectSelection * sels)
{
	this->table = tbl;
	if(!sels)
		this->selection = new ObjectSelection();
	else
		this->selection = sels;
	connect(selection, SIGNAL(changed()), this, SLOT(repaint()));

	this->update();
}

//**************************************************************************************
// SLOT: clears the selections and selectionRegion
//**************************************************************************************
void ScatterView::clearSelections(void)
{
	selectionRegion.clear();
	if(selection)
		selection->clear();
}

//********************************************************************************************
// SLOT
//********************************************************************************************
void ScatterView::SetNormalize(bool val)
{
	if( val != mySettings->normalize )
	{
		mySettings->normalize = val;
		this->update();
	}
}

//********************************************************************************************
// SLOT
//********************************************************************************************
void ScatterView::SetColForX(int x)
{
	if(!table) return;
	if (x >= 0 && x < table->columnCount())
	{
		columnNumForX = x;
		this->update();
	}
}
//********************************************************************************************
// SLOT
//********************************************************************************************
void ScatterView::SetColForY(int y)
{ 
	if(!table) return;
	if (y >= 0 && y < table->columnCount())
	{
		columnNumForY = y;
		this->update();
	}
}

void ScatterView::SetColForXY(int x, int y)
{
	if(!table) return;
	if (x >= 0 && x < table->columnCount() && y >= 0 && y < table->columnCount())
	{
		columnNumForX = x;
		columnNumForY = y;
		this->update();
	}
}

void ScatterView::SetColForColor(int c)
{
	if(!table) return;
	if(c>0 && c<table->columnCount())
	{
		columnNumForColor = c;
		this->update();
	}
}

void ScatterView::Refresh()
{
    this->update();
}
//***********************************************************************************************
//***********************************************************************************************
//***********************************************************************************************
// Mouse Events:
//***********************************************************************************************
void ScatterView::mousePressEvent(QMouseEvent *event)
{
    QWidget::mousePressEvent(event);
    origin = event->pos();

	if(!selection) return;
	int labelval = indexAt(origin);

	Qt::KeyboardModifiers modifiers = event->modifiers();
	if( selMode == 1 )
	{
		selectionRegion.append(origin);
		this->repaint();
	}
	else
	{
		if(modifiers == Qt::ControlModifier && labelval >= 0)
			selection->toggle( labelval );
		else if(modifiers == Qt::NoModifier && labelval >= 0)
			selection->select( labelval );
	}
}

void ScatterView::mouseMoveEvent(QMouseEvent *event)
{
	QWidget::mouseMoveEvent(event);
}
void ScatterView::mouseReleaseEvent(QMouseEvent *event)
{
    QWidget::mouseReleaseEvent(event);
}

void ScatterView::keyPressEvent(QKeyEvent * event)
{
	switch(event->key()) 
	{
		case Qt::Key_Shift:
			selMode = 1;
		break;
    }
	QWidget::keyPressEvent(event);
}

void ScatterView::keyReleaseEvent(QKeyEvent * event)
{
	switch(event->key())
	{
		case Qt::Key_Shift:
			this->selectRegion();
			selMode = 0;
		break;
	}
	QWidget::keyReleaseEvent(event);
}

//**************************************************************************************
// This event happens when the window is resized.  For simplicity I clear the selection
// region so that I do not have to scale it accordingly and redraw it.
//**************************************************************************************
void ScatterView::resizeEvent(QResizeEvent *event)
{
	selectionRegion.clear();
	QWidget::resizeEvent(event);
}

//*********************************************************************************************************
//Returns the ID of the item at the coordinate.
//*********************************************************************************************************
long int ScatterView::indexAt(QPoint &point)
{
	if(!table) return -1;

	long int retval = -1;
    double wx = point.x();
	double wy = point.y();

	//First check to be sure clicked location is inside of plot area
	//QRect rect(LMargin, TMargin, this->width() - (LMargin+RMargin), this->height() - (BMargin+TMargin));
	//if ( (wx > rect.left()) && (wx < rect.right()) && (wy > rect.top()) && (wy < rect.bottom()) )
	//{
		for (int row = 0; row < table->rowCount(); ++row) 
		{
			QRect oRect = getObjectRect(row);

			if ( ( wx >= oRect.left() ) && ( wx <= oRect.right() ) && ( wy >= oRect.top() ) && ( wy <= oRect.bottom() ) ) 
			{
				//returns the index of the first column which is ID
				retval = row;
				break;
			}
		}
	//}
	return retval;
}

//**************************************************************************************
// All objects inside the a region defined by selectionRegion will be selected.
//**************************************************************************************
void ScatterView::selectRegion(void)
{
	if (selMode != 1)
		return;

	if(selectionRegion.size() <= 0)
		return;

	if(!table)
		return;

	//Make the last point the first point to create a closed loop
	selectionRegion.append( selectionRegion[0] );
	int numPoints = selectionRegion.size();

	//Turn my list of clicks into a path
	QPainterPath path;
	path.moveTo(selectionRegion[0]);
	for (int i=0; i < numPoints; i++)
	{
		path.lineTo(selectionRegion[i]);
	}
	//Draw the path in an image
	//QRect rect(LMargin, TMargin, this->width() - (LMargin+RMargin), this->height() - (BMargin+TMargin));
	QRect rect(0,0,this->width(), this->height());
	QImage img(rect.width(),rect.height(),QImage::Format_Mono);
	img.fill(Qt::white);
	QPainter painter(&img);
	painter.setPen(Qt::black);
	painter.setBrush(Qt::black);
	painter.drawPath(path);
	//Turn the image into a Region
	QBitmap bitmap = QBitmap::fromImage(img, Qt::MonoOnly | Qt::ThresholdDither);
	QRegion region(bitmap);

	//Get the Ids of the objects within this region and select them:
	std::set<long int> sels;
	int rows = table->rowCount();
	for (int row = 0; row < rows; ++row) 
	{
        QRegion point = getObjectRect( row );
		//if it intersects with the selection region, save the index
		if (!point.intersect(region).isEmpty())
		{
			sels.insert( row );
		}
    }
	selectionRegion.clear();
	selection->add(sels);
}

//***********************************************************************************************
//***********************************************************************************************
//***********************************************************************************************

//*********************************************************************************************************
// Find the maximum and minimum values of the data points and update the axis to show all data
//*********************************************************************************************************
void ScatterView::updateAxis(void)
{
	if(!table) return;

	double x1 = 0;
	double x2 = 0;
	double y1 = 0;
	double y2 = 0;
	double v = 0;
	int rows = table->rowCount();

	if (rows > 0)
	{	//initialize values
		//x1 = table->GetValue(0,columnNumForX).ToDouble();
		x1 = table->data(table->index(0, columnNumForX, QModelIndex())).toDouble();
		x2 = x1;
		//y1 = table->GetValue(0,columnNumForY).ToDouble();
		y1 = table->data(table->index(0, columnNumForY, QModelIndex())).toDouble();
		y2 = y1;
	}
	for (int row = 1; row < rows; ++row) 
	{	//find min/max of each axis
		//v = table->GetValue(row,columnNumForX).ToDouble();
		v = table->data(table->index(row, columnNumForX, QModelIndex())).toDouble();
		if (v < x1) x1 = v;
		else if (v > x2) x2 = v;
		//v = table->GetValue(row,columnNumForY).ToDouble();
		table->data(table->index(row, columnNumForY, QModelIndex())).toDouble();
		if (v < y1) y1 = v;
		else if (v > y2) y2 = v;
    }

	//Save them
	this->mySettings->d_minX = x1;
	this->mySettings->d_maxX = x2;
	this->mySettings->d_minY = y1;
	this->mySettings->d_maxY = y2;

	if(this->mySettings->normalize)
	{
		mySettings->setRange(-1,1,-1,1);
	}
	else
	{
		//now extend axis slightly so all data points will be visible
		double xrange = x2-x1;
		if(xrange == 0)
			xrange = 0.1;
		double yrange = y2-y1;
		if(yrange == 0)
			yrange = 0.1;
		double xp = .03*double(xrange);
		double yp = .03*double(yrange);
		mySettings->setRange(x1-xp,x2+xp,y1-yp,y2+yp);
	}
	mySettings->adjust();
}

//**************************************************************************************
//Repaints the view.  Called automatically
//**************************************************************************************
void ScatterView::paintEvent(QPaintEvent *event)
{
	refreshPixmap();
	QStylePainter painter(this);
	painter.drawPixmap(0, 0, pixmap);
}

 //***************************************************************************************************
//The refreshPixmap() function redraws the plot onto the off-screen pixmap and updates the display.
//We resize the pixmap to have the same size as the widget and fill it with the widget's erase color.
//This color is the "dark" component of the palette, because of the call to setBackgroundRole() in the
//Plotter constructor. If the background is a non-solid brush, QPixmap::fill() needs to know the
//offset in the widget where the pixmap will end up to align the brush pattern correctly. Here, the
//pixmap corresponds to the entire widget, so we specify position (0, 0).
//Then we create a QPainter to draw on the pixmap. The initFrom() call sets the painter's pen,
//background, and font to the same ones as the Plotter widget. Next we call drawGrid() and
//drawCurves() to perform the drawing. At the end, we call update() to schedule a paint event for the
//whole widget. The pixmap is copied to the widget in the paintEvent() function (p. 123).
//***************************************************************************************************
void ScatterView::refreshPixmap()
{
	this->updateAxis();
	pixmap = QPixmap(size());
	pixmap.fill(QColor(220,220,220));
	QPainter painter(&pixmap);
	painter.initFrom(this);
	drawGrid(&painter);
	drawCurves(&painter);
	drawSelection(&painter);
}

//***************************************************************************************************
//The drawGrid() function draws the grid behind the curves and the axes. The area on which we draw
//the grid is specified by rect. If the widget isn't large enough to accommodate the graph, we return
//immediately.
//The first for loop draws the grid's vertical lines and the ticks along the x axis. The second for loop
//draws the grid's horizontal lines and the ticks along the y axis. At the end, we draw a rectangle
//along the margins. The drawText() function is used to draw the numbers corresponding to the tick
//marks on both axes.
//The calls to drawText() have the following syntax:
//painter->drawText(x, y, width, height, alignment, text);
//where (x, y, width, height) define a rectangle, alignment the position of the text within that
//rectangle, and text the text to draw.
//***************************************************************************************************
void ScatterView::drawGrid(QPainter *painter)
{	
	//This rectangle is the graph area
	QRect rect(LMargin, TMargin, this->width() - (LMargin+RMargin), this->height() - (BMargin+TMargin));
	if (!rect.isValid())
		return;
	//Retrieve current plotSettings
	//PlotSettings settings = PlotSettings();
	PlotSettings settings = *mySettings;
	QPen quiteDark = palette().dark().color().light();
	QPen light = palette().light().color();
	//Draw vertical lines
	for (int i = 0; i <= settings.numXTicks; ++i) 
	{
		int x = rect.left() + (i * (rect.width() - 1) / settings.numXTicks);
		double label = settings.minX + (i * settings.spanX() / settings.numXTicks);
		//painter->setPen(quiteDark);
		painter->setPen(QPen(QBrush(Qt::black),1,Qt::DotLine));
		painter->drawLine(x, rect.top(), x, rect.bottom());
		//painter->setPen(light);
		painter->drawLine(x, rect.bottom(), x, rect.bottom() + 5);
		painter->setPen(QPen(QBrush(Qt::black),1));
		painter->drawText(x - 50, rect.bottom() + 5, 100, 15, Qt::AlignHCenter | Qt::AlignTop, QString::number(label));
	}
	for (int j = 0; j <= settings.numYTicks; ++j) 
	{
		int y = rect.bottom() - (j * (rect.height() - 1) / settings.numYTicks);
		double label = settings.minY + (j * settings.spanY() / settings.numYTicks);
		//painter->setPen(quiteDark);
		painter->setPen(QPen(QBrush(Qt::black),1,Qt::DotLine));
		painter->drawLine(rect.left(), y, rect.right(), y);
		//painter->setPen(light);
		painter->drawLine(rect.left() - 5, y, rect.left(), y);
		painter->setPen(QPen(QBrush(Qt::black),1));
		painter->drawText(rect.left() - LMargin, y - 10, LMargin - 5, 20,
			Qt::AlignRight | Qt::AlignVCenter,
			QString::number(label));
	}
	painter->drawRect(rect.adjusted(0, 0, -1, -1));
	
	//Now draw the labels for the x and y axis:
	QString xName;
	QString yName;
	if(table)
	{
		//xName = QString( table->GetColumnName(columnNumForX) );
		xName = table->headerData(columnNumForX, Qt::Horizontal).toString();
		//yName = QString( table->GetColumnName(columnNumForY) );
		yName = table->headerData(columnNumForY, Qt::Horizontal).toString();
	}
	else
	{
		xName = QString("x");
		yName = QString("y");
	}

    QFont fp = painter->font();
    fp.setBold(true);
    painter->setFont(fp);
	painter->setPen(Qt::blue);
	painter->drawText(rect.left(),rect.bottom() + 20, rect.width(), 20, Qt::AlignHCenter, xName);
	painter->save();
	painter->rotate(-90);
	painter->drawText(-1*rect.bottom(), rect.top() - TMargin, rect.height(), 20, Qt::AlignHCenter, yName);
	painter->restore();
}

//***************************************************************************************************
//The drawCurves() function draws the curves on top of the grid. We start by calling setClipRect() to
//set the QPainter's clip region to the rectangle that contains the curves (excluding the margins and
//the frame around the graph). QPainter will then ignore drawing operations on pixels outside the
//area.
//***************************************************************************************************
void ScatterView::drawCurves(QPainter *painter)
{
	if(!table)
		return;

	const int numColors = 5;
	//Yellow is reserved for selections!!!!
	QColor defaultColor[numColors] = {Qt::cyan, Qt::yellow, Qt::green, Qt::red, Qt::blue};

	QVector<QRect> rectangles( table->rowCount() );
	for (int row = 0; row < table->rowCount(); ++row) 
	{
		rectangles[row] = getObjectRect(row);

		//int c = table->GetValue(row,columnNumForColor).ToInt();
		int c = table->data(table->index(row, columnNumForColor, QModelIndex())).toInt();
		QColor myColor = defaultColor[ c % numColors ];

		painter->setPen(Qt::black);
		//painter->setPen(myColor);
		if(selection)
		{
			if(selection->isSelected( row ) )
			{
				continue;
			}
		}
		painter->setBrush(QBrush(myColor,Qt::SolidPattern));
		painter->drawRect(rectangles[row]);
	}
	for (int row = 0; row < table->rowCount(); ++row) 
	{
		QColor myColor = Qt::yellow;//Qt::yellow;
				painter->setPen(Qt::black);
				painter->setBrush(QBrush(myColor,Qt::SolidPattern));
		rectangles[row] = getObjectRect(row);

		//int c = table->GetValue(row,columnNumForColor).ToInt();
		//QColor myColor = defaultColor[ c % numColors ];

		//painter->setPen(Qt::black);
		//painter->setPen(myColor);
		if(selection)
		{
			if(selection->isSelected( row ) )
			{
				painter->drawRect(rectangles[row]);
			}
		}
	}
}

//**************************************************************************************
// This function draws the lines connecting the points in selectionRegion
//**************************************************************************************
void ScatterView::drawSelection(QPainter *painter)
{
	if (selMode == 1)
	{
		painter->setPen(Qt::black);
		for (int i=1; i < selectionRegion.size(); i++)
		{
			painter->drawLine(selectionRegion[i-1],selectionRegion[i]);
		}
	}
}

QRect ScatterView::getObjectRect(long int row)
{
	if(!table) return QRect();

	QRect rect(LMargin, TMargin, this->width() - (LMargin+RMargin), this->height() - (BMargin+TMargin));
	PlotSettings settings = *mySettings;

	//double valueX = table->GetValue(row,columnNumForX).ToDouble();
    double valueX = table->data(table->index(row, columnNumForX, QModelIndex())).toDouble();
	//double valueY = table->GetValue(row,columnNumForY).ToDouble();
    double valueY = table->data(table->index(row, columnNumForY, QModelIndex())).toDouble();
	//Adjust for normalization:
	if(settings.normalize == true)
	{
		valueX = -1 + 2* (valueX - settings.d_minX)/(settings.d_maxX - settings.d_minX);
		valueY = -1 + 2* (valueY - settings.d_minY)/(settings.d_maxY - settings.d_minY);
	}

    double dx = valueX - settings.minX;
	double dy = valueY - settings.minY;
	double x = rect.left() + (dx * (rect.width() - 1) / settings.spanX());
	double y = rect.bottom() - (dy * (rect.height() - 1) / settings.spanY());
	return QRect(int(x-2),int(y-2),5,5);
}


//***************************************************************************************************
//The PlotSettings constructor initializes both axes to the range 0 to 10 with 5 tick marks.
//***************************************************************************************************
PlotSettings::PlotSettings()
{
	minX = 0.0;
	maxX = 10.0;
	numXTicks = 5;
	minY = 0.0;
	maxY = 10.0;
	numYTicks = 5;
	normalize = false;
}

//***************************************************************************************************
// Allows for the adjustment of minX, maxX, minY, maxY
//***************************************************************************************************
void PlotSettings::setRange(double x1, double x2, double y1, double y2)
{
	minX = x1;
	maxX = x2;
	minY = y1;
	maxY = y2;
}

//***************************************************************************************************
//The adjust() function is called from mouseReleaseEvent() to round the minX, maxX, minY, and maxY
//values to "nice" values and to determine the number of ticks appropriate for each axis. The private
//function adjustAxis() does its work one axis at a time.
//***************************************************************************************************
void PlotSettings::adjust()
{
	adjustAxis(minX, maxX, numXTicks);
	adjustAxis(minY, maxY, numYTicks);
}

//***************************************************************************************************
//The adjustAxis() function converts its min and max parameters into "nice" numbers and sets its
//numTicks parameter to the number of ticks it calculates to be appropriate for the given [min, max]
//range. Because adjustAxis() needs to modify the actual variables (minX, maxX, numXTicks, etc.) and
//not just copies, its parameters are non-const references.
//Most of the code in adjustAxis() simply attempts to determine an appropriate value for the interval
//between two ticks (the "step"). To obtain nice numbers along the axis, we must select the step with
//care. For example, a step value of 3.8 would lead to an axis with multiples of 3.8, which is difficult
//for people to relate to. For axes labeled in decimal notation, "nice" step values are numbers of the
//form 10n, 2?0n, or 5?0n.
//We start by computing the "gross step", a kind of maximum for the step value. Then we find the
//corresponding number of the form 10n that is smaller than or equal to the gross step. We do this by
//taking the decimal logarithm of the gross step, rounding that value down to a whole number, then
//raising 10 to the power of this rounded number. For example, if the gross step is 236, we compute
//log 236 = 2.37291? then we round it down to 2 and obtain 102 = 100 as the candidate step value
//of the form 10n.
//Once we have the first candidate step value, we can use it to calculate the other two candidates:
//2?0n and 5?0n. For the example above, the two other candidates are 200 and 500. The 500
//candidate is larger than the gross step, so we can't use it. But 200 is smaller than 236, so we use
//200 for the step size in this example.
//It's fairly easy to derive numTicks, min, and max from the step value. The new min value is obtained
//by rounding the original min down to the nearest multiple of the step, and the new max value is
//obtained by rounding up to the nearest multiple of the step. The new numTicks is the number of
//intervals between the rounded min and max values. For example, if min is 240 and max is 1184 upon
//entering the function, the new range becomes [200, 1200], with 5 tick marks.
//This algorithm will give suboptimal resultsin some cases. A more sophisticated algorithm is
//described in Paul S. Heckbert's article "Nice Numbers for Graph Labels" published in Graphics
//Gems (ISBN 0-12-286166-3).
//***************************************************************************************************
void PlotSettings::adjustAxis(double &min, double &max,int &numTicks)
{
	const int MinTicks = 4;
	double grossStep = (max - min) / MinTicks;
	double step = pow(10.0, floor(log10(grossStep)));
	if (5 * step < grossStep) {
		step *= 5;
	} 
	else if (2 * step < grossStep) {
		step *= 2;
	}
	numTicks = int(ceil(max / step) - floor(min / step));
	if (numTicks < MinTicks)
		numTicks = MinTicks;
	min = floor(min / step) * step;
	max = ceil(max / step) * step;
}
