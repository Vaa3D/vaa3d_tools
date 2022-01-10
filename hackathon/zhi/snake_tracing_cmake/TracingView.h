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

/*=========================================================================
  Program:   Open Snake Tracing System
  Autohr:    Yu Wang
  Email: wangy15@rpi.edu
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $
=========================================================================*/

#ifndef TRACINGVIEW_H
#define TRACINGVIEW_H

#include <QtGui>
#include "ScatterView.h"
#include "OpenSnakeTracer.h"

struct EditValidation
{
   float TP, FP, FN, precision, recall;
   int numBranch, numSplit, numDelete, numMerge;

};

class TracingView : public QWidget
{
    Q_OBJECT
    //Q_PROPERTY(QColor penColor READ penColor WRITE setPenColor)
    //Q_PROPERTY(QImage Image READ Image WRITE setImage)
    Q_PROPERTY(int zoomFactor READ zoomFactor WRITE setZoomFactor)

public:

    TracingView(QWidget *parent = 0);
    void setPenColor(const QColor &newColor);
    void setZoomFactor(int newZoom);
    int zoomFactor() const { return zoom; }

    void drawSnakes(QPainter *painter);
	void drawSeedSnakes(QPainter *painter);
	void drawSelectedSnakes(QPainter *painter);
	void drawDeletedSnakes(QPainter *painter);
	void drawTracingSnakes(QPainter *painter);
    void drawSnakeTree(QPainter *painter);
	void drawSnakeTree_SWC(QPainter *painter);
	void drawInterestPoints(QPainter *painter);
	void drawClickedPoint(QPainter *painter);
    void drawSeedSnake(QPainter *painter);
	void drawSelectedSeeds(QPainter *painter);
    void drawSeeds(QPainter *painter);
    void drawSelection(QPainter *painter);
	void drawPath(QPainter *painter);

	void deselect();

	vnl_vector<int> getSelectedSnakes();

    void drawArrow( Point3D pt, Point3D ppt, double sze, const QPen& pen );

	void setModel(QStandardItemModel *md, ObjectSelection * sels = NULL);
    QStandardItemModel *model;

	QSize sizeHint() const;
    
public slots:
    void zoomIn();
	void zoomOut();
	void normalSize();

	void setNormalCursor();
    void setPickingCursor();

	void setDisplayImage(const QImage &newImage);
	void setImage(ImageOperation *IM);

    void setSnakes(SnakeListClass *S);
	void setSeedSnakes(SnakeListClass *s);
    void setSnakeTree(SnakeTree *s);
	void setSnakeTree_SWC(SnakeTree_SWC *s);
	void setInterestPoints(PointList3D pl);

	void setMontageView(bool in);

	void removeSnakeTree();
	void removeInterestPoints();

	void SnakesChanged();
	void SnakesChanged(int);
	void removeSnakes();

	void changeLineWidth(int);

	bool rootSnakeSelected();
	Point3D getClickedPoint();
	PointList3D getContour();

	void deleteSnake(EditValidation *edits);
	void mergeSnake(EditValidation *edits);
	void createBranch(EditValidation *edits);
	void splitSnake(EditValidation *edits);

	void deleteSeed();
	void invertSelection();

	void setTracingSnake(SnakeClass s);

    void displaySeed(bool value);
	void displaySnake(bool value);
    void displayColorLine(bool value);

	void selectRegion();

	void vtk_mousePress(double *picked_point, bool ctrl_pressed);

	void sync_selections_I(); //synchronize the selections of different views
    void sync_selections_II(); //synchronize the selections of different views

signals:
	void selected(const QItemSelection &,  QItemSelectionModel::SelectionFlags);
    void result_drawed();
	void result_drawed1();
	void manual_seed(PointList3D seed);
	void manual_path(PointList3D seed);
	void point_clicked(Point3D pt);
	//void worms_changed( Worms);
protected:
	void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
	void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);
	void keyPressEvent(QKeyEvent *event );
	void keyReleaseEvent(QKeyEvent *event );
 
private:
    void setImagePixel(const QPoint &pos, bool opaque);
    QRect pixelRect(int i, int j) const;
 
    ImageOperation *IM;
	SnakeListClass *SnakeList;
	SnakeListClass *SeedSnakes;
    
	SnakeClass snake;
	bool seed_display;
	bool snake_display;
	bool color_line_display;
	
    QColor curColor;
    QImage image;
	QImage display_image;
	bool tracing_snake;
    double zoom;
	
	ObjectSelection *snake_sels;
	ObjectSelection *snake_sels_table; //another selection for table and scatter plot
    ObjectSelection *seed_sels;

	Point3D click_point;
	PointList3D points;
	PointList3D contour;

    PointList3D path_points;
	int pathMode; // mode of picking path points

	PointList3D interest_points;
	bool interest_point_set;

	ObjectSelection *delete_sels;

	int selMode;
	QVector<QPoint> selectionRegion;	//current boundary points for group selection

	SnakeTree *snake_tree;
	SnakeTree_SWC *snake_tree_swc;
	bool tree_set;
	bool tree_set_swc;

	bool montage_view;
	int radius_state;
	int LineWidth;
	//int iteration_num;
};

#endif
