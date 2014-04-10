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

  Program:   Farsight Biological Image Segmentation and Visualization Toolkit
  Language:  C++
  Date:      $Date:  $
  Version:   $Revision: 0.00 $

=========================================================================*/
#include "TracingView.h"

TracingView::TracingView(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StaticContents);
    setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    setFocusPolicy(Qt::StrongFocus);

    curColor = Qt::black;
    zoom = 1;

	radius_state = 0;

	SnakeList = new SnakeListClass;
	SeedSnakes = new SnakeListClass;
	snake_tree = new SnakeTree;
	snake_tree_swc = new SnakeTree_SWC;

    image = QImage(5, 5, QImage::Format_ARGB32);
    image.fill(qRgba(255, 0, 0, 0));

	seed_display = true;

	snake_display = true;

	color_line_display = false;

	tracing_snake = false;

	tree_set = false;
	tree_set_swc = false;

	interest_point_set = false;

	IM = NULL;

	LineWidth = 2;

	snake_sels = new ObjectSelection;
	seed_sels = new ObjectSelection;
	delete_sels = new ObjectSelection;

	selMode = 0;
	pathMode = 0;

	//iteration_num = 1;

	connect(snake_sels, SIGNAL(changed()), this, SLOT(sync_selections_I()));
}

void TracingView::changeLineWidth(int in)
{
    LineWidth = in;
	update();
    updateGeometry();
}

void TracingView::setModel(QStandardItemModel *md, ObjectSelection * sels)
{
    model = md;
	//snake_sels = sels;
	snake_sels_table = sels;
	connect(snake_sels_table, SIGNAL(changed()), this, SLOT(repaint()));

	connect(snake_sels_table, SIGNAL(changed()), this, SLOT(sync_selections_II()));
}

void TracingView::setNormalCursor()
{
    setCursor(Qt::ArrowCursor);
}

void TracingView::setPickingCursor()
{
	setCursor(Qt::CrossCursor);
}

void TracingView::setPenColor(const QColor &newColor)
{
    curColor = newColor;
}

void TracingView::setZoomFactor(int newZoom)
{
}

void TracingView::zoomIn()
{
    zoom = zoom * 1.2;
	update();
	updateGeometry();
}

void TracingView::zoomOut()
{
    zoom = zoom * 0.8;
    update();
	updateGeometry();
}

void TracingView::normalSize()
{
    zoom = 1;
	update();
	updateGeometry();
}

QSize TracingView::sizeHint() const
{
    QSize size = zoom * image.size();
    //if (zoom >= 3)
    //    size += QSize(1, 1);
    return size;
}

void TracingView::setDisplayImage(const QImage &newImage)
{
    if (newImage != image) {
        image = newImage.convertToFormat(QImage::Format_ARGB32);
    }
	//normalSize();
	tracing_snake = false;
	update();
	updateGeometry();
}


void TracingView::setImage(ImageOperation *in)
{
   IM = in;
   tracing_snake = false;
   update();
   updateGeometry();
}


void TracingView::setTracingSnake(SnakeClass s)
{
	tracing_snake = true;
	snake = s;
	update();
	updateGeometry();
}

void TracingView::setSnakeTree(SnakeTree *s)
{
	tree_set = true;
    snake_tree = s;
	update();
	updateGeometry();
}

void TracingView::setSnakeTree_SWC(SnakeTree_SWC *s)
{
	tree_set_swc = true;
    snake_tree_swc = s;
	update();
	updateGeometry();
}


void TracingView::setInterestPoints(PointList3D pl)
{
	interest_points = pl;
	interest_point_set = true;
	update();
	updateGeometry();
}

void TracingView::setMontageView(bool in)
{
    montage_view = in;
}

void TracingView::removeInterestPoints()
{
    interest_point_set = false;
	update();
	updateGeometry();
}

void TracingView::removeSnakeTree()
{
	tree_set = false;
	tree_set_swc = false;
	update();
	updateGeometry();
}

void TracingView::setSnakes(SnakeListClass *S)
{
	SnakeList = S;
   /*tracing_snake = false;
   if( S->NSnakes != 0 )
   {
	//SnakeList->SetNSpace(S->NSnakes);
	SnakeList = S;
	//std::cout<<"S-NSnakes:"<<S->NSnakes<<std::endl;
	update();
	updateGeometry();
   }
   else
   {
	 emit result_drawed();
   } */
}

void TracingView::setSeedSnakes(SnakeListClass *S)
{
	SeedSnakes = S;
	update();
	updateGeometry();
}

void TracingView::SnakesChanged()
{
	tracing_snake = false;
	click_point.x = 0;
	click_point.y = 0;
	click_point.z = 0;
    if( SnakeList->NSnakes != 0 )
	{
	  snake_sels->clear();
	  update();
	  updateGeometry();
	}
	else
	{
	  emit result_drawed();
	}
}
void TracingView::SnakesChanged(int in)
{
	tracing_snake = false;
	click_point.x = 0;
	click_point.y = 0;
	click_point.z = 0;
    if( SnakeList->NSnakes != 0 )
	{
	  snake_sels->clear();
	  radius_state = in;
	  update();
	  updateGeometry();
	}
}

void TracingView::removeSnakes()
{
    //SnakeList.NSnakes = 0;
	//SnakeList->NSnakes = 0;
	SnakeList->RemoveAllSnakes();
	tracing_snake = false;
	update();
	updateGeometry();
}

void TracingView::deleteSeed()
{
   if( IM )
   {
	for( int i = 0; i < IM->SeedPt.GetSize(); i++ )
	{
	  if( seed_sels->isSelected(i) )
	  {
		  IM->visit_label(i) = 1;
	  }
	}

	seed_sels->clear(); 

    update();
	updateGeometry();
   }
}

void TracingView::invertSelection()
{
   if( IM )
   {
	for( int i = 0; i < IM->SeedPt.GetSize(); i++ )
	{
		seed_sels->toggle(i);
	}

    update();
	updateGeometry();
   }
}

bool TracingView::rootSnakeSelected()
{
	bool root_set = false;
    std::set<long int> set = snake_sels->getSelections();
	std::vector<int> snake_id;
	if( set.size() == 1 )
	{
	 for (std::set<long int>::iterator it = set.begin(); it != set.end(); it++)
	 {
		 snake_id.push_back(*it);
	 }

     int idx_snake = snake_id[0];
	 
	 if( SnakeList->valid_list[idx_snake] == 0 )
		 return root_set;

	 float dist_head = click_point.GetDistTo( SnakeList->Snakes[idx_snake].Cu.GetLastPt() );
	 float dist_tail = click_point.GetDistTo( SnakeList->Snakes[idx_snake].Cu.GetFirstPt() );
	 if( dist_head < dist_tail )
	 {
	    SnakeList->Snakes[idx_snake].Ru =  SnakeList->Snakes[idx_snake].Cu.Flip_4D(SnakeList->Snakes[idx_snake].Ru);
	 }
     root_set = true;
	}
	return root_set;
}

Point3D TracingView::getClickedPoint()
{
   return click_point;
}

PointList3D TracingView::getContour()
{
   return contour;
}

void TracingView::deleteSnake(EditValidation *edits)
{
	float length = 0;
    for( int i = 0; i < SnakeList->NSnakes; i++ )
	{
	  if( snake_sels->isSelected(i) )
	  {
		  length = SnakeList->Snakes[i].Cu.GetLength();
		  SnakeList->RemoveSnake(i);
		  delete_sels->toggle(i);
	  }
	}

	edits->numDelete++;
	//deleted snake is counted as false positive
    edits->FP += length;
	edits->TP -= edits->FP;
	edits->precision = edits->TP/(edits->TP+edits->FP);
    edits->recall = edits->TP/(edits->TP+edits->FN);

	snake_sels->clear();
    update();
	updateGeometry();
}

void TracingView::mergeSnake(EditValidation *edits)
{
	std::set<long int> set = snake_sels->getSelections();
	std::vector<int> snake_id;

   if( set.size() == 2 )
   {
	 for (std::set<long int>::iterator it = set.begin(); it != set.end(); it++)
	 {
		 snake_id.push_back(*it);
	 }

	   if( SnakeList->valid_list[snake_id[0]] == 0 || SnakeList->valid_list[snake_id[1]] == 0 )
		 return;

	  SnakeList->MergeSnake(snake_id[0],snake_id[1], true);
   }

   edits->numMerge++;

   snake_sels->clear();
   update();
   updateGeometry();
}

void TracingView::createBranch(EditValidation *edits)
{
    std::set<long int> set = snake_sels->getSelections();
	std::vector<int> snake_id;

   if( set.size() == 2 )
   {
	 for (std::set<long int>::iterator it = set.begin(); it != set.end(); it++)
	 {
		 snake_id.push_back(*it);
	 }

	   if( SnakeList->valid_list[snake_id[0]] == 0 || SnakeList->valid_list[snake_id[1]] == 0 )
		 return;

	   SnakeList->CreateBranch(snake_id[0],snake_id[1]);
   }

   edits->numBranch++;

   snake_sels->clear();
   update();
   updateGeometry();
}

void TracingView::splitSnake(EditValidation *edits)
{
	int idx_snake = -1;
	int idx_pt = -1;
	std::set<long int> set = snake_sels->getSelections();
	std::vector<int> snake_id;

   if( set.size() == 1 )
   {
     for (std::set<long int>::iterator it = set.begin(); it != set.end(); it++)
	 {
		 snake_id.push_back(*it);
	 }

     idx_snake = snake_id[0];
	 
	 if( SnakeList->valid_list[idx_snake] == 0 )
		 return;

	 vnl_vector<float> dist_temp( SnakeList->Snakes[idx_snake].Cu.GetSize() );
	 for( int j = 0; j < SnakeList->Snakes[idx_snake].Cu.GetSize(); j++ )
	 {
	   dist_temp(j) = click_point.GetDistTo( SnakeList->Snakes[idx_snake].Cu.Pt[j] );
	 }
     idx_pt = dist_temp.arg_min();

	 if( idx_pt == 0 || idx_pt == SnakeList->Snakes[idx_snake].Cu.GetSize() - 1 )
	   idx_pt = -1;
   }

	if( idx_snake != -1 && idx_pt != -1 )
	{
      SnakeList->SplitSnake(idx_snake, idx_pt);
	}

	edits->numSplit++;

	update();
	updateGeometry();
}

void TracingView::displaySeed(bool value)
{
   seed_display = value;
   update();
   updateGeometry();
}

void TracingView::displaySnake(bool value)
{ 
   snake_display = value;
   update();
   updateGeometry();
}

void TracingView::displayColorLine(bool value)
{
   color_line_display = value;
   update();
   updateGeometry();
}

void TracingView::sync_selections_I()
{
   int id_valid = -1;
   
   if( !snake_sels_table )
	   return;

   disconnect(snake_sels_table, SIGNAL(changed()), this, SLOT(sync_selections_II()));
   snake_sels_table->clear();

   for( int i = 0; i < SnakeList->NSnakes; i++ )
   {
     if( SnakeList->valid_list[i] != 0 )
	 {
		 id_valid++;
         if( snake_sels->isSelected(i) )
			 snake_sels_table->add(id_valid);
	 }
   }

   this->update();
   connect(snake_sels_table, SIGNAL(changed()), this, SLOT(sync_selections_II()));

}

void TracingView::sync_selections_II()
{
   int id_valid = -1;

   disconnect(snake_sels, SIGNAL(changed()), this, SLOT(sync_selections_I()));
   snake_sels->clear();
   
   for( int i = 0; i < SnakeList->NSnakes; i++ )
   {
     if( SnakeList->valid_list[i] != 0 )
	 {
		 id_valid++;
         if( snake_sels_table->isSelected(id_valid) )
			 snake_sels->add(i);
	 }
   }

   this->update();
   connect(snake_sels, SIGNAL(changed()), this, SLOT(sync_selections_I()));
}


void TracingView::vtk_mousePress(double *picked_point, bool ctrl_pressed)
{  
   if( SnakeList->NSnakes != 0 )
   {
      //find the snake nearest to clicked point
	  //int SM = SnakeList->IM->I->GetLargestPossibleRegion().GetSize()[0];
	  //int SN = SnakeList->IM->I->GetLargestPossibleRegion().GetSize()[1];
      //int SZ = SnakeList->IM->I->GetLargestPossibleRegion().GetSize()[2];
	  bool snake_found = false;
      //for(int i = 0; i < SZ; i++ )
	  //{
		click_point.x = picked_point[0];
        click_point.y = picked_point[1];
	    click_point.z = picked_point[2];
		//click_point.check_out_of_range_3D(SM,SN,SZ);
		//index[0] = click_point.x;
		//index[1] = click_point.y;
		//index[2] = click_point.z;
		click_point.z = -1;

		std::vector<float> dist_all;
		std::vector<int> id_all;
		std::vector<float> z_all;
		for( int j = 0; j < SnakeList->NSnakes; j++ )
		{
		  if( SnakeList->valid_list[j] == 0 )
		  {
		    dist_all.push_back(10000);
			id_all.push_back(j);
			z_all.push_back(0);
			continue;
		  }
		  for( int k = 0; k < SnakeList->Snakes[j].Cu.NP; k++ )
		  {
		    float dist_temp = SnakeList->Snakes[j].Cu.Pt[k].GetDistTo2D(click_point);
			dist_all.push_back(dist_temp);
			id_all.push_back(j);
			z_all.push_back(SnakeList->Snakes[j].Cu.Pt[k].z);
		  }
		}
	    
		vnl_vector<float> dist_all_vnl(dist_all.size());
		for(int j = 0; j < dist_all.size(); j++ )
		{
		  dist_all_vnl(j) = dist_all[j];
		}
		if( dist_all_vnl.min_value() < 10 )
		{
			int idx = dist_all_vnl.arg_min();
			click_point.z = z_all[idx];
		  	if(!ctrl_pressed)
	        {
			  snake_sels->select( id_all[idx] );
			}
			else if(ctrl_pressed)
			{
			  snake_sels->toggle( id_all[idx] );
			}

		    //click_point.check_out_of_range_3D(SM,SN,SZ);
			//std::cout<<click_point.x<<","<<click_point.y<<","<<click_point.z<<std::endl;
			emit point_clicked(click_point);
		}
		else
		{
			snake_sels->clear();
		}
		/*if( SnakeList->IM->IL->GetPixel(index) != 0 )
		{
		    if(!modifiers.testFlag(Qt::ControlModifier))
	        {
			  snake_sels->select( SnakeList->IM->IL->GetPixel(index) - 1 );
			}
			else if(modifiers.testFlag(Qt::ControlModifier))
			{
			  snake_sels->toggle( SnakeList->IM->IL->GetPixel(index) - 1 );
			}
			click_point.z = i;
			click_point.check_out_of_range_3D(SM,SN,SZ);
			emit point_clicked(click_point);
			snake_found = true;
			return;
		}*/
	  //}
   }
}


void TracingView::mousePressEvent(QMouseEvent *event)
{  
   //if( montage_view )
	//   return;

   QWidget::mousePressEvent(event);
   QPoint pos = event->pos();
   Qt::KeyboardModifiers modifiers = event->modifiers();

   if( selMode == 1 )
   {
		selectionRegion.append(pos);
		this->repaint();
   }
   else if( event->button() == Qt::RightButton )
   { 
        click_point.x = pos.rx()/zoom;
        click_point.y = pos.ry()/zoom;
	    click_point.z = 0;
		path_points.AddPt(click_point);
		update();
		updateGeometry();
   }
   else if( event->button() == Qt::LeftButton && SnakeList->NSnakes != 0 )
   {
      //find the snake nearest to clicked point
	  //int SM = SnakeList->IM->I->GetLargestPossibleRegion().GetSize()[0];
	  //int SN = SnakeList->IM->I->GetLargestPossibleRegion().GetSize()[1];
      //int SZ = SnakeList->IM->I->GetLargestPossibleRegion().GetSize()[2];
	  bool snake_found = false;
      //for(int i = 0; i < SZ; i++ )
	  //{
		click_point.x = pos.rx()/zoom;
        click_point.y = pos.ry()/zoom;
	    click_point.z = 0;
		//click_point.check_out_of_range_3D(SM,SN,SZ);
		//index[0] = click_point.x;
		//index[1] = click_point.y;
		//index[2] = click_point.z;
		click_point.z = -1;
		emit point_clicked(click_point);
		update();
		updateGeometry();

		std::vector<float> dist_all;
		std::vector<int> id_all;
		std::vector<float> z_all;
		for( int j = 0; j < SnakeList->NSnakes; j++ )
		{
		  if( SnakeList->valid_list[j] == 0 )
		  {
		    dist_all.push_back(10000);
			id_all.push_back(j);
			z_all.push_back(0);
			continue;
		  }
		  for( int k = 0; k < SnakeList->Snakes[j].Cu.NP; k++ )
		  {
		    float dist_temp = SnakeList->Snakes[j].Cu.Pt[k].GetDistTo2D(click_point);
			dist_all.push_back(dist_temp);
			id_all.push_back(j);
			z_all.push_back(SnakeList->Snakes[j].Cu.Pt[k].z);
		  }
		}
	    
		vnl_vector<float> dist_all_vnl(dist_all.size());
		for(int j = 0; j < dist_all.size(); j++ )
		{
		  dist_all_vnl(j) = dist_all[j];
		}
		if( dist_all_vnl.min_value() < 10 )
		{
			int idx = dist_all_vnl.arg_min();
			click_point.z = z_all[idx];
		  	if(!modifiers.testFlag(Qt::ControlModifier))
	        {
			  snake_sels->select( id_all[idx] );
			}
			else if(modifiers.testFlag(Qt::ControlModifier))
			{
			  snake_sels->toggle( id_all[idx] );
			}

		    //click_point.check_out_of_range_3D(SM,SN,SZ);
			//std::cout<<click_point.x<<","<<click_point.y<<","<<click_point.z<<std::endl;
			emit point_clicked(click_point);
		}
		else
		{
		  snake_sels->clear();
		}
		/*if( SnakeList->IM->IL->GetPixel(index) != 0 )
		{
		    if(!modifiers.testFlag(Qt::ControlModifier))
	        {
			  snake_sels->select( SnakeList->IM->IL->GetPixel(index) - 1 );
			}
			else if(modifiers.testFlag(Qt::ControlModifier))
			{
			  snake_sels->toggle( SnakeList->IM->IL->GetPixel(index) - 1 );
			}
			click_point.z = i;
			click_point.check_out_of_range_3D(SM,SN,SZ);
			emit point_clicked(click_point);
			snake_found = true;
			return;
		}*/
	  //}
   }
}

void TracingView::mouseDoubleClickEvent(QMouseEvent *event)
{
   QWidget::mouseDoubleClickEvent(event);
   QPoint pos = event->pos();
	if(event->button() == Qt::LeftButton)
	{
	  click_point.x = pos.rx()/zoom;
      click_point.y = pos.ry()/zoom;
	  click_point.z = 0;
	  points.RemoveAllPts();
	  points.AddPt(click_point);
      emit manual_seed(points);
	  points.RemoveAllPts();
	}
	else if(event->button() == Qt::RightButton)
	{
	  click_point.x = pos.rx()/zoom;
      click_point.y = pos.ry()/zoom;
	  click_point.z = 0;
   
	 for( int j = 0; j < SnakeList->NSnakes; j++ )
     {
      if( SnakeList->valid_list[j] == 0 )
	    continue;
      if( snake_sels->isSelected(j) )
      {
		  points.RemoveAllPts();
		  float dist_head = click_point.GetDistTo(SnakeList->Snakes[j].Cu.GetLastPt());
		  float dist_tail = click_point.GetDistTo(SnakeList->Snakes[j].Cu.GetFirstPt());
		  if( dist_head < dist_tail)
		  {
		    points.AddPtList(SnakeList->Snakes[j].Cu);
			points.AddPt(click_point);
		  }
		  else
		  {
		    points.AddPt(click_point);
		    points.AddPtList(SnakeList->Snakes[j].Cu);
		  }

		  //SnakeList->valid_list[j] = 0;
		  SnakeList->RemoveSnake(j);
		  emit manual_seed(points);
          points.RemoveAllPts();

		  break;
      }
     }

	}
	setNormalCursor();
}

void TracingView::mouseMoveEvent(QMouseEvent *event)
{
	setNormalCursor();
    if (event->buttons() & Qt::LeftButton) 
	{
       QPoint pos = event->pos();
	   click_point.x = pos.rx()/zoom;
       click_point.y = pos.ry()/zoom;
	   click_point.z = 0;
	   points.AddPt(click_point);
	   update();
       updateGeometry();
    } 
}
void TracingView::mouseReleaseEvent(QMouseEvent *event)
{
   if( points.NP != 0 )
   {
	emit manual_seed(points);
    points.RemoveAllPts();
	setNormalCursor();
   }
}

void TracingView::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) 
	{
		case Qt::Key_Shift:
		 selMode = 1;
		 break;
		case Qt::Key_Control:
		 pathMode = 1;
		 break;
    }
	 //QWidget::keyPressEvent(event);
}

void TracingView::keyReleaseEvent(QKeyEvent * event)
{
    switch(event->key())
	{
		 case Qt::Key_Shift:
		  this->selectRegion();
		  selMode = 0;
	   	  break;
		 case Qt::Key_Control:
          if( path_points.NP > 0 )
		  {
		   emit manual_path(path_points);
		  }
          path_points.RemoveAllPts();
		  pathMode = 0;
		  update();
		  updateGeometry();
		  break;
	}
	//QWidget::keyReleaseEvent(event);
}

void TracingView::selectRegion()
{
	if (selMode != 1)
		return;

	if(selectionRegion.size() <= 0)
		return;

	seed_sels->clear(); 

	//Make the last point the first point to create a closed loop
	selectionRegion.append( selectionRegion[0] );
	int numPoints = selectionRegion.size();

	contour.RemoveAllPts();

	for( int i = 0; i < numPoints; i++ )
	{
		contour.AddPt(selectionRegion.at(i).x()/zoom, selectionRegion.at(i).y()/zoom,0);
	}

    std::set<long int> sels;
	for( int i = 0; i < IM->SeedPt.GetSize(); i++ )
	{
		if( IM->SeedPt.Pt[i].Inpolygon2D(contour) == 1 )
	   {
	     sels.insert( i );
	   }
	}
    selectionRegion.clear();
	seed_sels->add(sels);
    update();
    updateGeometry();

}

void TracingView::paintEvent(QPaintEvent *event)
{
   QPainter painter_main(this);  //painter for displaying in the GUI
	//display_image = image; 

  if( tree_set_swc )
  {
    QSize size = zoom * image.size();
    display_image = image.scaled(size, Qt::KeepAspectRatio);

    QPainter painter(&display_image); //painter for painting on the image
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
	drawSnakeTree_SWC(&painter);
	painter_main.drawImage(0,0,display_image);
	return;
  }

  if( !IM )
  {
    QSize size = zoom * image.size();
	display_image = image.scaled(size, Qt::KeepAspectRatio);
    painter_main.drawImage(0,0,display_image);
	return;
  }

  if( !tracing_snake )
  {
	 // if( SnakeList->NSnakes == 0)
	 // {
	 //    display_image = image;
	 // }

   QSize size = zoom * image.size();
   display_image = image.scaled(size, Qt::KeepAspectRatio);

   QPainter painter(&display_image); //painter for painting on the image
   painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	drawSelection(&painter);
    drawPath(&painter);

	if(points.NP !=0)
	 drawSeedSnake(&painter);

	if( IM->SeedPt.NP !=0 && seed_display )
	{
	  drawSeeds(&painter);
	  drawSelectedSeeds(&painter);
	}

	if( SnakeList->NSnakes != 0 && snake_display && !tree_set )
	{
	  drawSnakes(&painter);
	  drawSeedSnakes(&painter);
	  drawSelectedSnakes(&painter);
	  drawDeletedSnakes(&painter);
	  delete_sels->clear();
	}

	//draw snake tree
	if( tree_set )
	{
	  drawSnakeTree(&painter);
	}


	if( interest_point_set )
	{
	  drawInterestPoints(&painter);
	}

	drawClickedPoint(&painter);

	 painter_main.drawImage(0,0,display_image);
    if( SnakeList->NSnakes != 0 )
	 emit result_drawed();
  }
  else
  {
	 QSize size = zoom * image.size();
	 QImage new_image = display_image.scaled(size, Qt::KeepAspectRatio);
     QPainter painter(&new_image); //painter for painting on the image
	 painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	 //only re-draw selected snakes
     drawTracingSnakes(&painter);
     drawSelectedSnakes(&painter);
     //drawSelection(&painter);
	 drawDeletedSnakes(&painter);
	 delete_sels->clear();

	 drawClickedPoint(&painter);

	 painter_main.drawImage(0,0,new_image);

	 /*QString save_image_name;
	 save_image_name.setNum(iteration_num);
	 save_image_name.append(".tif");
	 iteration_num += 5;
	 new_image.save(save_image_name);*/

	 emit result_drawed1();
  }

 
	//QImage new_image;
	//QSize size = zoom * display_image.size();
	//new_image = display_image.scaled(size, Qt::KeepAspectRatio);
	
	//painter_main.
}

void TracingView::drawSeedSnake(QPainter *painter)
{
  int line_width = 3;
  //draw branches

   QPointF *SP= new QPointF[points.NP];
   for(int i = 0; i < points.NP; i++)
   {
     SP[i].setX(points.Pt[i].x * zoom);
	 SP[i].setY(points.Pt[i].y * zoom);
   }

   QPen pen0(Qt::blue, line_width);
   painter->setPen(pen0); 
   painter->drawPolyline(SP,points.NP);
   QPen pen1(Qt::white, 2);
   painter->setPen(pen1);
   painter->drawPoints(SP,points.NP);
}

void TracingView::drawClickedPoint(QPainter *painter)
{
	int x = click_point.x;
	int y = click_point.y;
	int z = click_point.z;
	int width = 2;

	QPointF *SP1= new QPointF[2];
    QPointF *SP2= new QPointF[2];

	if( x != 0 && y != 0 && z != 0 )
	{
     int diameter = 2;
     painter->setPen(QColor(255,0,0,255));
	 SP1[0].setX((x-width)*zoom);
	 SP1[0].setY(y*zoom);
	 SP1[1].setX((x+width)*zoom);
	 SP1[1].setY(y*zoom);
	 SP2[0].setX(x*zoom);
	 SP2[0].setY((y-width)*zoom);
	 SP2[1].setX(x*zoom);
	 SP2[1].setY((y+width)*zoom);
	 painter->drawPolyline(SP1,2);
     painter->drawPolyline(SP2,2);
	}
}

void TracingView::drawInterestPoints(QPainter *painter)
{
 int line_width = 4;
 int diameter = line_width * 3;

  for( int i = 0; i < interest_points.GetSize(); i++ )
  {
   painter->setPen(Qt::black);
   painter->setBrush(Qt::darkYellow);
   painter->drawEllipse(QRect(interest_points.Pt[i].x * zoom-diameter / 2.0, interest_points.Pt[i].y * zoom-diameter / 2.0,
            diameter, diameter));
  }
}

void TracingView::drawSnakeTree(QPainter *painter)
{

  int line_width = LineWidth;
  //draw branches
  for( unsigned int j = 0; j < snake_tree->snake_id.size(); j++ )
  {
   QPointF *SP= new QPointF[SnakeList->Snakes[snake_tree->snake_id[j]].Cu.GetSize()];
   for(int i = 0; i < SnakeList->Snakes[snake_tree->snake_id[j]].Cu.GetSize(); i++)
   {
     SP[i].setX(SnakeList->Snakes[snake_tree->snake_id[j]].Cu.GetPt(i).x * zoom);
	 SP[i].setY(SnakeList->Snakes[snake_tree->snake_id[j]].Cu.GetPt(i).y * zoom);
   }
 
   QPen pen0(Qt::blue, line_width);
   painter->setPen(pen0); 
   painter->drawPolyline(SP,SnakeList->Snakes[snake_tree->snake_id[j]].Cu.GetSize());

   QPen pen1(Qt::yellow, 2);
   painter->setPen(pen1);
   painter->drawPoints(SP,SnakeList->Snakes[snake_tree->snake_id[j]].Cu.GetSize());
  }

  
  int diameter = line_width * 2;
  //draw root point
  painter->setPen(Qt::black);
  painter->setBrush(Qt::yellow);
  painter->drawEllipse(QRect(snake_tree->root_point.x * zoom-diameter / 2.0, snake_tree->root_point.y * zoom-diameter / 2.0,
   	   diameter*2, diameter*2));

  //draw branch point 
  for( int i = 0; i < snake_tree->branch_point.GetSize(); i++ )
  {
   painter->setPen(Qt::black);
   painter->setBrush(Qt::yellow);
   painter->drawEllipse(QRect(snake_tree->branch_point.Pt[i].x * zoom-diameter / 2.0, snake_tree->branch_point.Pt[i].y * zoom-diameter / 2.0,
            diameter, diameter));
  }

}

void TracingView::drawSnakeTree_SWC(QPainter *painter)
{
 int line_width = LineWidth;
  //draw branches
  for( int i = 0; i < snake_tree_swc->Snakes.NSnakes; i++)
  {

	 QPointF *SP= new QPointF[snake_tree_swc->Snakes.Snakes[i].Cu.GetSize()];

	for( int j = 0; j < snake_tree_swc->Snakes.Snakes[i].Cu.NP; j++ )
    {
     SP[j].setX(snake_tree_swc->Snakes.Snakes[i].Cu.Pt[j].x * zoom);
	 SP[j].setY(snake_tree_swc->Snakes.Snakes[i].Cu.Pt[j].y * zoom);
	}
 
     QPen pen0(Qt::blue, line_width);
     painter->setPen(pen0); 
     painter->drawPolyline(SP,snake_tree_swc->Snakes.Snakes[i].Cu.NP);

     QPen pen1(Qt::yellow, 2);
     painter->setPen(pen1);
     painter->drawPoints(SP,snake_tree_swc->Snakes.Snakes[i].Cu.NP);
  }

  int diameter = line_width * 2;

  float soma_radius = snake_tree_swc->soma_radius; 
  if( diameter * 2 > soma_radius )
	  soma_radius = diameter * 2;

  soma_radius = soma_radius * zoom;

  //draw root point
  painter->setPen(Qt::black);
  painter->setBrush(Qt::red);
  painter->setOpacity(0.6);
  painter->drawEllipse(QRect(snake_tree_swc->RootPt.x * zoom-soma_radius, snake_tree_swc->RootPt.y * zoom-soma_radius,
   	  soma_radius*2, soma_radius*2));

  painter->setOpacity(1);

  //draw branch point 
  for( int i = 0; i < snake_tree_swc->BranchPt.NP; i++ )
  {
   painter->setPen(Qt::black);
   painter->setBrush(Qt::yellow);
   painter->drawEllipse(QRect(snake_tree_swc->BranchPt.Pt[i].x * zoom-diameter / 2.0, snake_tree_swc->BranchPt.Pt[i].y * zoom-diameter / 2.0,
            diameter, diameter));
  }
}

void TracingView::drawSelection(QPainter *painter)
{
	if (selMode == 1)
	{
		QPen pen(Qt::yellow, 2);
		painter->setPen(pen);
		for (int i=1; i < selectionRegion.size(); i++)
		{
			painter->drawLine(selectionRegion[i-1],selectionRegion[i]);
		}
	}
}

void TracingView::drawPath(QPainter *painter)
{
    if(pathMode==1)
	{
		QPen pen(Qt::yellow, 2);
		painter->setPen(pen);
		for (int i=1; i < path_points.NP; i++)
		{
			painter->drawLine(path_points.Pt[i-1].x * zoom, path_points.Pt[i-1].y * zoom, path_points.Pt[i].x * zoom, path_points.Pt[i].y * zoom);
		} 
	}
}

void TracingView::drawDeletedSnakes(QPainter *painter)
{
  int line_width = 3;
  for( int j = 0; j < SnakeList->NSnakes; j++ )
  {
   if( delete_sels->isSelected(j) )
   {
    QPointF *SP= new QPointF[SnakeList->Snakes[j].Cu.GetSize()];
    for(int i = 0; i < SnakeList->Snakes[j].Cu.GetSize(); i++)
    {
     SP[i].setX(SnakeList->Snakes[j].Cu.GetPt(i).x * zoom);
	 SP[i].setY(SnakeList->Snakes[j].Cu.GetPt(i).y * zoom);
    }
 
    QPen pen0(Qt::black, line_width);
    painter->setPen(pen0); 
    painter->drawPolyline(SP,SnakeList->Snakes[j].Cu.GetSize());
   }
  }
}

void TracingView::deselect()
{
	 snake_sels->clear();
}

vnl_vector<int> TracingView:: getSelectedSnakes()
{
  vnl_vector<int> snake_id(SnakeList->NSnakes);
  snake_id.fill(0);

  for( int j = 0; j < SnakeList->NSnakes; j++ )
  {
   if( SnakeList->valid_list[j] == 0 )
	 continue;
   if( snake_sels->isSelected(j) )
   {
	  snake_id(j) = 1;
   }
  }

  return snake_id;
}
void TracingView::drawSelectedSnakes(QPainter *painter)
{
  int line_width = 3;

  for( int j = 0; j < SnakeList->NSnakes; j++ )
  {
   if( SnakeList->valid_list[j] == 0 )
	 continue;
   if( !snake_sels->isSelected(j) )
	 continue;

   QPointF *SP= new QPointF[SnakeList->Snakes[j].Cu.GetSize()];
   for(int i = 0; i < SnakeList->Snakes[j].Cu.GetSize(); i++)
   {
     SP[i].setX(SnakeList->Snakes[j].Cu.GetPt(i).x * zoom);
	 SP[i].setY(SnakeList->Snakes[j].Cu.GetPt(i).y * zoom);
   }
 
   QPen pen0(Qt::blue, line_width);
   painter->setPen(pen0); 
   painter->drawPolyline(SP,SnakeList->Snakes[j].Cu.GetSize());

   //when selected, also show the snake points
   QPen pen1(Qt::white, 2);
   painter->setPen(pen1);
   painter->drawPoints(SP,SnakeList->Snakes[j].Cu.GetSize());

  }
}

void TracingView::drawSnakes(QPainter *painter)
{ 
  int line_width = LineWidth;

  for( int j = 0; j < SnakeList->NSnakes; j++ )
  {
   if( SnakeList->valid_list[j] == 0 )
     continue;

   if( snake_sels->isSelected(j) )
	 continue;

   //centerline
   QPointF *SP= new QPointF[SnakeList->Snakes[j].Cu.GetSize()];
   //boundary
   QPointF *LB = new QPointF[SnakeList->Snakes[j].Cu.GetSize()];
   QPointF *RB = new QPointF[SnakeList->Snakes[j].Cu.GetSize()];

   for(int i = 0; i < SnakeList->Snakes[j].Cu.GetSize(); i++)
   {
     SP[i].setX(SnakeList->Snakes[j].Cu.GetPt(i).x * zoom);
	 SP[i].setY(SnakeList->Snakes[j].Cu.GetPt(i).y * zoom);
	 if( i == 0 )
	 {
	  float v2x = SnakeList->Snakes[j].Cu.GetPt(1).y - SnakeList->Snakes[j].Cu.GetPt(2).y;
	  float v2y = SnakeList->Snakes[j].Cu.GetPt(2).x - SnakeList->Snakes[j].Cu.GetPt(1).x;
	  float mag = sqrt(pow(v2x,2)+pow(v2y,2));
	  v2x /= (mag + std::numeric_limits<double>::epsilon());
	  v2y /= (mag + std::numeric_limits<double>::epsilon());
	  LB[i].setX( (SnakeList->Snakes[j].Cu.GetPt(i).x + SnakeList->Snakes[j].Ru[i] * v2x) * zoom );
	  LB[i].setY( (SnakeList->Snakes[j].Cu.GetPt(i).y + SnakeList->Snakes[j].Ru[i] * v2y) * zoom );
	  RB[i].setX( (SnakeList->Snakes[j].Cu.GetPt(i).x - SnakeList->Snakes[j].Ru[i] * v2x) * zoom );
	  RB[i].setY( (SnakeList->Snakes[j].Cu.GetPt(i).y - SnakeList->Snakes[j].Ru[i] * v2y) * zoom );
	 }
	 else
	 {
	  float v2x = SnakeList->Snakes[j].Cu.GetPt(i-1).y - SnakeList->Snakes[j].Cu.GetPt(i).y;
	  float v2y = SnakeList->Snakes[j].Cu.GetPt(i).x - SnakeList->Snakes[j].Cu.GetPt(i-1).x;
	  float mag = sqrt(pow(v2x,2)+pow(v2y,2));
	  v2x /= (mag + std::numeric_limits<double>::epsilon());
	  v2y /= (mag + std::numeric_limits<double>::epsilon());
      LB[i].setX( (SnakeList->Snakes[j].Cu.GetPt(i).x + SnakeList->Snakes[j].Ru[i] * v2x) * zoom );
	  LB[i].setY( (SnakeList->Snakes[j].Cu.GetPt(i).y + SnakeList->Snakes[j].Ru[i] * v2y) * zoom );
	  RB[i].setX( (SnakeList->Snakes[j].Cu.GetPt(i).x - SnakeList->Snakes[j].Ru[i] * v2x) * zoom );
	  RB[i].setY( (SnakeList->Snakes[j].Cu.GetPt(i).y - SnakeList->Snakes[j].Ru[i] * v2y) * zoom );
	 }

	 if( radius_state == 1 || radius_state == 3)
	 {
       QPen pen(Qt::red, 1);
       painter->setPen(pen); 
	   painter->drawEllipse(QRect(SnakeList->Snakes[j].Cu.GetPt(i).x * zoom - SnakeList->Snakes[j].Ru[i] * zoom, SnakeList->Snakes[j].Cu.GetPt(i).y * zoom - SnakeList->Snakes[j].Ru[i] * zoom,
   	   2 * SnakeList->Snakes[j].Ru[i] * zoom, 2 * SnakeList->Snakes[j].Ru[i] * zoom));
	 }

   }
  
  if( color_line_display )
  {
   /*if( j%10 == 0)
   {
    QPen pen0(Qt::red, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 1)
   {
    QPen pen0(Qt::darkRed, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 2)
   {
    QPen pen0(Qt::green, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 3)
   {
    QPen pen0(Qt::darkGreen, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 4)
   {
    QPen pen0(Qt::cyan, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 5)
   {
    QPen pen0(Qt::darkCyan, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 6)
   {
    QPen pen0(Qt::magenta, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 7)
   {
    QPen pen0(Qt::darkMagenta, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 8)
   {
    QPen pen0(Qt::yellow, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 9)
   {
    QPen pen0(Qt::darkYellow, line_width);
    painter->setPen(pen0); 
   }*/
   QPen pen0(QColor(rand()%255,rand()%255,rand()%255), line_width);
   painter->setPen(pen0);
  }
  else
  {
   //if( SnakeList->valid_list[j] != 0 )
   //{
     QPen pen0(Qt::green, line_width);
	 painter->setPen(pen0); 
   //}
   //else
   //{
   // QPen pen0(Qt::black, line_width);
   // painter->setPen(pen0);
   //}
  }

   //painter->setPen(pen0);
   painter->drawPolyline(SP,SnakeList->Snakes[j].Cu.GetSize());

   if( radius_state == 2 || radius_state == 3)
   {
     QPen pen1(Qt::blue, 2);
     painter->setPen(pen1);
	 painter->drawPolyline(SP,SnakeList->Snakes[j].Cu.GetSize());
	 QPen pen2(Qt::red, 2);
	 painter->setPen(pen2);
	 painter->drawPoints(SP,SnakeList->Snakes[j].Cu.GetSize());
     painter->drawPolyline(LB,SnakeList->Snakes[j].Cu.GetSize());
     painter->drawPolyline(RB,SnakeList->Snakes[j].Cu.GetSize());
   }

   //QPen pen0(Qt::yellow, 1);
   //painter->setPen(pen0);
   //painter->drawPoints(SP,SnakeList->Snakes[j].Cu.GetSize());

   //draw head
   //painter->setPen(Qt::blue);
   int diameter = line_width * 1.5;
   //painter->fillRect(QRect(SnakeList->Snakes[j].Cu.GetLastPt().x * zoom-diameter / 2.0, SnakeList->Snakes[j].Cu.GetLastPt().y * zoom-diameter / 2.0,
   //                                       diameter, diameter),Qt::blue);
   QPen pen_head(Qt::darkBlue, diameter);
   painter->setPen(pen_head);
   painter->drawPoint(SnakeList->Snakes[j].Cu.GetLastPt().x * zoom,SnakeList->Snakes[j].Cu.GetLastPt().y * zoom);


   //draw tail
   painter->setPen(Qt::darkBlue);
   painter->drawEllipse(QRect(SnakeList->Snakes[j].Cu.GetFirstPt().x * zoom-diameter / 2.0, SnakeList->Snakes[j].Cu.GetFirstPt().y * zoom-diameter / 2.0,
   	   diameter, diameter));

   //draw branch point 
    painter->setPen(Qt::yellow);
   /*for( int i = 0; i < SnakeList->Snakes[j].BranchPt.GetSize(); i++ )
   {
   
	painter->fillRect(QRect(SnakeList->Snakes[j].BranchPt.Pt[i].x * zoom-diameter / 2.0, SnakeList->Snakes[j].BranchPt.Pt[i].y * zoom-diameter / 2.0,
                                           diameter, diameter),Qt::yellow);
   }*/
  }

   int diameter = line_width * 1.5;
   painter->setPen(Qt::yellow);
   //std::cout<<"SnakeList->branch_points.NP:"<<SnakeList->branch_points.NP<<std::endl;
   for( int i = 0; i < SnakeList->branch_points.NP; i++ )
   {
     painter->fillRect(QRect(SnakeList->branch_points.Pt[i].x * zoom-diameter / 2.0, SnakeList->branch_points.Pt[i].y * zoom-diameter / 2.0,
                                           diameter, diameter),Qt::yellow);
   }
}

void TracingView::drawSeedSnakes(QPainter *painter)
{ 
  int line_width = 2;

  for( int j = 0; j < SeedSnakes->NSnakes; j++ )
  {
   QPointF *SP= new QPointF[SeedSnakes->Snakes[j].Cu.GetSize()];
   for(int i = 0; i < SeedSnakes->Snakes[j].Cu.GetSize(); i++)
   {
     SP[i].setX(SeedSnakes->Snakes[j].Cu.GetPt(i).x * zoom);
	 SP[i].setY(SeedSnakes->Snakes[j].Cu.GetPt(i).y * zoom);
   }
  

   if( j%10 == 0)
   {
    QPen pen0(Qt::red, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 1)
   {
    QPen pen0(Qt::darkRed, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 2)
   {
    QPen pen0(Qt::green, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 3)
   {
    QPen pen0(Qt::darkGreen, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 4)
   {
    QPen pen0(Qt::cyan, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 5)
   {
    QPen pen0(Qt::darkCyan, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 6)
   {
    QPen pen0(Qt::magenta, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 7)
   {
    QPen pen0(Qt::darkMagenta, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 8)
   {
    QPen pen0(Qt::yellow, line_width);
    painter->setPen(pen0); 
   }
   if( j%10 == 9)
   {
    QPen pen0(Qt::darkYellow, line_width);
    painter->setPen(pen0); 
   }

   //painter->setPen(pen0);
   painter->drawPolyline(SP,SeedSnakes->Snakes[j].Cu.GetSize());
  }
}

void TracingView::drawTracingSnakes(QPainter *painter)
{
   int line_width = 3;

      //centerline
   QPointF *SP= new QPointF[snake.Cu.GetSize()];
   //boundary
   QPointF *LB = new QPointF[snake.Cu.GetSize()];
   QPointF *RB = new QPointF[snake.Cu.GetSize()];

   for(int i = 0; i < snake.Cu.GetSize(); i++)
   {
     SP[i].setX(snake.Cu.GetPt(i).x * zoom);
	 SP[i].setY(snake.Cu.GetPt(i).y * zoom);
    if( radius_state == 2 || radius_state == 3 )
	{
	 if( i == 0 )
	 {
	  float v2x = snake.Cu.GetPt(1).y - snake.Cu.GetPt(2).y;
	  float v2y = snake.Cu.GetPt(2).x - snake.Cu.GetPt(1).x;
	  float mag = sqrt(pow(v2x,2)+pow(v2y,2));
	  v2x /= (mag + std::numeric_limits<double>::epsilon());
	  v2y /= (mag + std::numeric_limits<double>::epsilon());
	  LB[i].setX( (snake.Cu.GetPt(i).x + snake.Ru[i] * v2x) * zoom );
	  LB[i].setY( (snake.Cu.GetPt(i).y + snake.Ru[i] * v2y) * zoom );
	  RB[i].setX( (snake.Cu.GetPt(i).x - snake.Ru[i] * v2x) * zoom );
	  RB[i].setY( (snake.Cu.GetPt(i).y - snake.Ru[i] * v2y) * zoom );
	 }
	 else
	 {
	  float v2x = snake.Cu.GetPt(i-1).y - snake.Cu.GetPt(i).y;
	  float v2y = snake.Cu.GetPt(i).x - snake.Cu.GetPt(i-1).x;
	  float mag = sqrt(pow(v2x,2)+pow(v2y,2));
	  v2x /= (mag + std::numeric_limits<double>::epsilon());
	  v2y /= (mag + std::numeric_limits<double>::epsilon());
      LB[i].setX( (snake.Cu.GetPt(i).x + snake.Ru[i] * v2x) * zoom );
	  LB[i].setY( (snake.Cu.GetPt(i).y + snake.Ru[i] * v2y) * zoom );
	  RB[i].setX( (snake.Cu.GetPt(i).x - snake.Ru[i] * v2x) * zoom );
	  RB[i].setY( (snake.Cu.GetPt(i).y - snake.Ru[i] * v2y) * zoom );
	 } 
	}
   }

   /*QPen pen0(Qt::blue, 2); 
   painter->setPen(pen0);
   painter->drawPolyline(SP,snake.Cu.GetSize());
   QPen pen1(Qt::white, 2); 
   painter->setPen(pen1);
   painter->drawPoints(SP,snake.Cu.GetSize());
   QPen pen2(Qt::red, 2); 
   painter->setPen(pen2);
   painter->drawPolyline(LB,snake.Cu.GetSize());
   painter->drawPolyline(RB,snake.Cu.GetSize());*/

   if( radius_state == 2 || radius_state == 3 )
   {
     QPen pen1(Qt::blue, 2);
     painter->setPen(pen1);
	 painter->drawPolyline(SP,snake.Cu.GetSize());
	 QPen pen2(Qt::red, 2);
	 painter->setPen(pen2);
	 painter->drawPoints(SP,snake.Cu.GetSize());
     painter->drawPolyline(LB,snake.Cu.GetSize());
     painter->drawPolyline(RB,snake.Cu.GetSize());
   }
   else
   {
     QPen pen0(Qt::blue, 2); 
     painter->setPen(pen0);
     painter->drawPolyline(SP,snake.Cu.GetSize());
   }


   //draw head
   painter->setPen(Qt::darkRed);
   int diameter = line_width * 2;
   painter->drawEllipse(QRect(snake.Cu.GetLastPt().x * zoom-diameter / 2.0, snake.Cu.GetLastPt().y * zoom-diameter / 2.0,
                                           diameter, diameter));
   //draw tail
   painter->setPen(Qt::darkRed);
   painter->drawRect(snake.Cu.GetFirstPt().x * zoom-diameter / 2.0, snake.Cu.GetFirstPt().y * zoom-diameter / 2.0,
                                           diameter, diameter);


   /* typedef itk::VectorLinearInterpolateImageFunction< 
                       GradientImageType, float >  GradientInterpolatorType;

   GradientInterpolatorType::Pointer interpolator = GradientInterpolatorType::New();
   interpolator->SetInputImage(IM->V1);
   GradientImageType::IndexType index; 
   index[0] = (snake.Cu.GetFirstPt().x);
   index[1] = (snake.Cu.GetFirstPt().y);
   index[2] = (snake.Cu.GetFirstPt().z);
   GradientPixelType gradient = interpolator->EvaluateAtIndex(index);
   QPen pen1(Qt::blue, 5);
   painter->setPen(pen1);
   painter->drawPoint((snake.Cu.GetFirstPt().x - gradient[0] * 5) * zoom, (snake.Cu.GetFirstPt().y - gradient[1] * 5) * zoom);
   painter->drawPoint((snake.Cu.GetFirstPt().x - gradient[0] * 5) * zoom, (snake.Cu.GetFirstPt().y - gradient[1] * 5) * zoom);
   index[0] = (snake.Cu.GetLastPt().x);
   index[1] = (snake.Cu.GetLastPt().y);
   index[2] = (snake.Cu.GetLastPt().z);
   gradient = interpolator->EvaluateAtIndex(index);
   painter->setPen(pen1);
   painter->drawPoint((snake.Cu.GetLastPt().x + gradient[0] * 5) * zoom, (snake.Cu.GetLastPt().y + gradient[1] * 5) * zoom);
   painter->drawPoint((snake.Cu.GetLastPt().x - gradient[0] * 5) * zoom, (snake.Cu.GetLastPt().y - gradient[1] * 5) * zoom);

   interpolator->SetInputImage(IM->IGVF);
   index[0] = (snake.Cu.GetFirstPt().x);
   index[1] = (snake.Cu.GetFirstPt().y);
   index[2] = (snake.Cu.GetFirstPt().z);
   gradient = interpolator->EvaluateAtIndex(index);
   QPen pen2(Qt::white, 5);
   painter->setPen(pen2);
   painter->drawPoint((snake.Cu.GetFirstPt().x - gradient[0] * 5) * zoom, (snake.Cu.GetFirstPt().y - gradient[1] * 5) * zoom);
   painter->drawPoint((snake.Cu.GetFirstPt().x - gradient[0] * 5) * zoom, (snake.Cu.GetFirstPt().y - gradient[1] * 5) * zoom);
   index[0] = (snake.Cu.GetLastPt().x);
   index[1] = (snake.Cu.GetLastPt().y);
   index[2] = (snake.Cu.GetLastPt().z);
   gradient = interpolator->EvaluateAtIndex(index);
   painter->setPen(pen2);
   painter->drawPoint((snake.Cu.GetLastPt().x + gradient[0] * 5) * zoom, (snake.Cu.GetLastPt().y + gradient[1] * 5) * zoom);
   painter->drawPoint((snake.Cu.GetLastPt().x - gradient[0] * 5) * zoom, (snake.Cu.GetLastPt().y - gradient[1] * 5) * zoom); */

   //draw head probe points
   /*snake.GetProbe(5,30,3,2);
   QPointF *Probe = new QPointF[snake.Probe.GetSize()];
     for(int i = 0; i < snake.Probe.GetSize(); i++)
     {
       Probe[i].setX(snake.Probe.GetPt(i).x * zoom);
	   Probe[i].setY(snake.Probe.GetPt(i).y * zoom);
     }
   QPen pen1(Qt::blue,3);
   painter->setPen(pen1);
   painter->drawPoints(Probe, snake.Probe.GetSize()); */

}

void TracingView::drawSelectedSeeds(QPainter *painter)
{
	QPointF *SP1= new QPointF[IM->SeedPt.GetSize()];
   for(int i = 0; i < IM->SeedPt.GetSize(); i++)
   {
	 if( IM->visit_label(i) == 0 && seed_sels->isSelected(i) == 1)
	 {
	   SP1[i].setX(IM->SeedPt.GetPt(i).x * zoom);
	   SP1[i].setY(IM->SeedPt.GetPt(i).y * zoom);
	 }
	 else
	 {
       SP1[i].setX(-1);
	   SP1[i].setY(-1);
	 }
   }

   QPen pen1(Qt::yellow, 3); 
   painter->setPen(pen1);
   painter->drawPoints(SP1,IM->SeedPt.GetSize());
}

void TracingView::drawSeeds(QPainter *painter)
{
   QPointF *SP1= new QPointF[IM->SeedPt.GetSize()];
   for(int i = 0; i < IM->SeedPt.GetSize(); i++)
   {
	 if( IM->visit_label(i) == 0 && seed_sels->isSelected(i) == 0)
	 {
	   SP1[i].setX(IM->SeedPt.GetPt(i).x * zoom);
	   SP1[i].setY(IM->SeedPt.GetPt(i).y * zoom);
	 }
	 else
	 {
       SP1[i].setX(-1);
	   SP1[i].setY(-1);
	 }
   }

   QPen pen1(Qt::red, 3); 
   painter->setPen(pen1);
   painter->drawPoints(SP1,IM->SeedPt.GetSize());

   QString text("#Seeds:");
   QString num;
   num.setNum(IM->SeedPt.GetSize());
   text.append(num);
   QFont fp = painter->font();
   fp.setBold(true);
   painter->setFont(fp);
   painter->setPen(Qt::red);
   //painter->setCompositionMode(QPainter::CompositionMode_Overlay);
   painter->drawText(10,10, text);
    
   //int diameter = 3;

   //for(int i = 0; i < Seeds.GetSize(); i++)
   //{
   // painter->drawEllipse(QRect(Seeds.GetPt(i).x-diameter / 2.0, Seeds.GetPt(i).y-diameter / 2.0,
   //                                        diameter, diameter));
   //}
}


void TracingView::setImagePixel(const QPoint &pos, bool opaque)
{
    /*int i = pos.x() / zoom;
    int j = pos.y() / zoom;
    if (image.rect().contains(i, j)) {
        if (opaque) {
            image.setPixel(i, j, penColor().rgba());
        } else {
            image.setPixel(i, j, qRgba(0, 0, 0, 0));
        }

        update(pixelRect(i, j));
    }*/
}

QRect TracingView::pixelRect(int i, int j) const
{
    if (zoom >= 3) {
        return QRect(zoom * i + 1, zoom * j + 1, zoom - 1, zoom - 1);
    } else {
        return QRect(zoom * i, zoom * j, zoom, zoom);
    }
}
