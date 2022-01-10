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

#include "TracingGUI.h"
#include "ftkUtils.h"
#include "itkTimeProbe.h"

QtTracer::QtTracer(QWidget * parent, Qt::WindowFlags flags) : QMainWindow(parent, flags)
{  
	IM = new ImageOperation;
	Tracer = new OpenSnakeTracer;

	Process_Stage = 0;
	Old_Process_Stage = 0;
	Preprocess_Stage = 0;

	current_idx = -1; //current index of image in the batch processing I
	current_lvl = 1;

	thread_finished = 0;

	original_seed_num = 0;

	Z_shift = 0;

	nth_frame = 0;

	seeding = false;
	tracing_started = false;
	mg_process = false;
	current_mg_cell = 0;
	automated_process = false;
	batch_processI = false;
	batch_processII = false;
	batch_preprocessing = false;
	outside_preprocess = false;
	montage_created = false;
	bpII_finished = false;
	using_stacked_image = false;
	soma_seeding = false;
	show_table_scatter = false;

	snake_tree = NULL;
	snake_tree_swc = NULL;
	string_array = NULL;

	LineWidth = 3;

	//edits
	edits = new EditValidation;
	edits->TP = 0.0;
	edits->FP = 0.0;
	edits->FN = 0.0;
	edits->precision = 0.0;
	edits->recall = 0.0;
	edits->numBranch = 0;
	edits->numSplit = 0;
	edits->numDelete = 0;
	edits->numMerge = 0;

	//wrote_pt.SetN(10000);
	//All_Pt.SetN(100000);

	createMainGUI();
	createActions();
	createMenus();
	createToolBar();
	createViews();

	
	initializeRendering();

	setWindowTitle("Open-Snake Tracing System");
	setWindowIcon(QIcon(":/icons/banner.bmp"));
	setGeometry(QRect(200,200,400,400));

	//setEliteSetting();
	setAcceptDrops(true);
}

void QtTracer::dropEvent(QDropEvent *event)
{
     QByteArray data  = event->mimeData()->data("FileNameW");
     QString filename = QString::fromUtf16((ushort*)data.data(), data.size() / 2);
	 //std::cout<<"filename:"<<filename.toStdString()<<std::endl;
	 
	 file = filename;
	 file_raw = filename;

   if( !file.isEmpty() )
	 loadImage_DragNDrop();

     event->acceptProposedAction();
}

void QtTracer::mouseMoveEvent(QMouseEvent *event)
{
     if (!(event->buttons() & Qt::RightButton))
         return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    drag->setMimeData(mimeData);
    Qt::DropAction dropAction = drag->exec();
}

void QtTracer::dragEnterEvent(QDragEnterEvent *event)
{
      event->acceptProposedAction();
}

void QtTracer::createMainGUI()
{
	tracingViewer = new TracingView;
	slider = new SlidersGroup(false);
	slider->setCheckable(0);

	scrollArea = new QScrollArea;
	scrollArea->setBackgroundRole(QPalette::Dark);
	scrollArea->setWidget(tracingViewer); 
	scrollArea->setWidgetResizable(true);
	//scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
	//scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

	imageLabel = new QLabel;
	imageLabel->setBackgroundRole(QPalette::Base);
	imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	imageLabel->setScaledContents(true);

	crossSectionLabel1 = new QLabel;
	crossSectionLabel1->setBackgroundRole(QPalette::Base);
	crossSectionLabel1->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	crossSectionLabel1->setScaledContents(true);

	crossSectionLabel2 = new QLabel;
	crossSectionLabel2->setBackgroundRole(QPalette::Base);
	crossSectionLabel2->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	crossSectionLabel2->setScaledContents(true);

	grid_layout = new QGridLayout;
	grid_layout->addWidget(imageLabel,0,0,2,2);
	grid_layout->addWidget(crossSectionLabel1,0,2,2,1);
	grid_layout->addWidget(crossSectionLabel2,2,0,1,2);

	scrollArea_BW = new QScrollArea;
	scrollArea_BW->setBackgroundRole(QPalette::Dark);
	scrollArea_BW->setLayout(grid_layout); 
	scrollArea_BW->setWidgetResizable(true);

	montageLabel = new QLabel;
	montageLabel->setBackgroundRole(QPalette::Base);
	montageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	montageLabel->setScaledContents(true);

	scrollArea_MT = new QScrollArea;
	scrollArea_MT->setBackgroundRole(QPalette::Dark);
	scrollArea_MT->setWidget(montageLabel); 
	scrollArea_MT->setWidgetResizable(true);

	this->QVTK = new QVTKWidget();

	imageTab = new QTabWidget;
	imageTab->addTab(scrollArea, "Tracing View");
	imageTab->addTab(QVTK,"3D View");
	imageTab->addTab(scrollArea_BW, "Cross-Section View");
	imageTab->addTab(scrollArea_MT, "Montage View");

	int index_tab = imageTab->indexOf(scrollArea_BW);
	int index_tab1 = imageTab->indexOf(scrollArea_MT);
	imageTab->setTabEnabled(index_tab,true);
	imageTab->setTabEnabled(index_tab1,false);

	opacity = new QLabel("Opacity");
	opacity_slider = new QSlider(Qt::Vertical);
	opacity_slider->setSingleStep(1);
	opacity_slider->setMinimum(0);
	opacity_slider->setMaximum(100);
	connect(opacity_slider, SIGNAL(valueChanged(int)), this, SLOT(changeOpacity(int)));

	opacity_th = new QLabel("th");
	opacity_th_slider = new QSlider(Qt::Vertical);
	opacity_th_slider->setSingleStep(1);
	opacity_th_slider->setMinimum(0);
	opacity_th_slider->setMaximum(255);
	connect(opacity_th_slider, SIGNAL(valueChanged(int)), this, SLOT(changeOpacityTh(int)));

	brightness = new QLabel("Brightness");
	brightness_slider = new QSlider(Qt::Vertical);
	brightness_slider->setSingleStep(1);
	brightness_slider->setMinimum(0);
	brightness_slider->setMaximum(255);
	connect(brightness_slider, SIGNAL(valueChanged(int)), this, SLOT(changeBrightness(int)));


	line_width = new QLabel("Width");
	line_width_slider = new QSlider(Qt::Vertical);
	line_width_slider->setSingleStep(1);
	line_width_slider->setMinimum(1);
	line_width_slider->setMaximum(10);
	connect(line_width_slider, SIGNAL(valueChanged(int)), this, SLOT(changeLineWidth(int)));
	connect(line_width_slider, SIGNAL(valueChanged(int)), tracingViewer, SLOT(changeLineWidth(int)));

	layout2 = new QVBoxLayout;
	layout3 = new QHBoxLayout;

	QLayout *layout_slider = new QVBoxLayout;
	layout_slider->addWidget(opacity);
	layout_slider->addWidget(opacity_slider);
	layout_slider->addWidget(opacity_th);
	layout_slider->addWidget(opacity_th_slider);
	layout_slider->addWidget(brightness);
	layout_slider->addWidget(brightness_slider);
	layout_slider->addWidget(line_width);
	layout_slider->addWidget(line_width_slider);
	opacity->setVisible(false);
	opacity_slider->setVisible(false);
	opacity_th->setVisible(false);
	opacity_th_slider->setVisible(false);
	brightness->setVisible(false);
	brightness_slider->setVisible(false);
	//line_width->setVisible(false);
	//line_width_slider->setVisible(false);

	layout3->addLayout(layout_slider);
	layout3->addWidget(imageTab);

	layout2->addLayout(layout3);
	layout2->addWidget(slider);


	left_widget = new QWidget;
	left_widget->setLayout(layout2);

	general_para = new General_Parameters(tr("Processing Methods"));

	general_para12 = new General_Parameters12(tr("Preprocessing"));

	general_para12->setVisible(false);
	general_para2 = new General_Parameters2(tr("Tracing"));
	general_para2->setVisible(false);
	general_para3 = new General_Parameters3(tr("Output"));
	general_para3->setVisible(false);

	main_splitter = new QSplitter(Qt::Horizontal);
	main_splitter->addWidget(left_widget);

	dockWidget = new QDockWidget(tr("Trace Settings"));

	dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea |
		Qt::RightDockWidgetArea);

	layout1 = new QVBoxLayout;

	layout_paras = new QHBoxLayout;

	layout_paras->addWidget(general_para);
	//layout1->addWidget(general_para1);
	layout_paras->addWidget(general_para12);
	layout_paras->addWidget(general_para2);
	layout_paras->addWidget(general_para3);

	layout1->addLayout(layout_paras);

	backButton = new QPushButton(QIcon(":/icons/Back.png"), "Back");
	backButton->setEnabled(false);
	nextButton = new QPushButton(QIcon(":/icons/Forward.png"), "Next");
	nextButton->setEnabled(false);
	startButtonI = new QPushButton(QIcon(":/icons/Start.png"), "Start Batch Tracing I");
	startButtonI->setVisible(false);
	startButtonII = new QPushButton(QIcon(":/icons/Start.png"), "Start Batch Tracing II");
	startButtonII->setVisible(false);
	layout1->addWidget(backButton);
	layout1->addWidget(nextButton);
	layout1->addWidget(startButtonI);
	layout1->addWidget(startButtonII);

	right_widget = new QWidget;
	right_widget->setLayout(layout1);

	dockWidget->setWidget(right_widget);
	addDockWidget(Qt::RightDockWidgetArea, dockWidget);

	set_root_dialog = new setRootDialog;
	set_root_dialog->setWindowTitle("Set Root");
	connect(set_root_dialog,SIGNAL(root_set()), this, SLOT(convertSnakeTree()));

	set_transform_dialog = new setTransformDialog;
	set_transform_dialog->setWindowTitle("Set Root Tile and Image Width and Height");
	connect(set_transform_dialog,SIGNAL(root_set()), this, SLOT(generateGraph()));

	connect(general_para->stackImageButton,SIGNAL(clicked()), this, SLOT(stackImage()));
	connect(general_para->loadButton,SIGNAL(clicked()), this, SLOT(loadImage()));
	connect(general_para->loadDisplayButton,SIGNAL(clicked()), this, SLOT(loadDisplayImage()));
	connect(general_para->reloadButton,SIGNAL(clicked()), this, SLOT(reloadImage()));

	connect(general_para->autoButton,SIGNAL(clicked()), this, SLOT(autoProcessing()));
	connect(general_para->batchButtonI,SIGNAL(clicked()), this, SLOT(batchProcessingI()));
	connect(general_para->batchButtonII,SIGNAL(clicked()), this, SLOT(batchProcessingII()));
	connect(general_para,SIGNAL(auto_Selected(bool)), this, SLOT(auto_mode_slot(bool)));
	connect(general_para,SIGNAL(batchI_Selected(bool)), this, SLOT(batchI_mode_slot(bool)));
	connect(general_para,SIGNAL(batchII_Selected(bool)), this, SLOT(batchII_mode_slot(bool)));

	connect(general_para12->preprocessButton1,SIGNAL(clicked()), this, SLOT(outsidePreprocess()));
	connect(general_para12->preprocessButton,SIGNAL(clicked()), this, SLOT(resetPreprocessStage()));
	connect(general_para12,SIGNAL(reprocess(int)), this, SLOT(Reprocess_Vessel(int)));

	connect(general_para12->outputSeeds,SIGNAL(clicked()), this, SLOT(outputSeeds()));
	connect(general_para2->tracingButton,SIGNAL(clicked()), this, SLOT(Init_Tracing()));

	connect(general_para3->load_swc_button, SIGNAL(clicked()), this, SLOT(loadSWC()));
	connect(general_para3->output_swc_raw_button, SIGNAL(clicked()), this, SLOT(output_swc_raw_slot()));
	connect(general_para3->output_swc_button, SIGNAL(clicked()), this, SLOT(outputSWC()));

	general_para3->output_swc_button->setEnabled(false);

	connect(backButton,SIGNAL(clicked()), this, SLOT(backStep()));
	connect(nextButton,SIGNAL(clicked()), this, SLOT(nextStep()));
	connect(startButtonI,SIGNAL(clicked()), this, SLOT(batchTracingI()));
	connect(startButtonII,SIGNAL(clicked()), this, SLOT(batchTracingII()));

	connect(slider,SIGNAL(stateChanged(int)), this, SLOT(changeDisplay(int)));
	connect(slider,SIGNAL(displaySeedsChanged(bool)), tracingViewer, SLOT(displaySeed(bool)));
	connect(slider,SIGNAL(displaySnakesChanged(bool)), tracingViewer, SLOT(displaySnake(bool)));

	connect(slider,SIGNAL(displaySeedsChanged(bool)), this, SLOT(draw3DTraces()));
	connect(slider,SIGNAL(displaySnakesChanged(bool)), this, SLOT(draw3DTraces()));

	connect(slider,SIGNAL(colorLinesChanged(bool)), tracingViewer, SLOT(displayColorLine(bool)));
	connect(slider,SIGNAL(colorLinesChanged(bool)), this, SLOT(draw3DTraces()));
	connect(slider,SIGNAL(radiusSphereChanged(int)), this, SLOT(draw3DTraces()));
	connect(slider,SIGNAL(radiusCircleChanged(int)), this, SLOT(draw3DTraces()));
	connect(slider,SIGNAL(radiusTubeChanged()), this, SLOT(draw3DTraces()));

	connect(slider,SIGNAL(radiusSphereChanged(int)), tracingViewer, SLOT(SnakesChanged(int)));
	connect(slider,SIGNAL(radiusCircleChanged(int)), tracingViewer, SLOT(SnakesChanged(int)));

	connect(slider,SIGNAL(valueChanged(int)), this, SLOT(changeSlice(int)));

	connect(imageTab,SIGNAL(currentChanged(int)), this, SLOT(tab_change(int)));

	qRegisterMetaType<SnakeClass>("SnakeClass");
	qRegisterMetaType<SnakeListClass>("SnakeListClass");

	connect(Tracer->tracing_thread,SIGNAL(stretched(SnakeClass)), tracingViewer, SLOT(setTracingSnake(SnakeClass)));
	connect(Tracer->tracing_thread,SIGNAL(stretched(SnakeClass)), this, SLOT(draw3DTracing(SnakeClass)));

	//connect(Tracer->tracing_thread,SIGNAL(snakeTraced()), tracingViewer, SLOT(SnakesChanged()));

	connect(Tracer->tracing_thread,SIGNAL(snakeTraced(bool)), this, SLOT(SnakesChangedSlot(bool)));

	connect(Tracer->tracing_thread,SIGNAL(snakeTraced_manual_seed(float)), this, SLOT(SnakesChangedSlot()));
	connect(Tracer->tracing_thread,SIGNAL(snakeTraced_manual_seed(float)), this, SLOT(updateFN(float)));

	qRegisterMetaType<Point3D>("Point3D");
	//connect(tracingViewer,SIGNAL(result_drawed()), this, SLOT(Tracing()));
	connect(tracingViewer,SIGNAL(manual_seed(PointList3D)), this, SLOT(Manual_Seed_Tracing(PointList3D)));
	//connect(tracingViewer,SIGNAL(manual_path(PointList3D)), this, SLOT(Manual_Seed_TracingI(PointList3D)));

	connect(tracingViewer,SIGNAL(point_clicked(Point3D)), this, SLOT(PointClickedSlot(Point3D)));
	//connect(tracingViewer,SIGNAL(result_drawed1()), Tracer->tracing_thread, SLOT(resume()));

	connect(this, SIGNAL(batch_tracingI_continue()), this, SLOT(batchTracingI()));
	connect(this, SIGNAL(batch_tracingII_continue()), this, SLOT(batchTracingII()));

	connect(this, SIGNAL(start_continue_process()), this, SLOT(Process()));

	connect(this, SIGNAL(init_the_tracing()), this, SLOT(Init_Tracing()));

	connect(this, SIGNAL(start_tracing()), this, SLOT(Tracing()));

	//connect vtk event and qt slots
	Connections = vtkEventQtSlotConnect::New();
	Connections->Connect(this->QVTK->GetRenderWindow()->GetInteractor(),
		vtkCommand::RightButtonPressEvent,
		this,
		SLOT(vtk_right_pick(vtkObject *)));

	Connections->Connect(this->QVTK->GetRenderWindow()->GetInteractor(),
		vtkCommand::LeftButtonPressEvent,
		this,
		SLOT(vtk_left_pick(vtkObject *)));

	Connections->Connect(this->QVTK->GetRenderWindow()->GetInteractor(),
		vtkCommand::LeftButtonPressEvent,
		this,
		SLOT(vtk_removePoint()));

	this->setCentralWidget(main_splitter);
	this->statusBar()->showMessage("Ready");


	//edits info
	this->PrecisionLabel = new QLabel(this);
	this->PrecisionLabel->setText(QString::number(edits->precision));
	this->RecallLabel = new QLabel(this);
	this->RecallLabel->setText(QString::number(edits->recall));

	this->BranchLabel = new QLabel(this);
	this->BranchLabel->setText(QString::number(edits->numBranch));
	this->SplitLabel = new QLabel(this);
	this->SplitLabel->setText(QString::number(edits->numSplit));
	this->DeleteLabel = new QLabel(this);
	this->DeleteLabel->setText(QString::number(edits->numDelete));
	this->MergeLabel = new QLabel(this);
	this->MergeLabel->setText(QString::number(edits->numMerge));

	this->statusBar()->addPermanentWidget(new QLabel("Precision: ", this));
	this->statusBar()->addPermanentWidget(this->PrecisionLabel,0);
	this->statusBar()->addPermanentWidget(new QLabel("Recall: ", this));
	this->statusBar()->addPermanentWidget(this->RecallLabel,0);
	this->statusBar()->addPermanentWidget(new QLabel("Branched: ", this));
	this->statusBar()->addPermanentWidget(this->BranchLabel,0);
	this->statusBar()->addPermanentWidget(new QLabel("Split: ", this));
	this->statusBar()->addPermanentWidget(this->SplitLabel,0);
	this->statusBar()->addPermanentWidget(new QLabel(" Deleted: ", this));
	this->statusBar()->addPermanentWidget(this->DeleteLabel,0);
	this->statusBar()->addPermanentWidget(new QLabel(" Merged: ", this));
	this->statusBar()->addPermanentWidget(this->MergeLabel,0);
}

void QtTracer::decimateSeeds()
{
	float percent = 0.1 * original_seed_num/IM->SeedPt.NP;
	vnl_random r;
	PointList3D new_seeds;
	for( int i = 0; i < IM->SeedPt.NP; i++ )
	{
		if( r.drand64() >= percent )
			new_seeds.AddPt(IM->SeedPt.Pt[i]);
	}
	IM->SeedPt = new_seeds;
	IM->visit_label.set_size(IM->SeedPt.GetSize());
	IM->visit_label.fill(0);
}

void QtTracer::showAllSetting()
{
	general_para->setVisible(true);
	general_para12->setVisible(true);
	general_para2->setVisible(true);
	general_para3->setVisible(true);
}


void QtTracer::setEliteSetting()
{
	general_para->setEliteSetting();
	general_para12->setEliteSetting();
	general_para2->setEliteSetting();
	general_para3->setEliteSetting();
}

void QtTracer::setFullSetting()
{
	general_para->setFullSetting();
	general_para12->setFullSetting();
	general_para2->setFullSetting();
	general_para3->setFullSetting();
}


void QtTracer::deleteSnake()
{
	tracingViewer->deleteSnake(edits);
	draw3DTraces();

	this->DeleteLabel->setText(QString::number(edits->numDelete));
	this->PrecisionLabel->setText(QString::number(edits->precision));
	this->RecallLabel->setText(QString::number(edits->recall));

	//recompute the features
	if( show_table_scatter )
		displayFeatures();
}
void QtTracer::mergeSnake()
{
	tracingViewer->mergeSnake(edits);
	this->MergeLabel->setText(QString::number(edits->numMerge));

	//recompute the features
	if( show_table_scatter )
		displayFeatures();
}
void QtTracer::splitSnake()
{
	tracingViewer->splitSnake(edits);
	this->SplitLabel->setText(QString::number(edits->numSplit));

	//recompute the features
	if( show_table_scatter )
		displayFeatures();
}
void QtTracer::branchSnake()
{
	tracingViewer->createBranch(edits);
	this->BranchLabel->setText(QString::number(edits->numBranch));

	//recompute the features
	if( show_table_scatter )
		displayFeatures();
}
void QtTracer::updateFN(float fn)
{
	edits->FN += fn;
	edits->recall = edits->TP/(edits->TP + edits->FN);
	this->RecallLabel->setText(QString::number(edits->recall));
}

void QtTracer::createActions()
{
	Go = new QAction(QIcon(":/icons/Go.png"), tr("Go"), this);
	//Resume->setShortcut(tr("Ctrl+G"));
	Go->setToolTip(tr("Start tracing"));
	connect(Go, SIGNAL(triggered()), this, SLOT(tracing_go()));

	Pause = new QAction(QIcon(":/icons/Pause.png"), tr("Pause"), this);
	//Pause->setShortcut(tr("Ctrl+G"));
	Pause->setToolTip(tr("suspend tracing"));
	connect(Pause, SIGNAL(triggered()), this, SLOT(tracing_suspend()));

	Stop = new QAction(QIcon(":/icons/Stop.png"), tr("Stop"), this);
	//Resume->setShortcut(tr("Ctrl+G"));
	Stop->setToolTip(tr("stop tracing"));
	connect(Stop, SIGNAL(triggered()), this, SLOT(tracing_stop()));

	Pick_Seed = new QAction(tr("Manual Seed"), this); 
	//enable the manual seeding
	connect(Pick_Seed,SIGNAL(triggered()), this, SLOT(enable_manual_seeding())); 
	Pick_Seed->setShortcut(tr("A"));
	//stop the tracing thread
	//connect(Pick_Seed,SIGNAL(triggered()), Tracer->tracing_thread, SLOT(stop()));

	Delete_Seed = new QAction(tr("Delete Seed"), this);
	connect(Delete_Seed,SIGNAL(triggered()), tracingViewer, SLOT(deleteSeed())); 

	Invert_Selection = new QAction(tr("Invert Selection"), this);
	connect(Invert_Selection,SIGNAL(triggered()), tracingViewer, SLOT(invertSelection()));

	Decimate_Seed = new QAction(tr("Decimate Seed"), this);
	connect(Decimate_Seed,SIGNAL(triggered()), this, SLOT(decimateSeeds()));

	Delete_Snake = new QAction(tr("Delete Snake"), this);
	connect(Delete_Snake,SIGNAL(triggered()), this, SLOT(deleteSnake())); 
	Delete_Snake->setShortcut(tr("D"));

	Split_Snake = new QAction(tr("Split Snake"), this);
	connect(Split_Snake,SIGNAL(triggered()), this, SLOT(splitSnake()));
	Split_Snake->setShortcut(tr("S"));

	Merge_Snake = new QAction(tr("Merge Snake"), this);
	connect(Merge_Snake,SIGNAL(triggered()), this, SLOT(mergeSnake())); 
	Merge_Snake->setShortcut(tr("M"));

	Set_Root = new QAction(tr("Set Root"), this);
	connect(Set_Root,SIGNAL(triggered()), this, SLOT(setRootPoint())); 
	Set_Root->setShortcut(tr("R"));

	Clear_Tree = new QAction(tr("Clear Tree"), this);
	connect(Clear_Tree,SIGNAL(triggered()), this, SLOT(clearSnakeTree()));

	Refine_Branch = new QAction(tr("Refine Branching"), this);
	connect(Refine_Branch,SIGNAL(triggered()), this, SLOT(refineBranch()));
	//Refine_Branch->setShortcut(tr("B"));

	Create_Branch = new QAction(tr("Create Branching"), this);
	connect(Create_Branch,SIGNAL(triggered()), this, SLOT(branchSnake()));
	Create_Branch->setShortcut(tr("B"));

	Remove_Soma = new QAction(tr("Remove Soma"), this);
	connect(Remove_Soma,SIGNAL(triggered()), this, SLOT(removeSoma()));

	Save_Soma = new QAction(tr("Save Soma"), this);
	connect(Save_Soma,SIGNAL(triggered()), this, SLOT(saveSoma()));

	Pick_Soma_Seeds = new QAction(tr("Pick Soma Seeds"), this);
	connect(Pick_Soma_Seeds,SIGNAL(triggered()), this, SLOT(pickSomaSeeds()));

	Load_Soma_Seeds = new QAction(tr("Load Soma Seeds"), this);
	connect(Load_Soma_Seeds,SIGNAL(triggered()), this, SLOT(loadSomaSeeds()));

	Segment_Soma = new QAction(tr("Segment Soma"),this);
	connect(Segment_Soma,SIGNAL(triggered()), this, SLOT(segmentSoma()));

	Load_Soma = new QAction(tr("Load Soma"),this);
	connect(Load_Soma,SIGNAL(triggered()), this, SLOT(loadSoma()));

	Clear_Segmentation = new QAction(tr("Clear Segmentation"),this);
	connect(Clear_Segmentation,SIGNAL(triggered()), this, SLOT(clearSegmentation()));

	SegmentI = new QAction(tr("Post-segmentation I"), this);
	SegmentI->setToolTip("2-Label Segmentation");
	connect(SegmentI,SIGNAL(triggered()), this, SLOT(SegmentationI()));

	SegmentII = new QAction(tr("Post-segmentation II"), this);
	connect(SegmentII,SIGNAL(triggered()), this, SLOT(SegmentationII()));
	SegmentII->setToolTip("Multi-Label Segmentation");

	Save_Segmentation = new QAction(tr("Save Segmentation"), this);
	connect(Save_Segmentation,SIGNAL(triggered()), this, SLOT(saveSegmentation()));

	Remove_Isolated = new QAction(tr("Select Isolated Traces"), this);
	connect(Remove_Isolated,SIGNAL(triggered()), this, SLOT(removeIsolated()));

	Break_Branches = new QAction(tr("Break Branches"), this);
	connect(Break_Branches,SIGNAL(triggered()), this, SLOT(breakBranches()));

	Save_Image = new QAction(tr("Save Image"), this);
	connect(Save_Image,SIGNAL(triggered()), this, SLOT(saveImage()));
	Save_Image->setShortcut(tr("Ctrl+S"));

	Save_Setting = new QAction(tr("Save Settings"), this);
	connect(Save_Setting,SIGNAL(triggered()), this, SLOT(save_settings()));
	Save_Setting->setShortcut(tr("Ctrl+A"));

	Load_Setting = new QAction(tr("Load Settings"), this);
	connect(Load_Setting,SIGNAL(triggered()), this, SLOT(load_settings()));
	Load_Setting->setShortcut(tr("Ctrl+L"));

	loadImageAct = new QAction(tr("Open Image"), this);
	loadImageAct->setShortcut(tr("Ctrl+O"));
	connect(loadImageAct, SIGNAL(triggered()), this, SLOT(loadImage()));

	exitAct = new QAction(tr("Exit"), this);
	exitAct->setShortcut(tr("Ctrl+Q"));
	connect(exitAct, SIGNAL(triggered()), this, SLOT(exit()));

	aboutAct = new QAction(tr("About"), this);
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

	zoomInAct = new QAction(tr("Zoom In"), this);
	zoomInAct->setShortcut(tr("Ctrl+Z"));
	connect(zoomInAct, SIGNAL(triggered()), tracingViewer, SLOT(zoomIn()));

	zoomOutAct = new QAction(tr("Zoom Out"), this);
	zoomOutAct->setShortcut(tr("Ctrl+X"));
	connect(zoomOutAct, SIGNAL(triggered()), tracingViewer, SLOT(zoomOut()));

	normalSizeAct = new QAction(tr("Original Size"), this);
	connect(normalSizeAct, SIGNAL(triggered()), tracingViewer, SLOT(normalSize()));

	showScatterAction = new QAction(tr("Show Scatter Plot"), this);
	connect(showScatterAction, SIGNAL(triggered()), this, SLOT(showScatter()));

	showTableAction = new QAction(tr("Show Table Plot"), this);
	connect(showTableAction, SIGNAL(triggered()), this, SLOT(showTable()));

	loadSWCAction = new QAction(tr("Load SWC"), this);
	loadSWCAction->setShortcut(tr("Shift+F3"));
	connect(loadSWCAction, SIGNAL(triggered()), this, SLOT(loadSWC()));

	Show_AllSetting = new QAction(tr("Show Settings"), this);
	connect(Show_AllSetting, SIGNAL(triggered()), this, SLOT(showAllSetting()));

	Set_EliteSetting = new QAction(tr("Elite Settings"), this);
	connect(Set_EliteSetting, SIGNAL(triggered()), this, SLOT(setEliteSetting()));

	Set_FullSetting = new QAction(tr("Full Settings"), this);
	connect(Set_FullSetting, SIGNAL(triggered()), this, SLOT(setFullSetting()));

	SaveRendering = new QAction(tr("Save Rendering to Image"), this);
	connect(SaveRendering, SIGNAL(triggered()), this, SLOT(writeRendering()));
	SaveRendering->setShortcut(tr("Ctrl+R"));

    SaveRenderingAnimation = new QAction(tr("Save Rendering to Animation"), this);
	connect(SaveRenderingAnimation, SIGNAL(triggered()), this, SLOT(writeRenderingAnimation()));

	outputSWCAction = new QAction(tr("Output SWC"), this);
	connect(outputSWCAction, SIGNAL(triggered()), this, SLOT(outputSWC()));

	outputSWCRawAction = new QAction(tr("Output SWC Raw"), this);
	connect(outputSWCRawAction, SIGNAL(triggered()), this, SLOT(outputSWC_Raw()));

	Output_XLS = new QAction(tr("Save Features to XLS"), this);
	connect(Output_XLS, SIGNAL(triggered()), this, SLOT(outputXLS()));
	Output_XLS->setShortcut(tr("Shift+F4")); 

	Detect_Spines = new QAction(tr("Detect Spines"), this);
	connect(Detect_Spines, SIGNAL(triggered()), this, SLOT(detectSpines()));
}

void QtTracer::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(loadImageAct);
	fileMenu->addAction(Save_Image);
	fileMenu->addSeparator();
	fileMenu->addAction(loadSWCAction);
	fileMenu->addSeparator();
	fileMenu->addAction(exitAct);

	SeedMenu = menuBar()->addMenu(tr("&Seeds"));
	SeedMenu->addAction(Pick_Seed);
	SeedMenu->addAction(Delete_Seed);
	SeedMenu->addAction(Invert_Selection);
	SeedMenu->addAction(Decimate_Seed);

	SnakeMenu = menuBar()->addMenu(tr("&Snakes"));
	SnakeMenu->addAction(Delete_Snake);
	SnakeMenu->addAction(Split_Snake);
	SnakeMenu->addAction(Merge_Snake);

	BranchMenu = menuBar()->addMenu(tr("&Branches"));
	BranchMenu->addAction(Create_Branch);
	BranchMenu->addAction(Break_Branches);

	TreeMenu = menuBar()->addMenu(tr("&Tree"));
	TreeMenu->addAction(Set_Root);
	TreeMenu->addAction(Clear_Tree);

	SomaMenu = menuBar()->addMenu(tr("&Soma"));
	SomaMenu->addAction(Save_Soma);
	SomaMenu->addAction(Pick_Soma_Seeds);
	SomaMenu->addAction(Load_Soma_Seeds);
	SomaMenu->addAction(Remove_Soma);
	SomaMenu->addAction(Segment_Soma);
	SomaMenu->addAction(Load_Soma);
	SomaMenu->addAction(Clear_Segmentation);

	PostSegMenu = menuBar()->addMenu(tr("&Segmentation"));
	PostSegMenu->addAction(SegmentI);
	PostSegMenu->addAction(SegmentII);
	PostSegMenu->addAction(Save_Segmentation);

	OutlierMenu = menuBar()->addMenu(tr("&Outlier Removal"));
	OutlierMenu->addAction(Remove_Isolated);

	SettingMenu = menuBar()->addMenu(tr("&Settings"));
	SettingMenu->addAction(Show_AllSetting);
	SettingMenu->addAction(Set_EliteSetting);
	SettingMenu->addAction(Set_FullSetting);
	SettingMenu->addSeparator();
	SettingMenu->addAction(Load_Setting);
	SettingMenu->addAction(Save_Setting);

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(showScatterAction);
	viewMenu->addAction(showTableAction);
	viewMenu->addSeparator();
	viewMenu->addAction(zoomInAct);
	viewMenu->addAction(zoomOutAct);
	viewMenu->addAction(normalSizeAct);

	outputMenu = menuBar()->addMenu(tr("&Output"));
	outputMenu->addAction(SaveRendering);
	outputMenu->addAction(SaveRenderingAnimation);
	outputMenu->addAction(Output_XLS);
	outputMenu->addAction(Save_Image);

	aboutMenu = menuBar()->addMenu(tr("&About"));
	aboutMenu->addAction(aboutAct);
}

void QtTracer::createToolBar()
{
	Tool = this->addToolBar(tr("Operation Buttons and Progress Bar"));

	Tool->addAction(Go);
	Tool->addAction(Pause);
	Tool->addAction(Stop);
	Progress = new QProgressBar();
	Progress->setMinimum(0);
	Progress->setMaximum(1);

	Tool->addWidget(Progress);

	Tool_Misc = this->addToolBar(tr("Operations"));
	//Tool_Misc->addAction(Pick_Seed);
	//Tool_Misc->addAction(Delete_Seed);
	//Tool_Misc->addAction(Invert_Selection);
	Tool_Misc->addSeparator();
	//Tool_Misc->addAction(Delete_Snake);
	//Tool_Misc->addAction(Split_Snake);
	//Tool_Misc->addAction(Merge_Snake);
	//Tool_Misc->addSeparator();
	Tool_Misc->addAction(Set_Root);
	Tool_Misc->addAction(Clear_Tree);
	Tool_Misc->addSeparator();
	//Tool_Misc->addAction(Refine_Branch);
	Tool_Misc->addAction(Create_Branch);
	Tool_Misc->addAction(Break_Branches);
	Tool_Misc->addSeparator();
	Tool_Misc->addAction(Save_Soma);
	Tool_Misc->addAction(Pick_Soma_Seeds);
	Tool_Misc->addAction(Load_Soma_Seeds);
	Tool_Misc->addAction(Remove_Soma);
	Tool_Misc->addAction(Segment_Soma);
	Tool_Misc->addAction(Clear_Segmentation);
	Tool_Misc->addSeparator();
	Tool_Misc->addAction(SegmentI);
	Tool_Misc->addAction(SegmentII);
	Tool_Misc->addSeparator();
	Tool_Misc->addAction(Detect_Spines);
	Tool_Misc->addAction(Remove_Isolated);
	Tool_Misc->addAction(loadSWCAction);
}

void QtTracer::createViews()
{
	//create model 
	model = new QStandardItemModel(1, 9, this);
	model->setHeaderData(0, Qt::Horizontal, tr("Length"));
	model->setHeaderData(1, Qt::Horizontal, tr("Average Radius"));
	model->setHeaderData(2, Qt::Horizontal, tr("Volume"));
	model->setHeaderData(3, Qt::Horizontal, tr("Surface"));
	model->setHeaderData(4, Qt::Horizontal, tr("Average Section Area"));
	model->setHeaderData(5, Qt::Horizontal, tr("#Snaxels"));
	model->setHeaderData(6, Qt::Horizontal, tr("Tortuosity"));
	model->setHeaderData(7, Qt::Horizontal, tr("Average Intensity"));
	model->setHeaderData(8, Qt::Horizontal, tr("Average Vesselness"));

	//create table and scatter views
	table = new QTableView;
	//QFont newFont("Courier", 15, QFont::Bold, false);
	//table->setFont(newFont);
	table->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents); 
	table_sum = new QTableView;
	//table_sum->setFont(newFont);
	table_sum->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents); 

	scatterPlot = new ScatterView;
	selections = new ObjectSelection;

	table->setEditTriggers(0);
	table->setModel(model);

	table_sum->setEditTriggers(0);

	selectionModel = new QItemSelectionModel(model);
	table->setSelectionModel(selectionModel);
	table->setAlternatingRowColors(true);
	tracingViewer->setModel(model,selections);

	scatterPlot->setModels(model,selections);
	scatterPlot->setWindowTitle("Scatter Plot");

	//connect selections
	connect(selectionModel,SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
		this, SLOT(setSelected(const QItemSelection &, const QItemSelection &)));
	connect(selections,SIGNAL(changed(void)), this, SLOT(setSelected_sels(void)));
	connect(selections,SIGNAL(changed(void)), this, SLOT(drawClickedTraces(void)));

	//Features
	//table->show();
	table->setGeometry(QRect(400,400,800,100));
	table->setWindowTitle("Snake Features");

	//scatterPlot->show();
	scatterPlot->setGeometry(QRect(50,50,450,450));

}

void QtTracer::enable_manual_seeding()
{
	seeding = true;
	m_seed.RemoveAllPts();
	//m_seed.x = 0;
	//m_seed.y = 0;
	//m_seed.z = 0;
	tracingViewer->setPickingCursor();
	this->statusBar()->showMessage("Manually pick a seed");
}
void QtTracer::SnakesChangedSlot() //snake changed slot for manual seeding (including seeding snakes)
{
	//recompute the features
	if( show_table_scatter )
		displayFeatures();


	if( Progress->value() != Progress->maximum() )
	{
		//remove the seeds
		if( !batch_processII ||  seed_snakes.NSnakes == 0 || !seeding )
		{
			Tracer->RemoveSeeds();

			//Progress->setMaximum(IM->SeedPt.GetSize());
		}

		Progress->setValue(0);
		Progress->setValue(IM->visit_label.sum());
		//tracing finished
		if( IM->SeedPt.GetSize() == IM->visit_label.sum() )
		{

			tracing_stop();

			//updates information for editing
			if( !batch_processI && !batch_processII )
			{
				for(int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
				{
					if( Tracer->SnakeList.valid_list[i] == 0 )
						continue;
					edits->TP += Tracer->SnakeList.Snakes[i].Cu.GetLength();
				}
				edits->precision = edits->TP/(edits->TP + edits->FP);
				edits->recall = edits->TP/(edits->TP + edits->FN);

				this->PrecisionLabel->setText(QString::number(edits->precision));
				this->RecallLabel->setText(QString::number(edits->recall));
			}

			/*general_para2->enableSetting();

			tracing_started = false;
			Tracer->tracing = false;

			//Tracer->tracing_thread->stopped = true;
			Tracer->tracing_thread->stop();

			nextButton->setEnabled(true);

			//reset seeds status of IM
			IM->visit_label.fill(0);*/

			Tracer->tracing = false;

			draw3DTraces();
			tracingViewer->SnakesChanged();

			std::cout<<"--------------Processing Finished--------------"<<std::endl;
			//go to next step when automated processing
			if( automated_process || batch_processI || batch_processII )
				//if( automated_process || batch_processI )
			{
				nextStep();
			}
			return;
		}
	}

	//tracingViewer->setSeeds_VisitLabel(&Tracer->visit_label);
	//check if the seed snake tracing is finished
	if( batch_processII && seed_snakes.NSnakes != 0 && seeding )
	{
		//check if there is any seed snake left
		int sum = 0;
		for ( std::vector<int>::iterator it=seed_snakes.valid_list.begin() ; it < seed_snakes.valid_list.end(); it++ )
			sum += *it;
		if( seed_snakes.NSnakes == 0 || sum == 0 )
		{
			seeding = false;
			//when seed snake tracing is finished, remove all the seeds covered by the snakes
			Tracer->RemoveSeeds_SN();
		}

	}

	//if( seeding && seed_snakes.NSnakes != 0 )
	if( batch_processII && seed_snakes.NSnakes != 0 && seeding ) 
	{
		tracingViewer->SnakesChanged();
		draw3DTraces();
		Seed_Snake_Tracing();
	}
	else
	{
		if( Tracer->use_multi_threads )
		{
			thread_finished++;
			if( thread_finished < 6 )
				return;
			else
			{
				thread_finished = 0;
				//tracingViewer->SnakesChanged();
				//draw3DTraces();
				//tracing_suspend();
			}
		}

		//for manual seeding in normal mode, change seeding state to false after tracing from manual seed is finished
		if( seeding = true )
		{
			seeding = false;
			//in case of manual seed tracing after tracing is stopped, need to switch tracing thread to be stopped
			if( Progress->value() == Progress->maximum() || !tracing_started )
			{
				Tracer->tracing_thread->stopped = true;
			}
		}

		Tracer->tracing = false;

		tracingViewer->SnakesChanged();
		//draw3DTraces();
		drawNew3DTrace(); //faster way to display new trace

		emit start_tracing();

	}
}

void QtTracer::SnakesChangedSlot(bool new_come)
{
	//std::cout<<"snake changed slot........."<<std::endl;

	//recompute the features
	if( show_table_scatter )
		displayFeatures();


	if( Progress->value() != Progress->maximum() )
	{
		//remove the seeds
		if( !batch_processII ||  seed_snakes.NSnakes == 0 || !seeding )
			Tracer->RemoveSeeds();

		Progress->setValue(0);
		Progress->setValue(IM->visit_label.sum());
		//tracing finished
		if( IM->SeedPt.GetSize() == IM->visit_label.sum() )
		{

			tracing_stop();

			//updates information for editing
			if( !batch_processI && !batch_processII )
			{
				for(int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
				{
					if( Tracer->SnakeList.valid_list[i] == 0 )
						continue;
					edits->TP += Tracer->SnakeList.Snakes[i].Cu.GetLength();
				}
				edits->precision = edits->TP/(edits->TP + edits->FP);
				edits->recall = edits->TP/(edits->TP + edits->FN);

				this->PrecisionLabel->setText(QString::number(edits->precision));
				this->RecallLabel->setText(QString::number(edits->recall));
			}

			/*general_para2->enableSetting();

			tracing_started = false;
			Tracer->tracing = false;

			//Tracer->tracing_thread->stopped = true;
			Tracer->tracing_thread->stop();

			nextButton->setEnabled(true);

			//reset seeds status of IM
			IM->visit_label.fill(0);*/

			Tracer->tracing = false;

			draw3DTraces();
			tracingViewer->SnakesChanged();

			std::cout<<std::endl;
			std::cout<<"--------------Processing Finished--------------"<<std::endl;
			//go to next step when automated processing
			if( mg_process )
			{
				nextStep();
			}

			if( automated_process || batch_processI || batch_processII )
				//if( automated_process || batch_processI )
			{
				nextStep();
			}
			return;
		}
	}

	//tracingViewer->setSeeds_VisitLabel(&Tracer->visit_label);
	//check if the seed snake tracing is finished
	if( batch_processII && seed_snakes.NSnakes != 0 && seeding )
	{
		//check if there is any seed snake left
		int sum = 0;
		for ( std::vector<int>::iterator it=seed_snakes.valid_list.begin() ; it < seed_snakes.valid_list.end(); it++ )
			sum += *it;
		if( seed_snakes.NSnakes == 0 || sum == 0 )
		{
			seeding = false;
			//when seed snake tracing is finished, remove all the seeds covered by the snakes
			Tracer->RemoveSeeds_SN();
		}

	}

	//if( seeding && seed_snakes.NSnakes != 0 )
	if( batch_processII && seed_snakes.NSnakes != 0 && seeding ) 
	{
		tracingViewer->SnakesChanged();
		draw3DTraces();
		Seed_Snake_Tracing();
	}
	else
	{
		if( Tracer->use_multi_threads )
		{
			thread_finished++;
			if( thread_finished < 6 )
				return;
			else
			{
				thread_finished = 0;
				//tracingViewer->SnakesChanged();
				//draw3DTraces();
				//tracing_suspend();
			}
		}


		Tracer->tracing = false;

		tracingViewer->SnakesChanged();
		//draw3DTraces();
		if( new_come)
			drawNew3DTrace(); //faster way to display new trace

		emit start_tracing();

	}
}

void QtTracer::PointClickedSlot(Point3D click_point)
{
	QString MSG("Point: ");
	QString X_NUM;
	X_NUM.setNum(click_point.x);
	MSG.append(X_NUM);
	MSG.append(", ");
	QString Y_NUM;
	Y_NUM.setNum(click_point.y);
	MSG.append(Y_NUM);

	if( batch_processII )
		return;

	if( slider->State == 1 && click_point.z != -1 )
	{
		ImageType::IndexType idx;
		idx[0] = click_point.x;
		idx[1] = click_point.y;
		idx[2] = click_point.z;
		int intensity = IM->I->GetPixel(idx);
		QString I_NUM;
		I_NUM.setNum(intensity);
		MSG.append(" Intensity: ");
		MSG.append(I_NUM);
	}
	else if ( slider->State == 2 && click_point.z != -1 )
	{
		ImageType::IndexType idx;
		idx[0] = click_point.x;
		idx[1] = click_point.y;
		idx[2] = click_point.z;
		int intensity = IM->IVessel->GetPixel(idx);
		QString I_NUM;
		I_NUM.setNum(intensity);
		MSG.append(" Intensity: ");
		MSG.append(I_NUM);
	}
	else if ( slider->State == 3 && click_point.z != -1 )
	{
		ImageType::IndexType idx;
		idx[0] = click_point.x;
		idx[1] = click_point.y;
		idx[2] = click_point.z;
		int intensity = IM->IL->GetPixel(idx);
		QString I_NUM;
		I_NUM.setNum(intensity);
		MSG.append(" Intensity: ");
		MSG.append(I_NUM);
	}

	//if( click_point.z != -1 )
	// drawClickedTraces();

	this->statusBar()->showMessage(MSG);
}

void QtTracer::tab_change(int in)
{
	int index = imageTab->indexOf(scrollArea_BW);
	int index1 = imageTab->indexOf(scrollArea_MT);
	int index2 = imageTab->indexOf(QVTK);

	opacity->setVisible(false);
	opacity_slider->setVisible(false);
	opacity_th->setVisible(false);
	opacity_th_slider->setVisible(false);
	brightness->setVisible(false);
	brightness_slider->setVisible(false);
	//line_width->setVisible(false);
	//line_width_slider->setVisible(false);

	if( index == in )
	{ 
		set_cross_section_view(slider->State);
	}
	else if( index1 == in && !montage_created )
	{
		createMontage();
	}
	else if( index2 == in )
	{
		opacity->setVisible(true);
		opacity_slider->setVisible(true);
		opacity_th->setVisible(true);
		opacity_th_slider->setVisible(true);
		brightness->setVisible(true);
		brightness_slider->setVisible(true);
		//line_width->setVisible(true);
		//line_width_slider->setVisible(true);
		draw3DTraces();
		drawClickedTraces();
	}
}

void QtTracer::set_cross_section_view(int in)
{
	int x,y,z;
	int width = 3;
	x = tracingViewer->getClickedPoint().x;
	y = tracingViewer->getClickedPoint().y;
	z = tracingViewer->getClickedPoint().z;

	if( x != 0 && y != 0 && z != 0 )
	{
		QPointF *SP1= new QPointF[2];
		QPointF *SP2= new QPointF[2];

		QImage xy_slice,yz_slice,xz_slice;

		if(in==1)
		{
			xy_slice = convertITK2QT(IM->extract_one_slice(IM->ImRescale(IM->I),z), false);
			yz_slice = convertITK2QT(IM->extract_one_slice_yz(IM->ImRescale(IM->I),x), false);
			xz_slice = convertITK2QT(IM->extract_one_slice_xz(IM->ImRescale(IM->I),y), false);
		}
		else if(in==2)
		{
			xy_slice = convertITK2QT(IM->extract_one_slice(IM->ImRescale(IM->IVessel),z), false);
			yz_slice = convertITK2QT(IM->extract_one_slice_yz(IM->ImRescale(IM->IVessel),x), false);
			xz_slice = convertITK2QT(IM->extract_one_slice_xz(IM->ImRescale(IM->IVessel),y), false);
		}
		else if(in==3)
		{
			xy_slice = convertITK2QT(IM->extract_one_slice(IM->ImRescale(IM->IL),z), false);
			yz_slice = convertITK2QT(IM->extract_one_slice_yz(IM->ImRescale(IM->IL),x), false);
			xz_slice = convertITK2QT(IM->extract_one_slice_xz(IM->ImRescale(IM->IL),y), false);
		}

		QPainter painter1(&xy_slice); //painter for painting on the image
		painter1.setCompositionMode(QPainter::CompositionMode_SourceOver);
		int diameter = 2;
		painter1.setPen(QColor(200,0,0,255));
		//painter1.setBrush(Qt::yellow);
		//painter1.drawEllipse(QRect(x-diameter / 2.0, y-diameter / 2.0,
		//  diameter*2, diameter*2));
		SP1[0].setX(x-width);
		SP1[0].setY(y);
		SP1[1].setX(x+width);
		SP1[1].setY(y);
		SP2[0].setX(x);
		SP2[0].setY(y-width);
		SP2[1].setX(x);
		SP2[1].setY(y+width);
		painter1.drawPolyline(SP1,2);
		painter1.drawPolyline(SP2,2);
		imageLabel->setPixmap(QPixmap::fromImage(xy_slice));

		QPainter painter2(&yz_slice); //painter for painting on the image
		painter2.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter2.setPen(QColor(200,0,0,255));
		//painter2.setBrush(Qt::yellow);
		//painter2.drawEllipse(QRect(z-diameter / 2.0, y-diameter / 2.0,
		//  diameter*2, diameter*2));
		SP1[0].setX(z-width);
		SP1[0].setY(y);
		SP1[1].setX(z+width);
		SP1[1].setY(y);
		SP2[0].setX(z);
		SP2[0].setY(y-width);
		SP2[1].setX(z);
		SP2[1].setY(y+width);
		painter2.drawPolyline(SP1,2);
		painter2.drawPolyline(SP2,2);
		crossSectionLabel1->setPixmap(QPixmap::fromImage(yz_slice));

		QPainter painter3(&xz_slice); //painter for painting on the image
		painter3.setCompositionMode(QPainter::CompositionMode_SourceOver);
		painter3.setPen(QColor(200,0,0,255));
		//painter3.setBrush(Qt::yellow);
		//painter3.drawEllipse(QRect(x-diameter / 2.0, z-diameter / 2.0,
		//  diameter*2, diameter*2));
		SP1[0].setX(x-width);
		SP1[0].setY(z);
		SP1[1].setX(x+width);
		SP1[1].setY(z);
		SP2[0].setX(x);
		SP2[0].setY(z-width);
		SP2[1].setX(x);
		SP2[1].setY(z+width);
		painter3.drawPolyline(SP1,2);
		painter3.drawPolyline(SP2,2);
		crossSectionLabel2->setPixmap(QPixmap::fromImage(xz_slice));
	}
}

void QtTracer::tracing_go()
{
	if( Process_Stage == 2 )
	{
		if( general_para2->enabled ) 
		{
			Init_Tracing();
		}
		else
		{
			Tracer->tracing_thread->resume();
		}
	}
}

void QtTracer::tracing_suspend()
{
	if( Process_Stage == 2)
	{
		Tracer->tracing_thread->suspend();
	}
}

void QtTracer::tracing_stop()
{
	if( Process_Stage == 2 )
	{
		Progress->setValue(Progress->maximum());

		End_t = time(NULL);
		tracing_time = difftime(End_t, Start_t);
		QString MSG("Time Elapsed for Tracing (including editing time): ");
		QString MSG_Num;
		MSG_Num.setNum(tracing_time);
		MSG.append(MSG_Num);
		MSG.append(" seconds");

		this->statusBar()->showMessage(MSG);

		general_para2->enableSetting();

		nextButton->setEnabled(true);

		//reset seeds status of IM
		IM->visit_label.fill(0);


		Tracer->tracing_thread->stop();

		tracing_started = false;

		//tracingViewer->removeSnakes();
		//go to next step when automated processing
		//if( automated_process )
		//	 nextStep();
	}
}

void QtTracer::auto_mode_slot(bool in)
{
	if( in )
	{
		general_para->loadButton->setEnabled(false);
		general_para->reloadButton->setEnabled(false);
		general_para->loadDisplayButton->setEnabled(false);

		general_para->autoButton->setEnabled(false);
		general_para->batchButtonI->setEnabled(false);
		general_para->batchButtonII->setEnabled(false);

		automated_process = true;
		general_para12->setVisible(true);
		general_para2->setVisible(true);
		general_para3->setVisible(true);
		nextButton->setEnabled(true);
		//startButtonI->setVisible(false);
		//startButtonII->setVisible(false);
	}
	else
	{ 
		general_para->loadButton->setEnabled(true);
		general_para->reloadButton->setEnabled(true);
		general_para->loadDisplayButton->setEnabled(true);

		general_para->autoButton->setEnabled(true);
		general_para->batchButtonI->setEnabled(true);
		general_para->batchButtonII->setEnabled(true);

		automated_process = false;
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		general_para3->setVisible(false);
		startButtonI->setVisible(false);
		startButtonII->setVisible(false);
	}
}

void QtTracer::batchI_mode_slot(bool in)
{
	if( in )
	{
		general_para->loadButton->setEnabled(false);
		general_para->reloadButton->setEnabled(false);
		general_para->loadDisplayButton->setEnabled(false);

		general_para->autoButton->setEnabled(false);
		general_para->batchButtonI->setEnabled(false);
		general_para->batchButtonII->setEnabled(false);

		batch_processI = true;
		general_para12->setVisible(true);
		general_para2->setVisible(true);
		general_para3->setVisible(true);
		nextButton->setVisible(false);
		backButton->setVisible(false);
		//startButtonI->setVisible(true);
		//startButtonII->setVisible(false);

		general_para12->setCurveletBatchEnabled(true);
	}
	else
	{
		general_para->loadButton->setEnabled(true);
		general_para->reloadButton->setEnabled(true);
		general_para->loadDisplayButton->setEnabled(true);

		general_para->autoButton->setEnabled(true);
		general_para->batchButtonI->setEnabled(true);
		general_para->batchButtonII->setEnabled(true);

		batch_processI = false;
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		general_para3->setVisible(false);
		nextButton->setVisible(true);
		backButton->setVisible(true);
		startButtonI->setVisible(false);
		startButtonII->setVisible(false);

		general_para12->setCurveletBatchEnabled(false);
	}
}
void QtTracer::batchII_mode_slot(bool in)
{
	if( in )
	{
		general_para->loadButton->setEnabled(false);
		general_para->reloadButton->setEnabled(false);
		general_para->loadDisplayButton->setEnabled(false);

		general_para->autoButton->setEnabled(false);
		general_para->batchButtonI->setEnabled(false);
		general_para->batchButtonII->setEnabled(false);

		batch_processII = true;
		general_para12->setVisible(true);
		general_para2->setVisible(true);
		general_para3->setVisible(true);
		nextButton->setVisible(false);
		backButton->setVisible(false);
		//startButtonI->setVisible(false);
		//startButtonII->setVisible(true);

		general_para12->setCurveletBatchEnabled(true);
	}
	else
	{
		general_para->loadButton->setEnabled(true);
		general_para->reloadButton->setEnabled(true);
		general_para->loadDisplayButton->setEnabled(true);

		general_para->autoButton->setEnabled(true);
		general_para->batchButtonI->setEnabled(true);
		general_para->batchButtonII->setEnabled(true);

		batch_processII = false;
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		general_para3->setVisible(false);
		nextButton->setVisible(true);
		backButton->setVisible(true);
		startButtonI->setVisible(false);
		startButtonII->setVisible(false);

		general_para12->setCurveletBatchEnabled(false);
	}
}

void QtTracer::autoProcessing()
{
	//outsidePreprocess();
	//batchPreprocessing();
	if( !automated_process )
	{
		automated_process = true;
		general_para12->setVisible(true);
		general_para2->setVisible(true);
		nextButton->setEnabled(true);
		startButtonI->setVisible(false);
		startButtonII->setVisible(false);
	}
	else
	{ 
		automated_process = false;
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		startButtonI->setVisible(false);
		startButtonII->setVisible(false);
	}
}

void QtTracer::batchProcessingI()
{
	if( !batch_processI )
	{
		batch_processI = true;

		current_idx = -1;

		general_para12->setVisible(true);
		general_para2->setVisible(true);
		general_para3->setVisible(true);

		//specify the image folder
		QString dir_path = QFileDialog::getExistingDirectory(this, tr("Open Image Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		QStringList filters;
		foreach (QByteArray format, QImageReader::supportedImageFormats())
			filters += "*." + format;
		filters << "*.tiff" << "*.tif";

		images_dir.setPath(dir_path);
		images_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
		images_dir.setNameFilters(filters);

		string_array = new QString[images_dir.count()];
		for( unsigned int i = 0; i < images_dir.count(); i++ )
		{
			string_array[i] = images_dir[i];
		}

		this->statusBar()->showMessage(dir_path);
		//selected a directory containing no images
		if( images_dir.count() == 0){
			QMessageBox::warning(this, tr("Warning"),
				tr("No image exists in this directory"));
			return;

		}
		//specify the output swc folder
		swcs_path = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
		nextButton->setVisible(false);
		backButton->setVisible(false);
		startButtonI->setVisible(true);
		startButtonII->setVisible(false);

		this->statusBar()->showMessage(swcs_path);
	}
	else
	{ 
		batch_processI = false;
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		general_para3->setVisible(false);
		nextButton->setVisible(true);
		backButton->setVisible(true);
		startButtonI->setVisible(false);
		startButtonII->setVisible(false);
	}
}

void QtTracer::mgPrune_n_Save()
{
	//prune the snakes
	PointList3D temp;
	std::vector<float> r;

	std::vector<int> stem_label;

	for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
	{
		if( Tracer->SnakeList.valid_list[i] == 0 )
			continue;

		ImageType::IndexType index;
		std::vector<int> label;

		temp.RemoveAllPts();
		r.clear();

		for( int j = 0; j < Tracer->SnakeList.Snakes[i].Cu.NP; j++ )
		{
			index[0] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x;
			index[1] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y;
			index[2] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z;
			int is = IM->ISoma->GetPixel(index);
			label.push_back(is);

			if( is != current_mg_cell )
			{
				temp.AddPt(Tracer->SnakeList.Snakes[i].Cu.Pt[j]);
				r.push_back(Tracer->SnakeList.Snakes[i].Ru[j]);
			}
		}

		//filter out snake with two tips in the soma
		if( label[0] != current_mg_cell || label[Tracer->SnakeList.Snakes[i].Cu.NP-1] != current_mg_cell)
		{
			Tracer->SnakeList.Snakes[i].Cu = temp;
			Tracer->SnakeList.Snakes[i].Ru = r;
		}
		else
		{
			Tracer->SnakeList.valid_list[i] = 0;
			continue;
		}

		if( label[0] == current_mg_cell )
		{
			Tracer->SnakeList.Snakes[i].Cu.AddTailPt(IM->Centroid.Pt[current_mg_cell-1]);
			Tracer->SnakeList.Snakes[i].Ru.insert(Tracer->SnakeList.Snakes[i].Ru.begin(), IM->soma_radii[current_mg_cell-1]);
			stem_label.push_back(i);
		}
		else if(label[label.size()-1] == current_mg_cell)
		{
			Tracer->SnakeList.Snakes[i].Cu.AddPt(IM->Centroid.Pt[current_mg_cell-1]);
			Tracer->SnakeList.Snakes[i].Ru.push_back(IM->soma_radii[current_mg_cell-1]);

			//reverse the snake
			//Tracer->SnakeList.Snakes[i].Ru = Tracer->SnakeList.Snakes[i].Cu.Flip_4D(Tracer->SnakeList.Snakes[i].Ru);

			stem_label.push_back(i);
		}
	}

	std::cout<<"Output the SWC file....."<<std::endl;
	outputSWC_MG(stem_label);

	current_mg_cell++; 

	tracingViewer->SnakesChanged();
	draw3DTraces();
}

void QtTracer::mgTracing() // microglia tracing
{
	ImageType::IndexType index;

	if( current_mg_cell == 1 )
	{
		//remove seeds in soma region
		PointList3D seeds_temp;
		for( int i = 0; i < IM->SeedPt.NP; i++ )
		{
			index[0] = IM->SeedPt.Pt[i].x;
			index[1] = IM->SeedPt.Pt[i].y;
			index[2] = IM->SeedPt.Pt[i].z;
			if(IM->IMask->GetPixel(index) == 0)
				seeds_temp.AddPt(IM->SeedPt.Pt[i]);
		}

		//create a folder for storing small images and swc files
		image_folder = file;
		image_folder.replace(QString(".tif"), QString(""));
		image_folder.replace(QString(".tif"), QString(""));
		QString cmd_create_folder = image_folder;
		cmd_create_folder.prepend("mkdir ");
		cmd_create_folder.replace(QString("/"), QString("\\"));
		system(cmd_create_folder.toStdString().c_str());

		IM->SeedPt = seeds_temp;
		/*IM->SeedPt_mg = seeds_temp;
		IM->visit_label.set_size(IM->SeedPt.GetSize());
		IM->visit_label.fill(0);*/

		//assign seeds to seed sets
		for( int i = 0; i < IM->num_soma; i++ )
		{
			PointList3D temp;
			IM->SeedSets.push_back(temp);
		}

		for( int i = 0; i < IM->SeedPt.NP; i++ )
		{
			index[0] = IM->SeedPt.Pt[i].x;
			index[1] = IM->SeedPt.Pt[i].y;
			index[2] = IM->SeedPt.Pt[i].z;

			int cell_id = IM->IVoronoi->GetPixel(index);
			IM->SeedSets[cell_id-1].AddPt(IM->SeedPt.Pt[i]); 
		} 
	}

	//choose the seed set for current microglia tree
	std::cout<<"choose current seed set"<<std::endl;

	IM->SeedPt.RemoveAllPts();

	IM->SeedPt = IM->SeedSets[current_mg_cell-1];
	/*for( int i = 0; i < IM->SeedPt_mg.NP; i++ )
	{
	index[0] = IM->SeedPt_mg.Pt[i].x;
	index[1] = IM->SeedPt_mg.Pt[i].y;
	index[2] = IM->SeedPt_mg.Pt[i].z;
	if(IM->IVoronoi->GetPixel(index) == current_mg_cell)
	IM->SeedPt.AddPt(IM->SeedPt_mg.Pt[i]);
	} */
	IM->visit_label.set_size(IM->SeedPt.GetSize());
	IM->visit_label.fill(0);

	Progress->setMaximum(IM->SeedPt.GetSize()); //change the maximum value of the progress bar

	std::cout<<"#seed points:"<<IM->SeedPt.NP<<std::endl;
	std::cout<<"current_mg_cell:"<<current_mg_cell<<std::endl;
}

void QtTracer::autoTracing() //test how seed points affect the tracing performance
{

	if( IM->SeedPt.NP <= 1 ) 
		QMessageBox::information(this, tr("Progress"), tr("Seed Testing Finished"));
	else
	{
		Process_Stage = 1;
		Process();
	}
}

void QtTracer::batchTracingI()
{
	current_idx++;
	if( current_idx == 0 )
		start_time_batch_processI = 0;

	if( (unsigned)current_idx < images_dir.count() )
	{
		//do tracing for each image
		//load image first
		//if using outside preprocessor, don't need to load the image first
		if( !outside_preprocess )
			loadImage();
		//Processing
		Process_Stage = 1;
		Process();
	}
	else
	{
		clock_t end_time = clock(); 
		float elapsed_time = (float)(end_time - start_time_batch_processII)/CLOCKS_PER_SEC;
		elapsed_time /= 60;

		std::cout<<"Time Elapsed Time for Batch Processing:"<<elapsed_time<<" mins"<<std::endl;
		QMessageBox::information(this, tr("Progress"), tr("Batch Processing Finished"));
	}
}

void QtTracer::batchPreprocessing()
{
	batch_preprocessing = true;
	QString dir_path = QFileDialog::getExistingDirectory(this, tr("Open Image Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
	QStringList filters;
	//foreach (QByteArray format, QImageReader::supportedImageFormats())
	//filters += "*." + format;
	filters << "*.tiff" << "*.tif";
	//filters << "*_CV_SV2D.tif";

	images_dir.setPath(dir_path);
	images_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
	images_dir.setNameFilters(filters);

	string_array = new QString[images_dir.count()];
	for( unsigned int i = 0; i < images_dir.count(); i++ )
	{
		string_array[i] = images_dir[i];
	}

	for( current_idx = 0; current_idx < images_dir.count(); current_idx++ )
	{
		curvelet_scalar_voting();
	}
}

void QtTracer::batchProcessingII()
{
	if( !batch_processII )
	{

		//int NSpace = 3000;
		//GSnakeList.SetNSpace(NSpace);
		//for( int i = 0; i < NSpace; i++ )
		//{
		//	GSnakeList.Snakes[i].Cu.SetN(NSpace);
		//}

		batch_processII = true;

		general_para12->setVisible(true);
		general_para2->setVisible(true);
		general_para3->setVisible(true);

		//specify the image folder
		QString dir_path = QFileDialog::getExistingDirectory(this, tr("Open Image Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		QStringList filters;
		//foreach (QByteArray format, QImageReader::supportedImageFormats())
		//filters += "*." + format;
		filters << "*.tiff" << "*.tif";
		//filters << "*_CV_SV2D.tif";

		images_dir.setPath(dir_path);
		images_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
		images_dir.setNameFilters(filters);

		this->statusBar()->showMessage(dir_path);

		//selected a directory containing no images
		if( images_dir.count() == 0){
			QMessageBox::warning(this, tr("Warning"),
				tr("No image exists in this directory"));
			batch_processII = false;
			general_para12->setVisible(false);
			general_para2->setVisible(false);
			general_para3->setVisible(false);
			nextButton->setVisible(true);
			backButton->setVisible(true);
			return;

		}
		//specify the output swc folder
		swcs_path = QFileDialog::getExistingDirectory(this, tr("Select Output Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		/*bool joint_xmls = false;

		if( joint_xmls )
		{
		//load xml file
		file = QFileDialog::getOpenFileName(this , "Load Joint Transforms XML", ".",
		tr("XML ( *.xml )"));
		this->statusBar()->showMessage("filename:\t" + file);
		}
		else
		{
		QString dir_path1 = QFileDialog::getExistingDirectory(this, tr("Open XMLs Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);
		QStringList filters1;
		filters1 << "*.xml";
		xmls_dir.setPath(dir_path1);
		xmls_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
		xmls_dir.setNameFilters(filters1);
		} */

		if( 1 )
		{
			//std::string file_name = file.toStdString();
			//const char *file_name_ITK = file_name.c_str();

			string_array = new QString[images_dir.count()];
			std::string *std_string_array = new std::string[images_dir.count()]; 

			for( unsigned int i = 0; i < images_dir.count(); i++ )
		 {
			 string_array[i] = images_dir[i];
			 std_string_array[i] = images_dir[i].toStdString();
			 std::cout<<std_string_array[i]<<std::endl;
		 }

			reg.set_n(images_dir.count(),std_string_array);

			/*//read xmls
			if( joint_xmls )
			{
			std::string file_name = file.toStdString();
			reg.read_xml(file_name);
			}
			else
			{   
			for( unsigned int i = 0; i < xmls_dir.count(); i++ )
		 {
		 std::string file_name = xmls_dir.absoluteFilePath(xmls_dir[i]).toStdString();
		 reg.read_xml(file_name); 
		 }
		 }*/

			//load coordinates (for checking adjacency)
			txt_fileName = QFileDialog::getOpenFileName(this, tr("Load Coordinates"),
				".", tr("Text File (*.txt)"));
			QFile txt_file(txt_fileName);
			if (!txt_file.open(QIODevice::ReadOnly)) 
			{
				QMessageBox::information(this, tr("Warning"), tr("Cannot load this file"));
				return;
			}
			else
			{ 
				coordinates.RemoveAllPts();
				QTextStream in(&txt_file);

				//skip the first three lines
				QString line = in.readLine();
				QStringList fields = line.split(" ",QString::SkipEmptyParts);
				line = in.readLine();
				line = in.readLine();

				for( unsigned int i = 0; i < images_dir.count(); i++ )
				{
					line = in.readLine();
					fields = line.split(" ",QString::SkipEmptyParts);
					coordinates.AddPt(fields.at(1).toInt(), fields.at(2).toInt(),0);
				}

				set_transform_dialog->show();

				//nextButton->setVisible(false);
				backButton->setVisible(false);
				startButtonI->setVisible(false);
				startButtonII->setVisible(true);

				int index_tab1 = imageTab->indexOf(scrollArea_MT);
				imageTab->setTabEnabled(index_tab1,true);

		 }
		}
	}
	else
	{ 
		batch_processII = false;
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		general_para3->setVisible(false);
		nextButton->setVisible(true);
		backButton->setVisible(true);
	}
}

void QtTracer::generateGraph()
{
	std::cout<<"Generating the tile graph..."<<std::endl;
	reg.set_coordinates(coordinates, set_transform_dialog->getWidth(), set_transform_dialog->getHeight());
	//calculate the transforms using coordinates
	reg.calculate_transform();
	reg.set_root(set_transform_dialog->getRoot()-1);

	createMontage();
}

void QtTracer::createMontage()
{
	std::cout<<"Creating montage image..."<<std::endl;
	this->statusBar()->showMessage("Creating montage image...");

	int sh = 5;
	int line_width = 5;
	min_x = 10000; //for dealing with negative x coordinates 
	float max_x = 0;
	float max_y = 0;
	for( int i = 0; i < coordinates.GetSize(); i++ )
	{
		if( coordinates.Pt[i].x > max_x )
			max_x = coordinates.Pt[i].x;
		if( coordinates.Pt[i].y > max_y )
			max_y = coordinates.Pt[i].y;
		if( coordinates.Pt[i].x < min_x )
			min_x = coordinates.Pt[i].x;
	}

	max_x = max_x + set_transform_dialog->getWidth();
	max_y = max_y + set_transform_dialog->getHeight();
	max_x = (max_x - min_x)/general_para->getSf();
	max_y = max_y/general_para->getSf();

	QImage background(max_x,max_y,QImage::Format_RGB32);
	Background = background;
	QPainter painter(&Background);
	painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

	//display 2D montage
	for( unsigned int i = 0; i < images_dir.count(); i++ )
	{
		file = images_dir.absoluteFilePath(images_dir[i]);
		if (!file.isEmpty())
		{
			if( reg.TT[i].level == 0 )
				continue;
			std::string file_name = file.toStdString();
			const char *file_name_ITK = file_name.c_str();
			IM->ImRead_NoSmooth(file_name_ITK, general_para->getRemoveSlice());

			QImage image = convertITK2QT(IM->ImMaxProjection(IM->I), false);

			QPointF POS;

			//POS.setX(coordinates.Pt[i].x);
			//POS.setY(coordinates.Pt[i].y);
			POS.setX((coordinates.Pt[set_transform_dialog->getRoot()-1].x + reg.TT[i].T[0] - min_x)/general_para->getSf());
			POS.setY((coordinates.Pt[set_transform_dialog->getRoot()-1].y + reg.TT[i].T[1])/general_para->getSf());
			painter.drawPixmap(POS,QPixmap::fromImage(image));
			std::cout<<coordinates.Pt[set_transform_dialog->getRoot()-1].x + reg.TT[i].T[0]<<","<<coordinates.Pt[set_transform_dialog->getRoot()-1].x + reg.TT[i].T[1]<<std::endl;

			//draw 3D volume renderings
			//drawVolume((coordinates.Pt[i].x - min_x)/general_para->getSf(), (coordinates.Pt[i].y)/general_para->getSf(), 0);
		}
	}

	/*//draw the tile graph
	int tile_left = reg.N;
	int lvl = 1;
	while( tile_left )
	{
	std::vector<int> left_index;
	for( int i = 0; i < reg.N; i++ )
	{ 
	if( reg.TT[i].level == lvl && reg.TT[i].visited == 0 )
	{
	left_index.push_back(i);
	}
	}

	if( left_index.size() == 0 )
	{
	lvl++;
	}
	else
	{
	int idx = left_index[0];

	reg.TT[idx].visited = 1;
	//check if current level runs out of tiles
	bool next_level = true;
	for( int j = 0; j < reg.N; j++ )
	{ 
	if( reg.TT[j].level == lvl && reg.TT[j].visited == 0 )
	{
	next_level = false;
	}
	}
	if( next_level )
	lvl++;

	QPen pen1(Qt::yellow, 10); 
	painter.setPen(pen1);
	//draw connection from this tile
	for( unsigned int j = 0; j < images_dir.count(); j++ )
	{
	if( reg.TG[idx*reg.N + j].overlap > 0.03)
	{
	painter.drawLine((coordinates.Pt[idx].x - min_x + reg.SM/2)/general_para->getSf(), (coordinates.Pt[idx].y + reg.SN/2)/general_para->getSf(),
	(coordinates.Pt[j].x - min_x + reg.SM/2)/general_para->getSf(), (coordinates.Pt[j].y + reg.SN/2)/general_para->getSf());
	int diameter = 50;
	painter.fillRect(QRect((coordinates.Pt[idx].x - min_x + reg.SM/2)/general_para->getSf()-diameter / 2.0,(coordinates.Pt[idx].y + reg.SN/2)/general_para->getSf()-diameter / 2.0,
	diameter, diameter),Qt::red);
	}
	}

	}

	tile_left = 0;
	for( int i = 0; i < reg.N; i++ )
	{ 
	if( reg.TT[i].visited == 0 )
	{
	tile_left++;
	}
	}
	}

	for( int i = 0; i < reg.N; i++)
	{
	reg.TT[i].visited = 0;
	} */

	for( unsigned int i = 0; i < images_dir.count(); i++ )
	{
		if( reg.TT[i].level == 0 ) //remove detached tiles
			continue;

		QPointF POS;
		//POS.setX((coordinates.Pt[set_transform_dialog->getRoot()-1].x + reg.TT[i].T[0])/general_para->getSf());
		//POS.setY((coordinates.Pt[set_transform_dialog->getRoot()-1].y + reg.TT[i].T[1])/general_para->getSf());
		POS.setX((coordinates.Pt[i].x - min_x)/general_para->getSf());
		POS.setY((coordinates.Pt[i].y)/general_para->getSf());

		QPen pen0(Qt::blue, line_width);
		painter.setPen(pen0); 
		painter.drawRect(POS.x(), POS.y(), set_transform_dialog->getWidth()/general_para->getSf(), set_transform_dialog->getHeight()/general_para->getSf());

		QString text;
		if( i == set_transform_dialog->getRoot() - 1 )
		{
			text.append("Root:");
		}

		text.append(reg.string_array[i].c_str());

		std::cout<<"reg.string_array"<<" "<<i+1<<reg.string_array[i]<<std::endl;
		//for( int ij = 0; ij < 6; ij ++ )
		// text.append(reg.string_array[i].c_str()[ij]);
		QFont fp = painter.font();
		fp.setBold(true);
		fp.setPointSize(25);
		painter.setFont(fp);
		painter.setPen(Qt::white); 
		painter.drawText(POS, text);
	}

	//std::cout<<"check point 1"<<std::endl;

	QSize size0 = general_para->getSf() * Background.size();
	Background = Background.scaled(size0,Qt::KeepAspectRatio);

	QSize size = (float)1/(float)sh * Background.size();
	QImage display_image = Background.scaled(size, Qt::KeepAspectRatio);
	montageLabel->setPixmap(QPixmap::fromImage(display_image));

	montage_created = true;
	this->statusBar()->showMessage("Montage created...");

	//std::cout<<"check point 2"<<std::endl;
}

void QtTracer::batchTracingII()
{
	int tile_left = 0;
	for( int i = 0; i < reg.N; i++ )
	{ 
		if( reg.TT[i].visited == 0 )
		{
			tile_left++;
		}
	}

	if( tile_left == reg.N )
	{
		//this->Renderer->RemoveAllViewProps(); //clean the volume renderings

		start_time_batch_processII = clock(); 
	}

	std::cout<<"--------------------------Tiles Left:"<<tile_left<<"--------------------------"<<std::endl;

	if( tile_left )
	{
		std::vector<int> left_index;
		for( int i = 0; i < reg.N; i++ )
		{ 
			if( reg.TT[i].level == current_lvl && reg.TT[i].visited == 0 )
		 {
			 left_index.push_back(i);
		 }
		}

		if( left_index.size() == 0 )
		{
			current_lvl++;
		}
		else
		{
			current_idx = left_index[0];
			reg.TT[current_idx].visited = 1;
			//check if current level runs out of tiles
			bool next_level = true;
			for( int j = 0; j < reg.N; j++ )
			{ 
				if( reg.TT[j].level == current_lvl && reg.TT[j].visited == 0 )
				{
					next_level = false;
				}
			}
			if( next_level )
				current_lvl++;

			if( !outside_preprocess )
				loadImage();

			Process_Stage = 1;
			Process();
		}
	}
	else
	{
		/*GSnakeList_BP.SetNSpace(GSnakeList.NSnakes);
		for( int i = 0; i < GSnakeList.NSnakes; i++ )
		{
		GSnakeList_BP.Snakes[i].Cu.SetN(5000);
		GSnakeList_BP.Snakes[i].Cu = GSnakeList.Snakes[i].Cu;
		//GSnakeList.Snakes[i].Cu = GSnakeList_BP.Snakes[i].Cu;
		}*/

		std::cout<<"Total Number of Snakes:"<<GSnakeList.NSnakes<<std::endl;
		this->statusBar()->clearMessage();
		QString MSG("Total Number of Snakes: ");
		QString MSG_Num;
		MSG_Num.setNum(GSnakeList.NSnakes);
		MSG.append(MSG_Num);
		this->statusBar()->showMessage(MSG);

		bpII_finished = true;

		if( !montage_created )
		{
			createMontage();
		}

		tracingViewer->setMontageView(true); //in montage view, use another snake selection method
		tracingViewer->setDisplayImage(Background);
		tracingViewer->setImage(IM);

		int num_pt = 0;
		for( int k = 0; k < GSnakeList.NSnakes; k++ )
		{
			if(GSnakeList.valid_list[k] == 1)
				num_pt += GSnakeList.Snakes[k].Cu.NP;
			//std::cout<<k<<"th of "<<GSnakeList.NSnakes<<"Snakes:"<<GSnakeList.Snakes[k].Cu.NP<<std::endl;
		}

		std::cout<<"Num of Points:"<<num_pt<<std::endl;

		tracingViewer->setSnakes(&GSnakeList);
		tracingViewer->SnakesChanged();

		//draw3DTraces_Global();

		QMessageBox::information(this, tr("Progress"), tr("Batch Processing Finished"));
	}
}

int QtTracer::estimateZShift()
{
	Point3D temp_pt;

	/*int Zmin = 10000;
	int Zmax = 0;
	for( int i = 0; i < seed_snakes.NSnakes; i++ )
	{
	for( int j = 0; j < seed_snakes.Snakes[i].Cu.NP; j++ )
	{
	if( seed_snakes.Snakes[i].Cu.Pt[j].z > Zmax )
	Zmax = seed_snakes.Snakes[i].Cu.Pt[j].z;
	if( seed_snakes.Snakes[i].Cu.Pt[j].z < Zmin )
	Zmin = seed_snakes.Snakes[i].Cu.Pt[j].z;
	}
	}*/

	int ZS = 200;
	vnl_vector<float> score( 2 * ZS + 1 );
	score.fill(0);
	int k = 0;
	for( int z = -ZS; z < ZS; z++ )
	{
		for( int i = 0; i < seed_snakes.NSnakes; i++ )
		{
			for( int j = 0; j < seed_snakes.Snakes[i].Cu.NP; j++ )
			{
				temp_pt.x = seed_snakes.Snakes[i].Cu.Pt[j].x;
				temp_pt.y = seed_snakes.Snakes[i].Cu.Pt[j].y; 
				temp_pt.z = seed_snakes.Snakes[i].Cu.Pt[j].z + z;
				bool out_of_bound = temp_pt.check_out_of_range_3D(IM->SM, IM->SN, IM->SZ);
				if( out_of_bound )
				{
					continue;
				}
				ProbImageType::IndexType index;
				index[0] = temp_pt.x;
				index[1] = temp_pt.y;
				index[2] = temp_pt.z;
				//std::cout<<IM->SN<<", "<<index[2]<<std::endl;

				score(k) += IM->I->GetPixel(index);

			}
		}
		k++;
	}
	int Z_star = -ZS + score.arg_max();
	std::cout<<"Estimated Z Shift:"<<Z_star<<std::endl;

	for( int i = 0; i < seed_snakes.NSnakes; i++ )
	{
		for( int j = 0; j < seed_snakes.Snakes[i].Cu.NP; j++ )
		{
			seed_snakes.Snakes[i].Cu.Pt[j].z += Z_star;
		}
	}
	return Z_star;
}

void QtTracer::addGlobalSnake()
{
	int NP_Per_Tile = 60; //maximum number of points of one trace in one tile(for NM4 only)

	int connection_ratio = 5;
	//int Z_shift = 200;


	SnakeClass temp_snake;

	std::cout<<"check point addglobalsnake 1"<<std::endl;

	std::vector<int> merging_label;
	merging_label.clear();

	for( int i = 0; i < GSnakeList.NSnakes; i++ )
	{
		merging_label.push_back(0);
	}

	//resample seed snakes
	/*for( int i = 0; i < reg.Seed_Snakes.NSnakes; j++ )
	{
	reg.Seed_Snakes.Snakes[i].Cu.curveinterp_3D((float)general_para2->getPtDistance());
	}*/

	//optimize Z translation value
	std::cout<<"optimize Z translation value"<<std::endl;

	SnakeListClass temp_snake_list;

	for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
	{
		if( Tracer->SnakeList.valid_list[i] == 0 )
			continue;

		temp_snake = Tracer->SnakeList.Snakes[i];
		temp_snake_list.AddSnake( reg.convert_global_snake( temp_snake, current_idx, general_para->getSh() * general_para->getSf() ) );
	}

	PointList3D temp_cu;

	//Z optimization based on overlapping cues
	/*vnl_vector<float> weight( 2 * Z_shift + 1 );
	weight.fill(0);
	for( int shift = -Z_shift; shift <= Z_shift; shift++ )
	{
	for( int i = 0; i < temp_snake_list.NSnakes; i++ )
	{
	temp_cu.RemoveAllPts();
	temp_cu = temp_snake_list.Snakes[i].Cu;
	for( int ij = 0; ij < temp_cu.NP; ij++ )
	{
	temp_cu.Pt[ij].z += shift;
	}
	//find overlapping region with seed snakes/eliminated repeating part
	vnl_vector<float> overlapping(reg.Seed_Snakes.NSnakes);
	for( int j = 0; j < reg.Seed_Snakes.NSnakes; j++ )
	{
	overlapping(j) = 0;
	for( int k = 0; k < reg.Seed_Snakes.Snakes[j].Cu.NP; k++ )
	{
	bool overlap = false;
	for( int z = 0; z < temp_cu.NP; z++ )
	{
	if( reg.Seed_Snakes.Snakes[j].Cu.Pt[k].GetDistTo( temp_cu.Pt[z] ) <= general_para3->getConnection() * connection_ratio )
	{
	overlap = true;
	break;
	}
	}
	if( overlap )
	overlapping(j)++;
	}
	//overlapping(j) /= reg.Seed_Snakes.Snakes[j].Cu.NP;
	}  
	weight(shift+Z_shift) += overlapping.max_value();
	}  	
	} 
	//find the Z shift with largest weight and translate the snakes
	int ZS = weight.arg_max() - Z_shift;
	std::cout<<"shift in Z dimension:"<<ZS<<std::endl;
	for( int i = 0; i < temp_snake_list.NSnakes; i++ )
	{
	for( int j = 0; j < temp_snake_list.Snakes[i].Cu.NP; j++ )
	{
	temp_snake_list.Snakes[i].Cu.Pt[j].z += ZS;
	}
	}
	*/

	//Z optimization based on tip point distance
	/*PointList3D seed_snake_tips;
	for( int i = 0; i < reg.Seed_Snakes.NSnakes; i++ )
	{
	seed_snake_tips.AddPt(reg.Seed_Snakes.Snakes[i].Cu.GetFirstPt());
	seed_snake_tips.AddPt(reg.Seed_Snakes.Snakes[i].Cu.GetLastPt());
	}

	vnl_vector<float> weight( 2 * Z_shift + 1 );
	weight.fill(0);
	for( int shift = -Z_shift; shift <= Z_shift; shift++ )
	{
	for( int i = 0; i < temp_snake_list.NSnakes; i++ )
	{
	float min_value = 1024;
	temp_cu.RemoveAllPts();
	temp_cu = temp_snake_list.Snakes[i].Cu;
	temp_cu.Pt[0].z += shift;
	temp_cu.Pt[temp_cu.NP-1].z += shift;

	for( int k = 0; k < seed_snake_tips.NP; k++ )
	{
	float dist1 = temp_cu.Pt[0].GetDistTo( seed_snake_tips.Pt[k], 2);
	float dist2 = temp_cu.Pt[temp_cu.NP-1].GetDistTo( seed_snake_tips.Pt[k], 2);
	if( dist1 <= dist2 && dist1 < min_value )
	min_value = dist1;
	else if( dist2 < dist1 && dist2 < min_value )
	min_value = dist2;
	}
	weight(shift+Z_shift) += min_value;
	}  	
	}
	//find the Z shift with smallest weight and translate the snakes
	int ZS;

	if( GSnakeList.NSnakes == 0 )
	ZS = 0;
	else
	ZS = weight.arg_min() - Z_shift; */

	//std::cout<<"shift in Z dimension:"<<ZS<<std::endl;
	for( int i = 0; i < temp_snake_list.NSnakes; i++ )
	{
		for( int j = 0; j < temp_snake_list.Snakes[i].Cu.NP; j++ )
		{
			temp_snake_list.Snakes[i].Cu.Pt[j].z -= Z_shift;
		}
	}

	//shifted volume rendering
	//file = images_dir.absoluteFilePath(string_array[current_idx]);
	//drawVolume((coordinates.Pt[current_idx].x - min_x)/general_para->getSf(), (coordinates.Pt[current_idx].y)/general_para->getSf(), Z_shift);

	//add snakes to global snake list and do automatic linking based on overlapping cues
	for( int i = 0; i < temp_snake_list.NSnakes; i++ )
	{
		if( temp_snake_list.valid_list[i] == 1 )
		{
			//temp_snake = temp_snake_list.GetSnake(i);
			//GSnakeList.AddSnake( reg.convert_global_snake( temp_snake, current_idx, general_para->getSh() * general_para->getSf() ) );

			GSnakeList.AddSnake( temp_snake_list.Snakes[i] );
			GSnakeList.Snakes[GSnakeList.NSnakes-1].BranchPt.RemoveAllPts();

			merging_label.push_back(0);

			//find overlapping region with seed snakes/eliminated repeating part
			vnl_vector<float> overlapping(reg.Seed_Snakes.NSnakes);
			vnl_vector<float> dist_to_seed_snake(reg.Seed_Snakes.NSnakes);

			for( int j = 0; j < reg.Seed_Snakes.NSnakes; j++ )
			{
				vnl_vector<float> tip_dist(4);
				tip_dist(0) = GSnakeList.Snakes[GSnakeList.NSnakes-1].Cu.GetFirstPt().GetDistTo( reg.Seed_Snakes.Snakes[j].Cu.GetFirstPt(), 4);
				tip_dist(1) = GSnakeList.Snakes[GSnakeList.NSnakes-1].Cu.GetFirstPt().GetDistTo( reg.Seed_Snakes.Snakes[j].Cu.GetLastPt(), 4);
				tip_dist(2) = GSnakeList.Snakes[GSnakeList.NSnakes-1].Cu.GetLastPt().GetDistTo( reg.Seed_Snakes.Snakes[j].Cu.GetFirstPt(), 4);
				tip_dist(3) = GSnakeList.Snakes[GSnakeList.NSnakes-1].Cu.GetLastPt().GetDistTo( reg.Seed_Snakes.Snakes[j].Cu.GetLastPt(), 4);

				dist_to_seed_snake(j) = tip_dist.min_value();

				overlapping(j) = 0;
				//std::cout<<"seed snake points:"<<reg.Seed_Snakes.Snakes[j].Cu.NP<<std::endl;
				//std::cout<<"seed snake length:"<<reg.Seed_Snakes.Snakes[j].Cu.GetLength()<<std::endl;
				for( int k = 0; k < reg.Seed_Snakes.Snakes[j].Cu.NP; k++ )
				{
					bool overlap = false;
					for( int z = 0; z < GSnakeList.Snakes[GSnakeList.NSnakes-1].Cu.NP; z++ )
			  {
				  if( reg.Seed_Snakes.Snakes[j].Cu.Pt[k].GetDistTo( GSnakeList.Snakes[GSnakeList.NSnakes-1].Cu.Pt[z] ) <= general_para3->getConnection() * connection_ratio )
				  {
					  overlap = true;
					  break;
				  }
			  }
					if( overlap )
						overlapping(j)++;
				}
				overlapping(j) /= reg.Seed_Snakes.Snakes[j].Cu.NP;
			}

			//resample the new-added snake to avoid overflow
			//GSnakeList.Snakes[GSnakeList.NSnakes-1].Cu.curveinterp_3D(NP_Per_Tile);

			//if( overlapping.max_value() != 0 )
			std::cout<<"overlapping.max_value():"<<overlapping.max_value()<<std::endl;
			if( overlapping.max_value() > 0 )
			{
				int idx = overlapping.arg_max();

				//avoid second merging
				if( merging_label[reg.seed_snake_label[idx]] == 0 && GSnakeList.valid_list[reg.seed_snake_label[idx]] == 1 )
					//if( merging_label[reg.seed_snake_label[idx]] == 0 && GSnakeList.valid_list(reg.seed_snake_label[idx]) == 1 && dist_to_seed_snake(idx) == dist_to_seed_snake.min_value() )
				{
					merging_label[reg.seed_snake_label[idx]] = 1;
					//std::cout<<"overlapping.max_value:"<<overlapping.max_value()<<std::endl;
					GSnakeList.MergeSnake(reg.seed_snake_label[idx], GSnakeList.NSnakes-1, false);
				}
			}
		}
	}

	//std::cout<<"check point addglobalsnake 3"<<std::endl;

	//display the global snake in 3-D view
	//draw3DTraces_Global();

	//display the global snake in montage view
	float zoom = 0.2;
	QSize size = zoom * Background.size();
	QImage display_image = Background.scaled(size, Qt::KeepAspectRatio);
	QPainter painter(&display_image);

	int line_width = 3;

	std::cout<<"Number of Global Snakes:"<<GSnakeList.NSnakes<<std::endl;
	int max_snake_point = 0;
	int min_snake_point = 10000;
	for( int j = 0; j < GSnakeList.NSnakes; j++ )
	{
		if( GSnakeList.valid_list[j] == 0 )
			continue;

		if( GSnakeList.Snakes[j].Cu.NP > max_snake_point )
			max_snake_point = GSnakeList.Snakes[j].Cu.NP;
		if( GSnakeList.Snakes[j].Cu.NP < min_snake_point )
			min_snake_point = GSnakeList.Snakes[j].Cu.NP;
	}

	outputSWC_Raw_MT();

	std::cout<<"Max Number of Snake Points:"<<max_snake_point<<std::endl;
	std::cout<<"Min Number of Snake Points:"<<min_snake_point<<std::endl;


	for( int j = 0; j < GSnakeList.NSnakes; j++ )
	{
		//std::cout<<"jth snake in GSnakeList:"<<j<<std::endl;
		//std::cout<<"valid:"<<GSnakeList.valid_list[j]<<std::endl;
		//std::cout<<"GSnakeList.Snakes[j].Cu:"<<GSnakeList.Snakes[j].Cu.GetSize()<<std::endl;
		//std::cout<<"GSnakeList.Snakes[j].Ru:"<<GSnakeList.Snakes[j].Ru.size()<<std::endl;
		if( GSnakeList.Snakes[j].Cu.NP < 3 )
			GSnakeList.valid_list[j] = 0;
		if( GSnakeList.valid_list[j] == 0 )
			continue;

		QPointF *SP= new QPointF[GSnakeList.Snakes[j].Cu.GetSize()];
		for(int i = 0; i < GSnakeList.Snakes[j].Cu.GetSize(); i++)
		{
			SP[i].setX((GSnakeList.Snakes[j].Cu.GetPt(i).x - min_x) * zoom);
			SP[i].setY(GSnakeList.Snakes[j].Cu.GetPt(i).y * zoom);
		}

		QPen pen0(Qt::red, line_width);
		painter.setPen(pen0); 
		painter.drawPolyline(SP,GSnakeList.Snakes[j].Cu.GetSize());

		//draw head
		int diameter = line_width * 2;
		QPen pen_head(Qt::yellow, diameter);
		painter.setPen(pen_head);
		painter.drawPoint((GSnakeList.Snakes[j].Cu.GetLastPt().x - min_x) * zoom,GSnakeList.Snakes[j].Cu.GetLastPt().y * zoom);

		//draw tail
		painter.setPen(Qt::yellow);
		painter.drawEllipse(QRect((GSnakeList.Snakes[j].Cu.GetFirstPt().x - min_x) * zoom - diameter / 2.0, GSnakeList.Snakes[j].Cu.GetFirstPt().y * zoom - diameter / 2.0,
			diameter, diameter));
	}

	//display the elapsed time
	clock_t end_time = clock(); 
	float elapsed_time = (float)(end_time - start_time_batch_processII)/CLOCKS_PER_SEC;
	elapsed_time /= 60;

	QString text("Elapsed Time: ");
	QString num;
	num.setNum(elapsed_time);
	text.append(num);
	text.append(" mins");
	QFont fp = painter.font();
	fp.setBold(true);
	//fp.setPointSize(30);
	fp.setPointSize(80);
	painter.setFont(fp);
	painter.setPen(Qt::white);
	//painter->setCompositionMode(QPainter::CompositionMode_Overlay);
	//painter.drawText(60,60, text);
	painter.drawText(150,150, text);
	//display the seed snakes
	for( int j = 0; j < reg.Seed_Snakes.NSnakes; j++ )
	{
		if( reg.Seed_Snakes.Snakes[j].Cu.NP < 3 )
			reg.Seed_Snakes.valid_list[j] = 0;
		if( reg.Seed_Snakes.valid_list[j] == 0 )
			continue;

		QPointF *SP= new QPointF[reg.Seed_Snakes.Snakes[j].Cu.GetSize()];
		for(int i = 0; i < reg.Seed_Snakes.Snakes[j].Cu.GetSize(); i++)
		{
			SP[i].setX((reg.Seed_Snakes.Snakes[j].Cu.GetPt(i).x - min_x) * zoom);
			SP[i].setY(reg.Seed_Snakes.Snakes[j].Cu.GetPt(i).y * zoom);
		}

		QPen pen0(Qt::white, line_width);
		painter.setPen(pen0); 
		painter.drawPolyline(SP,reg.Seed_Snakes.Snakes[j].Cu.GetSize());
	}



	int tile_processed = 0;
	for( int i = 0; i < reg.N; i++ )
	{ 
		if( reg.TT[i].visited == 1 )
		{
			tile_processed++;
		}
	}

	QString file_name;
	file_name.setNum(tile_processed);
	file_name.append(".tif");
	display_image.save(file_name);

	//std::cout<<"check point addglobalsnake 4"<<std::endl;
	montageLabel->setPixmap(QPixmap::fromImage(display_image));

	//save current global snake to swc
}


void QtTracer::Process()
{

	switch(Process_Stage)
	{
		//image processing stage
	case 0 :

		general_para3->setVisible(false);
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		general_para->setVisible(true);
		backButton->setEnabled(false);
		nextButton->setEnabled(false);
		break;

	case 1 :

		Preprocess_Stage = 1;
		Progress->setMinimum(0);
		Progress->setMaximum(4);
		Progress->setValue(0);

		nextButton->setEnabled(false);
		backButton->setEnabled(true);
		general_para->setVisible(false);
		general_para12->setVisible(true);
		general_para2->setVisible(false);
		general_para3->setVisible(false);

		//stop the tracing when go from step 2 to step 1
		if( !Tracer->tracing_thread->stopped )
			Tracer->tracing_thread->stop();
		general_para2->enableSetting();
		//clear the tracing when go from step 2 to step 1
		tracingViewer->removeSnakes();

		if( automated_process || batch_processI || batch_processII )
		{
			//if( automated_process && IM->SeedPt.NP != 0 )
			//	nextStep();
			//else
			//{
			Preprocess();  
			nextStep();
			//}
		}

		break;

	case 2 :

		slider->setImageState(1);
		slider->setDisplaySeed(false);

		Progress->setMinimum(0);
		Progress->setMaximum(IM->SeedPt.GetSize());
		Progress->setValue(0);
		Progress->setTextVisible(true);

		nextButton->setEnabled(false);
		general_para->setVisible(false);
		general_para12->setVisible(false);
		general_para2->setVisible(true);
		general_para3->setVisible(false);

		//in case coming from step 3, clear snake tree first
		if( snake_tree )
		{
			clearSnakeTree();
		}
		//in case going from step 3 to step 2, also clear snakes
		//if( Tracer->SnakeList.NSnakes != 0 )
		//{
		//	  Tracer->SnakeList.NSnakes = 0;
		//}
		//start the tracing thread state
		Tracer->tracing_thread->stopped = true;
		tracing_started = false;
		//enable the parameters in case they are disabled
		general_para2->enableSetting();

		if( Old_Process_Stage == 3 )
			nextButton->setEnabled(true);

		if( mg_process )
		{
			mgTracing();
			emit init_the_tracing();
		}

		if( automated_process || batch_processI || batch_processII )
		{
			emit init_the_tracing();
		}

		break;

	case 3:

		general_para->setVisible(false);
		general_para12->setVisible(false);
		general_para2->setVisible(false);
		general_para3->setVisible(true);

		general_para3->output_swc_button->setEnabled(false);

		nextButton->setEnabled(false);

		if( mg_process )
		{
			mgPrune_n_Save(); //prune traces in soma and output the swc file for current microglia cell

			if( current_mg_cell > IM->Centroid.NP )
			{
				mg_process = false;
				std::cout<<"------------------Microglia Tracig Finished------------------"<<std::endl;
			}
			else
			{
				Process_Stage = 2;
				Process();
			}
		}

		if( automated_process )
		{
			automated_process = false;
			//automated_process = true;
			//removeIsolated();
			//outputSWC_Raw();
			//decimateSeeds();
			//autoTracing();
		}
		if( batch_processI )
		{
			//output the tracing result
			outputSWC_Raw();  
			//go to next image
			//batchTracingI();
			emit batch_tracingI_continue();
		}

		if( batch_processII )
		{
			addGlobalSnake();
			outputSWC_Raw();
			//batchTracingII();
			emit batch_tracingII_continue();
		}

		break;

	default:;
	}
}

void QtTracer::nextStep()
{
	Old_Process_Stage = Process_Stage;
	Process_Stage += 1;
	emit start_continue_process();
}
void QtTracer::backStep()
{
	Old_Process_Stage = Process_Stage;
	Process_Stage -= 1;
	emit start_continue_process();
}

void QtTracer::changeSlice(int in)
{
	in--;
	if(slider->State == 1)
	{
		tracingViewer->setDisplayImage(convertITK2QT(IM->extract_one_slice(IM->ImRescale(IM->I),in), false));	
	}
	else if(slider->State == 2)
	{
		tracingViewer->setDisplayImage(convertITK2QT(IM->extract_one_slice(IM->ImRescale(IM->IVessel),in), false));	
	}
	else if(slider->State == 3)
	{
		tracingViewer->setDisplayImage(convertITK2QT(IM->extract_one_slice(IM->ImRescale(IM->IL),in), false));
	}
}

void QtTracer::changeDisplay(int in)
{
	int index_tab = imageTab->indexOf(scrollArea_BW);

	if( imageTab->currentIndex() == index_tab )
	{
		set_cross_section_view(in);
	}
	else
	{
		if(in==-1)
		{
			//tracingViewer->setDisplayImage(convertITK2QT(IM->ImMinProjection(IM->IRGB)));	
			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->IDisplay), false));	
		}
		if(in==1)
		{
			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->ImRescale(IM->I)), false));	
		}
		else if(in==2)
		{
			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->ImRescale(IM->IVessel)), false));	
		}
		else if(in== 3)
		{
			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->ImRescale(IM->IL)), false));
		}
	}
}

void QtTracer::Init_Tracing()
{
	//delete all the actors for showing previous traces
	clearTraces();

	slider->setCheckable(3); //allow to check label image

	//compute the background and foreground models for 4-D region snake
	if( mg_process != true )
	{
		IM->ImComputeInitBackgroundModel(general_para12->getThreshold());
		IM->ImComputeInitForegroundModel();
	}
	else if( current_mg_cell == 1 )
	{
		IM->ImComputeInitBackgroundModel(general_para12->getThreshold());
		IM->ImComputeInitForegroundModel();
	}

	//in case the user hit twice the start tracing button
	std::cout<<"--------------Tracing--------------"<<std::endl;

	nextButton->setEnabled(false); 

	if( tracing_started == true )
		return;

	//if Tracer is still tracing with last setting, wait for tracing to finish
	if( Tracer->tracing && !batch_processI && !batch_processII ) 
	{
		QMessageBox::information(this, tr("Warning"), tr("Please wait for current tracing to finish"));
		return;
	}

	//clear edits
	edits->TP = 0.0;
	edits->FP = 0.0;
	edits->FN = 0.0;
	edits->precision = 0.0;
	edits->recall = 0.0;
	edits->numBranch = 0;
	edits->numSplit = 0;
	edits->numDelete = 0;
	edits->numMerge = 0;
	this->PrecisionLabel->setText(QString::number(edits->precision));
	this->RecallLabel->setText(QString::number(edits->recall));
	this->BranchLabel->setText(QString::number(edits->numBranch));
	this->SplitLabel->setText(QString::number(edits->numSplit));
	this->DeleteLabel->setText(QString::number(edits->numDelete));
	this->MergeLabel->setText(QString::number(edits->numMerge));
	//clear surface rendering actors, if any
	if( mesh_actors.size() != 0 )
	{
		for( int i = 0; i < mesh_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(mesh_actors[i]);
		}
		mesh_actors.clear();
	}

	tracing_started = true;

	seeding = false;

	Progress->setValue(0);

	Start_t = time(NULL);

	tracingViewer->removeSnakes();

	IM->ImRefresh_LabelImage();

	Tracer->SetImage(IM);
	Tracer->use_multi_threads = general_para2->getMultiThreads();
	Tracer->Init();

	tracingViewer->setSnakes(&Tracer->SnakeList);

	Tracer->tracing_thread->stopped = false;

	if( Tracer->tracing_thread->ToSuspend )
	{
		Tracer->tracing_thread->resume();
	}

	//use seed snakes from previous tile for batch processing II
	SnakeClass temp_snake;
	SnakeClass temp_snake1;
	//seed_snakes.NSnakes = 0;
	seed_snakes.RemoveAllSnakes();
	if( batch_processII )
	{
		reg.eliminate_repeating(&GSnakeList, current_idx, 2 * general_para->getSh() * general_para->getSf(), general_para2->getMinimumLength());

		//reg.Seed_Snakes.RemoveAllSnakes();

		if( reg.Seed_Snakes.NSnakes != 0 )
		{
			for( int i = 0; i < reg.Seed_Snakes.NSnakes; i++ )
			{
				temp_snake1 = reg.Seed_Snakes.Snakes[i];
				temp_snake = reg.convert_local_snake(temp_snake1, current_idx, general_para->getSh() * general_para->getSf());
				seed_snakes.AddSnake(temp_snake);
			}

			//seed_snakes.valid_list.set_size(seed_snakes.NSnakes);
			//seed_snakes.valid_list.fill(1);

			//estimate shift in Z dimension
			std::cout<<"Number of Seed Snakes:"<<seed_snakes.NSnakes<<std::endl;

			Z_shift = estimateZShift();

			tracingViewer->setSeedSnakes(&seed_snakes);

			//tracing from seed snakes
			//seeding = true;

			//Seed_Snake_Tracing();
			Tracing();
		}
		else
		{
			Tracing();
		}
	}
	else
	{
		Tracing();
	}
}

void QtTracer::Seed_Snake_Tracing()
{

	if( !seeding )
		return;

	if( Tracer->tracing_thread->stopped )
		Tracer->tracing_thread->stopped = false;

	for( int i = 0; i < seed_snakes.NSnakes; i++ )
	{
		if( seed_snakes.valid_list[i] != 0 )
		{
			m_seed = seed_snakes.Snakes[i].Cu;
			seed_snakes.valid_list[i] = 0;
			emit start_tracing();
			break;
		}
	}
}

void QtTracer::Manual_Seed_Tracing(PointList3D seeds)
{
	//if( Process_Stage == 2 )
	//{
	if( !seeding )
		return;

	//if( !tracing_started && Progress->value() == 0 )
	if( Tracer->SnakeList.NSnakes == 0 && !batch_processII )
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("Please start the tracing first"));
		return;
	}

	if( Tracer->tracing_thread->stopped )
		Tracer->tracing_thread->stopped = false;

	m_seed = seeds;

	Tracing();
	//}
}

void QtTracer::Manual_Seed_TracingI(PointList3D seeds)
{
	//if( Process_Stage == 2 )
	//{
	if( !seeding )
		seeding = true;

	seeds.curveinterp_3D(general_para2->getPtDistance());

	//if( !tracing_started && Progress->value() == 0 )
	if( Tracer->SnakeList.NSnakes == 0 && !batch_processII )
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("Please start the tracing first"));
		return;
	}

	if( Tracer->tracing_thread->stopped )
		Tracer->tracing_thread->stopped = false;

	m_seed = seeds;

	Tracing();
	//}
}

void QtTracer::Tracing()
{ 
	//if( seeding && m_seed.x !=0 && m_seed.y !=0 )
	if( seeding && m_seed.NP >= 1 )
	{
		general_para2->disableSetting();
		Tracer->setParas(general_para2->getPtDistance(), general_para2->getGamma(), 
			general_para2->getStretchRatio(), general_para2->getMinimumLength(), 
			general_para2->getCollisionDist(), general_para2->getRemoveSeedRange(),
			general_para2->getDeformationITER(), general_para2->getAutomaticMerging(),
			general_para2->general_para21->getMaxAngle(),general_para2->getFreezeBody(),
			general_para2->getCurrentForce(), general_para2->getCurrentTracing(),
			false, general_para2->getCurrentCoding(), general_para2->getSigmaRatio(), general_para2->getBorder());
		IM->SetCodingMethod(general_para2->getCurrentCoding());

		//IM->ImRefresh_TracingImage();

		if( m_seed.NP == 1 )
		{
			std::cout<<"manual single seed tracing..."<<std::endl;
		}
		else
		{
			std::cout<<"snake NP:"<<m_seed.NP<<std::endl;
			std::cout<<"manual snake tracing..."<<std::endl;
		}
		for( int i = 0; i < m_seed.NP; i++ )
		{
			m_seed.Pt[i].check_out_of_range_3D(IM->SM,IM->SN,IM->SZ);
		}
		Tracer->Cast_Open_Snake_3D(m_seed, true);  
		m_seed.RemoveAllPts();

	}

	//in case of manual seed tracing after tracing is stopped, need to switch tracing thread to be stopped
	//if( Progress->value() == Progress->maximum() || !tracing_started )
	//{
	//   Tracer->tracing_thread->stopped = true;
	//}

	if( !Tracer->tracing_thread->stopped && !seeding )
	{
		general_para2->disableSetting();
		Tracer->setParas(general_para2->getPtDistance(), general_para2->getGamma(), 
			general_para2->getStretchRatio(), general_para2->getMinimumLength(), 
			general_para2->getCollisionDist(), general_para2->getRemoveSeedRange(),
			general_para2->getDeformationITER(), general_para2->getAutomaticMerging(),
			general_para2->general_para21->getMaxAngle(), general_para2->getFreezeBody(),
			general_para2->getCurrentForce(), general_para2->getCurrentTracing(),
			false,general_para2->getCurrentCoding(),general_para2->getSigmaRatio(), general_para2->getBorder());
		IM->SetCodingMethod(general_para2->getCurrentCoding());
		this->statusBar()->clearMessage();
		QString MSG("Number of Snakes: ");
		QString MSG_Num;
		MSG_Num.setNum(Tracer->SnakeList.NSnakes);
		MSG.append(MSG_Num);
		this->statusBar()->showMessage(MSG);

		//std::cout<<"Tracing................."<<std::endl;

		Tracer->Open_Curve_Snake_Tracing();
	}
}


void QtTracer::SegmentationI()
{
	if( Tracer->SnakeList.NSnakes == 0 )
		return;

	std::cout<<"..........Segmentation/3D Surface Reconstruction.........."<<std::endl;
	PointList3D fast_seeds;
	PointList3D re_sample;
	for(int i = 0; i < Tracer->SnakeList.NSnakes; i++)
	{
		if( Tracer->SnakeList.valid_list[i] == 1 )
		{
			re_sample = Tracer->SnakeList.Snakes[i].Cu;
			re_sample.curveinterp_3D(1);
			fast_seeds.AddPtList(re_sample);
		}
	}

	std::vector<float> score = IM->ImFastMarchingI(fast_seeds);

	//save the scores for plotting
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Scores"),
		"score.txt",
		tr("Text File(*.txt)"));
	QFile file(fileName);

	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream out(&file);
		for( int j = 0; j < score.size(); j++ )
		{
			out<<score[j]<<endl;
		}
	}

	std::cout<<"..........Segmentation/3D Surface Reconstruction Finished.........."<<std::endl;
	surfaceRendering(false);

	/*int j = 0;
	for( int i = 0; i < 72; i += 1 )
	{
	this->Camera->Azimuth(5);

	IM->ImFastMarchingAnimation(i);
	surfaceRendering(false);
	j++;
	QString temp;
	temp.setNum(j);
	temp.append(".png");
	writeRendering(Renderer->GetRenderWindow(), temp.toStdString().c_str());
	} */

	/*for(int i = 1; i < 100; i +=2 )
	{
	IM->ImFastMarchingAnimation(i);
	surfaceRendering(false);
	QString temp;
	temp.setNum(i);
	temp.append(".png");
	writeRendering(Renderer->GetRenderWindow(), temp.toStdString().c_str());
	}*/
}

void QtTracer::SegmentationII()
{
	if( Tracer->SnakeList.NSnakes == 0 )
		return;

	std::cout<<"..........Segmentation/3D Surface Reconstruction.........."<<std::endl;
	int idx = 0;
	for(int i = 0; i < Tracer->SnakeList.NSnakes; i++)
	{
		if( Tracer->SnakeList.valid_list[i] == 1 )
		{
			idx++;
			IM->ImFastMarchingII(Tracer->SnakeList.Snakes[i].Cu, idx);
		}
	}
	std::cout<<"..........Segmentation/3D Surface Reconstruction Finished.........."<<std::endl;
	surfaceRendering(true);
}

void QtTracer::detectSpines()
{
	if( Tracer->SnakeList.NSnakes == 0 )
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("Please trace the dendrite first"));
	}
	else
	{
		std::cout<<"..........Spine Detection.........."<<std::endl;
		if( IM->IDisplay )
		{
			typedef itk::CastImageFilter<ImageType, ImageType> CasterType;
			CasterType::Pointer caster = CasterType::New();
			caster->SetInput(IM->IDisplay);
			caster->Update();
			IM->I = caster->GetOutput();
		}

		PointList3D spine_seeds;
		PointList3D temp;
		int N_sample = 16;
		float r_expand = 2;
		for(int i = 0; i < Tracer->SnakeList.NSnakes; i++)
		{
			if( Tracer->SnakeList.valid_list[i] == 1 )
			{
				spine_seeds.AddPtList(temp);
			}
		}

		IM->SeedPt = spine_seeds;
		IM->visit_label.set_size(IM->SeedPt.GetSize());
		IM->visit_label.fill(0);

		//IM->ImFastMarching_Spine(spine_seeds);
		//surfaceRendering(false);

		tracing_started = true;
		seeding = false;
		Progress->setValue(0);
		Tracer->tracing_thread->stopped = false;

		std::cout<<"NSnakes:"<<Tracer->SnakeList.NSnakes<<std::endl;
		Tracing();

		std::cout<<"..........Spine Detection Finished.........."<<std::endl;

	}
}

void QtTracer::removeIsolated()
{
	std::cout<<"..........Select Isolated Traces.........."<<std::endl;
	std::set<long int> ids;
	for(int i = 0; i < Tracer->SnakeList.NSnakes; i++)
	{
		if( Tracer->SnakeList.valid_list[i] == 1 )
		{
			//if( Tracer->SnakeList.Snakes[i].BranchPt.NP == 0 && Tracer->SnakeList.Snakes[i].RootPt.NP == 0)
			if( Tracer->SnakeList.Snakes[i].BranchPt.NP == 0)
			{
				//Tracer->SnakeList.RemoveSnake(i);
				ids.insert(i);
			}
		}
	}
	selections->select(ids);
	//tracingViewer->SnakesChanged();
	draw3DTraces(); 
}

void QtTracer::breakBranches()
{
	Point3D p;
	std::cout<<"..........Breaking The Branches.........."<<std::endl;
	for(int i = 0; i < Tracer->SnakeList.branch_points.NP; i++)
	{
		p = Tracer->SnakeList.branch_points.Pt[i];
		for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
		{
			if( Tracer->SnakeList.valid_list[j] == 0)
				continue;

			bool found_split = false;
			for( int k = 0; k < Tracer->SnakeList.Snakes[j].Cu.NP; k++ )
			{ 
				float dist = p.GetDistTo(Tracer->SnakeList.Snakes[j].Cu.Pt[k]);
				int length1 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(k,0);
				int length2 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(k,1);
				if( dist <= 1 && length1 >= general_para2->getMinimumLength() && length2 >= general_para2->getMinimumLength() )
				{
					Tracer->SnakeList.SplitSnake(j,k+1);	
					found_split = true;
					break;
				}
			}
			if( found_split )
				break;
		}
	}

	//recompute the features
	if( show_table_scatter )
		displayFeatures();

	std::cout<<"..........Branches Breaked.........."<<std::endl;
}


void QtTracer::refineBranch()
{
	if( Process_Stage == 2 && Tracer->SnakeList.NSnakes != 0 )
	{
		int iter_num = 100;
		Tracer->Refine_Branch_Point();
		tracingViewer->SnakesChanged();
	}
	else
	{
		QMessageBox::warning(this, tr("Warning"),
			tr("Please start the tracing first"));
	}
}

void QtTracer::saveSegmentation()
{
	QString save_file = QFileDialog::getSaveFileName(this, "Save Current Image", ".",
		tr("Image (*.tif )"));
	this->statusBar()->showMessage(save_file);

	if( !save_file.isEmpty() )
	{
		std::string file_name = save_file.toStdString();
		const char *file_name_ITK = file_name.c_str();

		IM->ImWrite_Segmentation(file_name_ITK);
		this->statusBar()->showMessage("Image Saved");
	}

}

void QtTracer::saveSoma()
{
	QString save_file = QFileDialog::getSaveFileName(this, "Save Current Image", ".",
		tr("Image ( *.mhd *.tif )"));
	this->statusBar()->showMessage(save_file);

	if( !save_file.isEmpty() )
	{
		std::string file_name = save_file.toStdString();
		const char *file_name_ITK = file_name.c_str();

		IM->ImWrite_Soma(file_name_ITK);
		this->statusBar()->showMessage("Image Saved");
	}

	//write the centroids
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Seeds to txt file"),
		"seeds.txt",
		tr("Text File(*.txt)"));

	QFile centroid_file(fileName);

	if (centroid_file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream *out_txt;
		out_txt = new QTextStream(&centroid_file);
		for( int i = 0; i < IM->Centroid.GetSize(); i++ )
		{
			*out_txt<<IM->Centroid.Pt[i].x * general_para->getSh()<<" ";
			*out_txt<<IM->Centroid.Pt[i].y * general_para->getSh()<<" ";
			*out_txt<<IM->Centroid.Pt[i].z;

			if( i != IM->Centroid.GetSize() - 1 )
				*out_txt<<"\n";
		}
	}

}

void QtTracer::removeSoma()
{
	if( tracingViewer->getContour().NP != 0 && !IM->IMask)
	{
	}
	else
	{
		if( !IM->IMask )
		{
			QString file_mask = QFileDialog::getOpenFileName(this, "Load Soma Mask Image", ".",
				tr("Mask Image ( *.tiff *.tif )"));
			this->statusBar()->showMessage("filename:\t" + file_mask);
			std::cout<<"Image:"<< file.toStdString()<<std::endl;

			if (!file_mask.isEmpty())
			{
				std::string file_name = file_mask.toStdString();
				const char *file_name_ITK = file_name.c_str();

				IM->ImMasking(file_name_ITK, general_para->getSh());
				tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
				this->statusBar()->showMessage("Image Masked");
				//nextButton->setEnabled(true);
				slider->setImageState(1);
			}
			else
			{
				return;
			}
		}
		else
		{
			std::cout<<"..........Masking.........."<<std::endl;
			IM->ImMasking(general_para->getSh());
			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
			this->statusBar()->showMessage("Image Masked");
			slider->setImageState(1);
			std::cout<<"..........Masking Finished.........."<<std::endl;

			//enable the mg processing mode
			mg_process = true;
			current_mg_cell = 1;

			//3D Surface Rendering
			if( mesh_actors.size() != 0 )
			{
				for( int i = 0; i < mesh_actors.size(); i++ )
				{
					this->Renderer->RemoveActor(mesh_actors[i]);
				}
				mesh_actors.clear();
			}

		}

		//3D Surface Rendering
		if( soma_mesh_actors.size() != 0 )
		{
			for( int i = 0; i < soma_mesh_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(soma_mesh_actors[i]);
			}
			soma_mesh_actors.clear();
		}

		typedef itk::ConnectedComponentImageFilter< ImageType, itk::Image<short int, 3> > ConnectedComponentType;
		ConnectedComponentType::Pointer connectedComponentFilter = ConnectedComponentType::New();
		connectedComponentFilter->SetInput( IM->IMask );
		connectedComponentFilter->Update();


		typedef itk::CastImageFilter<LabelImageType, itk::Image<short int, 3> > CasterType;
		CasterType::Pointer caster = CasterType::New();
		caster->SetInput(IM->ISoma);
		caster->Update();

		std::vector<vtkSmartPointer<vtkPolyData> > aa = getVTKPolyDataPrecise(caster->GetOutput());
		//std::cout<<"check point 2"<<std::endl;

		vnl_matrix<double> temp_color(IM->num_soma,3);
		soma_color = temp_color;

		for( int i = 0; i <aa.size(); i++ )
		{
			vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
			mapper->SetInput(aa[i]);
			mapper->ImmediateModeRenderingOff();
			mapper->Update();

			vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
			actor->SetMapper(mapper);

			double color[3];
			color[0] = (double)(rand()%10)/(double)10;
			color[1] = (double)(rand()%10)/(double)10;
			color[2] = (double)(rand()%10)/(double)10;

			soma_color(i,0) = color[0];
			soma_color(i,1) = color[1];
			soma_color(i,2) = color[2];

			actor->GetProperty()->SetColor(soma_color(i,0), soma_color(i,1), soma_color(i,2));
			/*if( i%6 == 0 )
			actor->GetProperty()->SetColor(1,0,0);
			else if( i%6 == 1 )
			actor->GetProperty()->SetColor(0,1,0);
			else if( i%6 == 2 )
			actor->GetProperty()->SetColor(0,0,1);
			else if( i%6 == 3 )
			actor->GetProperty()->SetColor(1,1,0);
			else if( i%6 == 4 )
			actor->GetProperty()->SetColor(1,0,1);
			else if( i%6 == 5 )
			actor->GetProperty()->SetColor(0,1,1);*/

			this->Renderer->AddActor(actor);

			soma_mesh_actors.push_back(actor);
		}
		this->QVTK->GetRenderWindow()->Render();
	}
}

void QtTracer::loadSomaSeeds()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Soma Seeds file"),
		".", tr("Text File (*.txt)"));

	if(!fileName.isEmpty())
	{
		std::cout<< "Load Centroids Table"<<endl;
		vtkSmartPointer<vtkTable> centroidsTable = ftk::LoadXYZTable( fileName.toStdString());

		picked_pts.RemoveAllPts();
		for( vtkIdType i = 0; i < centroidsTable->GetNumberOfRows(); i++)
		{
			float x = centroidsTable->GetValue( i, 0).ToDouble();
			float y = centroidsTable->GetValue( i, 1).ToDouble();
			float z = centroidsTable->GetValue( i, 2).ToDouble();
			picked_pts.AddPt(x, y, z);
		}

		std::cout<<"Seeds Loaded:"<<picked_pts.NP<<std::endl;
	}
}

void QtTracer::pickSomaSeeds()
{
	soma_seeding = true;
}

void QtTracer::segmentSoma()
{
	if( !IM->I )
		return;

	soma_seeding = false;

	if( picked_pts.NP != 0 )
	{  
		std::cout<<"..........Segment Soma.........."<<std::endl;
		IM->ImFastMarching_Soma(picked_pts);
		std::cout<<"..........Segment Soma Finished.........."<<std::endl;

		surfaceRendering(false, IM->IMask);

		//remove the picked points and spheres
		vtk_removePoint();
		picked_pts.RemoveAllPts();
	}
	else
	{
		PointList3D soma_seeds = IM->ImSomaCentroidExtraction();
		std::cout<<"#Detected Soma Seeds"<<soma_seeds.NP<<std::endl;
		std::cout<<"..........Segment Soma.........."<<std::endl;
		IM->ImFastMarching_Soma(soma_seeds);
		std::cout<<"..........Segment Soma Finished.........."<<std::endl;

		surfaceRendering(false, IM->IMask);

		//remove the picked points and spheres
		vtk_removePoint();
		picked_pts.RemoveAllPts();
	}


	/*//Masking
	std::cout<<"..........Masking.........."<<std::endl;
	IM->ImMasking(general_para->getSh());
	std::cout<<"..........Masking Finished.........."<<std::endl;*/

	//3D Surface Rendering
	/* if( soma_mesh_actors.size() != 0 )
	{
	for( int i = 0; i < soma_mesh_actors.size(); i++ )
	{
	this->Renderer->RemoveActor(soma_mesh_actors[i]);
	}
	soma_mesh_actors.clear();
	}

	typedef itk::ConnectedComponentImageFilter< ImageType, itk::Image<short int, 3> > ConnectedComponentType;
	ConnectedComponentType::Pointer connectedComponentFilter = ConnectedComponentType::New();
	connectedComponentFilter->SetInput( IM->IMask );
	connectedComponentFilter->Update();


	typedef itk::CastImageFilter<LabelImageType, itk::Image<short int, 3>> CasterType;
	CasterType::Pointer caster = CasterType::New();
	caster->SetInput(IM->ISoma);
	caster->Update();

	std::vector<vtkSmartPointer<vtkPolyData>> aa = getVTKPolyDataPrecise(caster->GetOutput());
	//std::cout<<"check point 2"<<std::endl;

	vnl_matrix<double> temp_color(IM->num_soma,3);
	soma_color = temp_color;

	for( int i = 0; i <aa.size(); i++ )
	{
	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInput(aa[i]);
	mapper->ImmediateModeRenderingOff();
	mapper->Update();

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	double color[3];
	color[0] = (double)(rand()%10)/(double)10;
	color[1] = (double)(rand()%10)/(double)10;
	color[2] = (double)(rand()%10)/(double)10;

	soma_color(i,0) = color[0];
	soma_color(i,1) = color[1];
	soma_color(i,2) = color[2];

	actor->GetProperty()->SetColor(soma_color(i,0), soma_color(i,1), soma_color(i,2));

	this->Renderer->AddActor(actor);

	soma_mesh_actors.push_back(actor);
	}
	this->QVTK->GetRenderWindow()->Render();
	} */
}

void QtTracer::loadSoma()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Soma file"),
		".", tr("Meta Image (*.mhd)"));

	if(!fileName.isEmpty())
	{
		IM->ImReadSoma(fileName.toStdString().c_str());
		//std::cout<<"surfaceRendering"<<std::endl;
		surfaceRendering(false, IM->IMask);
		std::cout<<"remove points"<<std::endl;
		vtk_removePoint();
		picked_pts.RemoveAllPts();
	}
}

void QtTracer::clearSegmentation()
{
	if( !IM->IMask )
		return;

	//3D Surface Rendering
	if( mesh_actors.size() != 0 )
	{
		for( int i = 0; i < mesh_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(mesh_actors[i]);
		}
		mesh_actors.clear();
	}

	this->QVTK->GetRenderWindow()->Render();

	//clean IMask
	IM->clear_IMask();
}

void QtTracer::saveImage()
{
	QString save_file = QFileDialog::getSaveFileName(this, "Save Current Image", ".",
		tr("Image ( *.mhd *.tif )"));
	this->statusBar()->showMessage(save_file);

	if( !save_file.isEmpty() )
	{
		std::string file_name = save_file.toStdString();
		const char *file_name_ITK = file_name.c_str();

		IM->ImWrite(file_name_ITK, slider->State);
		this->statusBar()->showMessage("Image Saved");
	}
}

void QtTracer::outsidePreprocess()
{
	if( !outside_preprocess )
	{  outside_preprocessor_path = QFileDialog::getExistingDirectory(this, tr("Select Folder Containing The Outside Preprocessor"),
	"/home",
	QFileDialog::ShowDirsOnly
	| QFileDialog::DontResolveSymlinks);
	outside_preprocess = true;
	this->statusBar()->showMessage("Curvelets + Scalar Voting Enabled");
	}
	else
	{
		outside_preprocess = false;
		this->statusBar()->showMessage("Curvelets + Scalar Voting Disabled");
	}
}
void QtTracer::curvelet_scalar_voting()
{
	if( batch_processI || batch_processII ||batch_preprocessing )
	{
		file = images_dir.absoluteFilePath(string_array[current_idx]);
	}
	else
	{
		file = file_raw;
	}

	QString exe_path1;
	exe_path1.append(outside_preprocessor_path);
	exe_path1.append(tr("/"));
	exe_path1.append(tr("curvelets "));
	exe_path1.append(tr("\""));
	exe_path1.append(file);
	exe_path1.append(tr("\""));
	exe_path1.append(tr(" "));
	exe_path1.append(outside_preprocessor_path);
	exe_path1.append("/options_curvelets");
	std::cout<<exe_path1.toStdString()<<std::endl;
	system(exe_path1.toStdString().c_str());

	QString exe_path2;
	exe_path2.append(outside_preprocessor_path);
	exe_path2.append(tr("/"));
	exe_path2.append(tr("scalar_voting_2d "));
	QString new_file = file;
	new_file.replace(QString(".tif"), QString("_CV.tif"));
	new_file.replace(QString(".tiff"), QString("_CV.tiff"));
	exe_path2.append(tr("\""));
	exe_path2.append(new_file);
	exe_path2.append(tr("\""));
	exe_path2.append(tr(" "));
	new_file.replace(QString(".tif"), QString("_cos.mhd"));
	exe_path2.append(tr("\""));
	exe_path2.append(new_file);
	exe_path2.append(tr("\""));
	exe_path2.append(tr(" "));
	new_file.replace(QString("cos"), QString("sin"));
	exe_path2.append(tr("\""));
	exe_path2.append(new_file);
	exe_path2.append(tr("\""));
	exe_path2.append(tr(" "));
	exe_path2.append(outside_preprocessor_path);
	exe_path2.append("/options_scalar_voting_2d");
	std::cout<<exe_path2.toStdString()<<std::endl;
	system(exe_path2.toStdString().c_str());

	file.replace(QString(".tif"), QString("_CV_SV2D.tif"));
	file.replace(QString(".tiff"), QString("_CV_SV2D.tiff"));
	std::cout<<"file:"<<file.toStdString()<<std::endl;

	if( !batch_processI && !batch_processII && !batch_preprocessing )
	{
		//IM->IRGB = IM->ImMaxProjection1(IM->I);
		IM->IDisplay = IM->ImCopy();
		slider->setCheckable(-1);
	}
	loadImage();
}

void QtTracer::resetPreprocessStage()
{
	Preprocess_Stage = 1;
	Preprocess();
}

void QtTracer::Preprocess()
{
	int progress = 0;

	std::cout<<"--------------Preprocessing--------------"<<std::endl;

	Start_t = time(NULL);

	if( outside_preprocess && Preprocess_Stage == 1)
	{
		curvelet_scalar_voting();
	}

	if( general_para12->getMultiScale() && Preprocess_Stage == 1 )
	{
		Progress->setMinimum(0);
		Progress->setMaximum(5);
		Progress->setValue(0);

		std::cout<<"Multi-Scale Vesselness Enhancement..."<<std::endl;
		this->statusBar()->showMessage("Multi-Scale Vesselness Enhancement");
		IM->ComputeMultiVesselness(general_para12->general_para1->getMinSigma(), general_para12->general_para1->getMaxSigma(), general_para12->general_para1->getSigmaStep());
		tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->ImRescale(IM->I)), false));	
		progress++;
		Progress->setValue(progress);

		//set original image as raw image
		slider->setCheckable(-1);
		slider->setImageState(-1);
	}


	general_para12->disableSetting();

	float gvf_gpu_time = 0.0;
	float gvf_cpu_time = 0.0;
	float vessel_gpu_time = 0.0;
	float vessel_cpu_time = 0.0;
	//without break for each case
	switch(Preprocess_Stage)
	{
	case 1:
		{
			std::cout<<"Compute Gradient Vector Flow..."<<std::endl;

			this->statusBar()->showMessage("Compute Gradient Vector Flow");
			
			itk::TimeProbe clock;
			clock.Start();

			//clock_t init_time1 = clock(); 
			IM->computeGVF(1000,general_para12->getNumIteration(),general_para12->getSmoothingScale());
			//clock_t end_time1 = clock(); 

			clock.Stop();
			std::cout << "Total: " << clock.GetTotal() << std::endl;
			//std::cout << end_time1-init_time1 << " CPU ITK: "  
			//	<< (float)(end_time1-init_time1)/CLOCKS_PER_SEC << " seconds." << std::endl;

			progress++;
			Progress->setValue(progress);
		}
	case 2:
		{
			if (general_para12->getLaplacianOfGaussian()){
				std::cout<<"Gaussing, and then Laplacing the gausses..."<<std::endl;
				clock_t init_time1 = clock(); 
				IM->ImLaplacian_of_Gaussian();
				clock_t end_time1 = clock(); 
				std::cout << end_time1-init_time1 << " CPU: "  
					<< (float)(end_time1-init_time1)/CLOCKS_PER_SEC << " seconds." << std::endl; 
			}

			std::cout<<"Compute Vesselness (CPU)..."<<std::endl;
			this->statusBar()->showMessage("Compute Vesselness (CPU)");
			clock_t init_time1 = clock(); 
			IM->ComputeGVFVesselness();
			clock_t end_time1 = clock(); 
			std::cout << end_time1-init_time1 << " CPU: "  
				<< (float)(end_time1-init_time1)/CLOCKS_PER_SEC << " seconds." << std::endl; 

			//automatically set the threshold
			if( general_para12->getThreshold() == 1 )
				general_para12->setThreshold(IM->v_threshold);

			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->ImRescale(IM->IVessel)), false));
			slider->setCheckable(2);
			slider->setImageState(2);

			if( Preprocess_Stage != 1 )
				progress = Preprocess_Stage;

			progress++;
			Progress->setValue(progress);
		}
	case 3:
		{
			std::cout<<"Detect Seed Points..."<<std::endl;
			this->statusBar()->showMessage("Detect Seed Points");
			IM->SeedDetection(general_para12->getThreshold(),general_para12->getCurrentSeedIndex(),general_para12->getSeedRadius());

			if( Preprocess_Stage == 3 )
				progress = 3;

			progress++;
			Progress->setValue(progress);

			std::cout<<"Adjust Seed Points..."<<std::endl;
			this->statusBar()->showMessage("Adjust Seed Points");
			IM->SeedAdjustment(general_para12->getSeedAdjustment());
			//IM->OutputSeeds();  // output seeds in 8 bit image

			original_seed_num = IM->SeedPt.NP; //for seed testing
			std::cout<<"Preprocessing Finished..."<<std::endl;
			//send IM to tracing viewer
			tracingViewer->setImage(IM);
			draw3DTraces();

			if( Preprocess_Stage == 4 )
				progress = 4;

			progress++;
			Progress->setValue(progress);
		}
	default:;
	}

	End_t = time(NULL);
	preprocessing_time = difftime(End_t, Start_t);
	QString MSG("Time Elapsed for Preprocessing: ");
	QString MSG_Num;
	MSG_Num.setNum(preprocessing_time);
	MSG.append(MSG_Num);
	MSG.append(" seconds");

	this->statusBar()->showMessage(MSG);

	nextButton->setEnabled(true);  

	general_para12->enableSetting();
}

void QtTracer::Reprocess_Vessel(int in)
{
	Preprocess_Stage = in;
	Preprocess();
}

void QtTracer::loadDisplayImage()
{
	file_display = QFileDialog::getOpenFileName(this , "Load Image for Display", ".",
		tr("Trace Image ( *.tiff *.tif )"));
	this->statusBar()->showMessage("filename:\t" + file_display);
	std::cout<<"Image:"<< file_display.toStdString()<<std::endl;
	drawVolume();
	if (!file_display.isEmpty())
	{
		std::string file_name = file_display.toStdString();
		const char *file_name_ITK = file_name.c_str();

		IM->ImDisplayRead(file_name_ITK, general_para->getSh());
		//IM->ConvertReadImage();

		//tracingViewer->setDisplayImage(convertITK2QT(IM->ImMinProjection(IM->IRGB)));	 
		tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->IDisplay), false));	
		//imageLabel->setPixmap(QPixmap::fromImage(convertITK2QT(IM->ImMaxProjection(IM->I), false)));
		//set slider range
		//slider->setMinimum(1);
		//slider->setMaximum(IM->I->GetLargestPossibleRegion().GetSize()[2]);
		//slider->setSliderAvailable(true);
		slider->setCheckable(-1);
		slider->setImageState(-1);
		this->statusBar()->showMessage("Ready");
	}
}

void QtTracer::stackImage()
{
	bool outside_preprocess_first;
	if( general_para12->getCurveletBatch() == 0 )
		outside_preprocess_first = true; //after stacking images, use curvelet and scalr voting to preprocess images before tracing
	else if( general_para12->getCurveletBatch() == 1 )
		outside_preprocess_first = false;

	std::cout<<"outside_preprocess_fist:"<<outside_preprocess_first<<std::endl;

	//specify the folder containing slices
	QString dir_path = QFileDialog::getExistingDirectory(this, tr("Open Slices Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if (dir_path.isEmpty())
	{
		QMessageBox::information(this, tr("Warning"), tr("Empty folder"));
		return;
	}

	//specify the folder for storing output
	QString output_path = QFileDialog::getExistingDirectory(this, tr("Output Stacked Image Directory"),
		"/home",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	if (output_path.isEmpty())
	{
		QMessageBox::information(this, tr("Warning"), tr("Empty folder"));
		return;
	}


	if( general_para->getBatchIProcessing() )
	{
		current_idx = -1;
		//specify the output swc folder
		swcs_path = QFileDialog::getExistingDirectory(this, tr("Output SWCs Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

		if(swcs_path.isEmpty())
		{
			QMessageBox::information(this, tr("Warning"), tr("Empty folder"));
			return;
		}

	}
	else if( general_para->getBatchIIProcessing() )
	{
		//specify the output swc folder
		swcs_path = QFileDialog::getExistingDirectory(this, tr("Output SWCs Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);
		if(swcs_path.isEmpty())
		{
			QMessageBox::information(this, tr("Warning"), tr("Empty folder"));
			return;
		}

		//load coordinates 
		txt_fileName = QFileDialog::getOpenFileName(this, tr("Load Coordinates"),
			".", tr("Text File (*.txt)"));

		int index_tab1 = imageTab->indexOf(scrollArea_MT);
		imageTab->setTabEnabled(index_tab1,true);
	}


	QDir slice_images_dir;
	slice_images_dir.setPath(dir_path);
	slice_images_dir.setFilter(QDir::AllDirs | QDir::NoDotAndDotDot | QDir::NoSymLinks);


	if( slice_images_dir.count() != 0 )  //multiple images
	{
		this->statusBar()->showMessage("stack multiple images...");
		std::cout<<"stack multiple images..."<<std::endl;
		for( unsigned i = 0; i < slice_images_dir.count(); i++ )
		{
			QString sub_dir_path = slice_images_dir.absoluteFilePath(slice_images_dir[i]);

			QDir sub_dir;
			sub_dir.setPath(sub_dir_path);
			sub_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
			QStringList filters;
			filters << "*.tif" << "*.tiff";
			sub_dir.setNameFilters(filters);

			QString output_name;
			output_name = output_path;
			output_name.append(tr("/"));
			//append prefix
			output_name.append(tr("DIADEM_"));
			output_name.append(slice_images_dir[i]);
			output_name.append(".tif");

			std::vector< std::string > filenames;
			for( unsigned j = 1; j <= sub_dir.count(); j++ ) //images start with 001.tif...
				//for( unsigned j = 0; j < sub_dir.count(); j++ ) //images start with 000.tif
			{
				QString temp, temp1, temp2, temp3, temp4, temp5;
				temp.setNum(j);
				temp.prepend(tr("/"));
				temp.append(".tif");
				temp1.setNum(j);
				temp1.prepend("0");
				temp1.prepend(tr("/"));
				temp1.append(".tif");
				temp2.setNum(j);
				temp2.prepend("00");
				temp2.prepend(tr("/"));
				temp2.append(".tif");

				temp3.setNum(j);
				temp3.prepend(tr("/"));
				temp3.append(".TIF");
				temp4.setNum(j);
				temp4.prepend("0");
				temp4.prepend(tr("/"));
				temp4.append(".TIF");
				temp5.setNum(j);
				temp5.prepend("00");
				temp5.prepend(tr("/"));
				temp5.append(".TIF");


				for( unsigned k = 0; k < sub_dir.count(); k++ )
				{
					if( sub_dir.absoluteFilePath(sub_dir[k]).contains(temp) 
						||  sub_dir.absoluteFilePath(sub_dir[k]).contains(temp1)
						||  sub_dir.absoluteFilePath(sub_dir[k]).contains(temp2)
						||  sub_dir.absoluteFilePath(sub_dir[k]).contains(temp3)
						||  sub_dir.absoluteFilePath(sub_dir[k]).contains(temp4)
						||  sub_dir.absoluteFilePath(sub_dir[k]).contains(temp5) )
					{
						std::cout<<sub_dir.absoluteFilePath(sub_dir[k]).toStdString()<<std::endl;
						filenames.push_back(sub_dir.absoluteFilePath(sub_dir[k]).toStdString());
						break;
					}
				}
			}
			IM->ImSeriesReadWrite( filenames, output_name.toStdString().c_str(), general_para->getSf(), general_para->getSixteenBit());
		}

		using_stacked_image = true;

		//set the output directory as current working directory
		QStringList filters;
		foreach (QByteArray format, QImageReader::supportedImageFormats())
			filters += "*." + format;
		filters << "*.tiff" << "*.tif";
		images_dir.setPath(output_path);
		images_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
		images_dir.setNameFilters(filters);

		//start batch processing I automatically
		if( general_para->getBatchIProcessing() )
		{ 
			string_array = new QString[images_dir.count()];
			for( unsigned int i = 0; i < images_dir.count(); i++ )
			{
				string_array[i] = images_dir[i];
			}
			//start tracing
			batchTracingI();
		}
		else if( general_para->getBatchIIProcessing() )
		{
			//int NSpace = 4000;
			//GSnakeList.SetNSpace(NSpace);
			string_array = new QString[images_dir.count()];
			std::string *std_string_array = new std::string[images_dir.count()]; 

			for( unsigned int i = 0; i < images_dir.count(); i++ )
			{
				string_array[i] = images_dir[i];
				std_string_array[i] = images_dir[i].toStdString();
			}

			//use curvelet and scalar voting first to preprocess all images before tracing
			if( outside_preprocess_first && outside_preprocess )
			{
				for( current_idx = 0; current_idx < images_dir.count(); current_idx++ )
				{
					curvelet_scalar_voting();
				}

				//reload preprocessed images
				QStringList filters1;
				filters1 << "*_CV_SV2D.tif";
				images_dir.setPath(output_path);
				images_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
				images_dir.setNameFilters(filters1);

				for( unsigned int i = 0; i < images_dir.count(); i++ )
				{
					string_array[i] = images_dir[i];
					std_string_array[i] = images_dir[i].toStdString();
				}

				outside_preprocess = false;
			}

			reg.set_n(images_dir.count(),std_string_array);

			//load the coordinate text file
			QFile txt_file(txt_fileName);
			if (!txt_file.open(QIODevice::ReadOnly)) 
			{
				QMessageBox::information(this, tr("Warning"), tr("Cannot load the coordinates file"));
				return;
			}
			else
			{ 
				coordinates.RemoveAllPts();
				QTextStream in(&txt_file);

				//read root tile number
				QString line = in.readLine();
				QStringList fields = line.split(" ",QString::SkipEmptyParts);
				set_transform_dialog->setRoot(fields.takeLast().toInt());
				std::cout<<"Root:"<<set_transform_dialog->getRoot()<<std::endl;

				//read image width
				line = in.readLine();
				fields = line.split(" ",QString::SkipEmptyParts);
				set_transform_dialog->setWidth(fields.takeLast().toInt());
				std::cout<<"Image Width:"<<set_transform_dialog->getWidth()<<std::endl;

				//read image height
				line = in.readLine();
				fields = line.split(" ",QString::SkipEmptyParts);
				set_transform_dialog->setHeight(fields.takeLast().toInt());
				std::cout<<"Image Height:"<<set_transform_dialog->getHeight()<<std::endl;

				for( unsigned int i = 0; i < images_dir.count(); i++ )
				{
					line = in.readLine();
					QStringList fields = line.split(" ",QString::SkipEmptyParts);
					coordinates.AddPt(fields.at(1).toInt(), fields.at(2).toInt(),0);
				}

				std::cout<<"#coordinates:"<<coordinates.NP<<std::endl;
			}

			//generate graph after all information is ready
			generateGraph();
			//start tracing
			batchTracingII();
		}

	}
	else  //single image
	{  
		this->statusBar()->showMessage("stack single image...");
		std::cout<<"stack single image..."<<std::endl;
		slice_images_dir.setPath(dir_path);
		slice_images_dir.setFilter(QDir::Files | QDir::NoDotAndDotDot | QDir::NoSymLinks );
		QStringList filters;
		filters << "*.tif" << "*.tiff";
		slice_images_dir.setNameFilters(filters);
		slice_images_dir.setSorting(QDir::Name);
		QString output_name;
		output_name = output_path;
		output_name.append(tr("/"));
		//append prefix
		output_name.append(tr("DIADEM_"));
		output_name.append(slice_images_dir.dirName());
		output_name.append(".tif");

		std::vector< std::string > filenames;
		for( unsigned j = 1; j <= slice_images_dir.count(); j++ )
		{
			QString temp, temp1, temp2, temp3, temp4, temp5;
			temp.setNum(j);
			temp.prepend(tr("/"));
			temp.append(".tif");
			temp1.setNum(j);
			temp1.prepend("0");
			temp1.prepend(tr("/"));
			temp1.append(".tif");
			temp2.setNum(j);
			temp2.prepend("00");
			temp2.prepend(tr("/"));
			temp2.append(".tif");

			temp3.setNum(j);
			temp3.prepend(tr("/"));
			temp3.append(".TIF");
			temp4.setNum(j);
			temp4.prepend("0");
			temp4.prepend(tr("/"));
			temp4.append(".TIF");
			temp5.setNum(j);
			temp5.prepend("00");
			temp5.prepend(tr("/"));
			temp5.append(".TIF");

			for( unsigned k = 0; k < slice_images_dir.count(); k++ )
		 {
			 if( slice_images_dir.absoluteFilePath(slice_images_dir[k]).contains(temp) 
				 ||  slice_images_dir.absoluteFilePath(slice_images_dir[k]).contains(temp1)
				 ||  slice_images_dir.absoluteFilePath(slice_images_dir[k]).contains(temp2)
				 ||  slice_images_dir.absoluteFilePath(slice_images_dir[k]).contains(temp3)
				 ||  slice_images_dir.absoluteFilePath(slice_images_dir[k]).contains(temp4)
				 ||  slice_images_dir.absoluteFilePath(slice_images_dir[k]).contains(temp5))
			 {
				 std::cout<<slice_images_dir.absoluteFilePath(slice_images_dir[k]).toStdString()<<std::endl;
				 filenames.push_back(slice_images_dir.absoluteFilePath(slice_images_dir[k]).toStdString());
				 break;
			 }
		 }
		}

		IM->ImSeriesReadWrite( filenames, output_name.toStdString().c_str(), general_para->getSf(), general_para->getSixteenBit());

		using_stacked_image = true;

		//load stacked single image
		file = output_name;
		file_raw = file;
		this->statusBar()->showMessage("filename:\t" + file);
		std::cout<<"Image:"<< file.toStdString()<<std::endl;
		if (!file.isEmpty())
		{
			std::string file_name = file.toStdString();
			const char *file_name_ITK = file_name.c_str();

			IM->ImRead(file_name_ITK);
			//IM->ConvertReadImage();
			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
			//imageLabel->setPixmap(QPixmap::fromImage(convertITK2QT(IM->ImMaxProjection(IM->I), false)));
			//set slider range
			slider->setMinimum(1);
			slider->setMaximum(IM->I->GetLargestPossibleRegion().GetSize()[2]);
			slider->setSliderAvailable(true);
			slider->setCheckable(1);

			this->statusBar()->showMessage("Ready");

			nextButton->setEnabled(true);

			slider->setImageState(1);
		}

		//start automated processing automatically
		if(general_para->getContinueAutoProcessing())
		{
			//autoProcessing();
			nextStep();
		}
	}
}

void QtTracer::loadImage_DragNDrop()
{
	this->statusBar()->showMessage("filename:\t" + file);

    //3D Rendering
	drawVolume();

	if (!file.isEmpty())
	{
		std::string file_name = file.toStdString();
		const char *file_name_ITK = file_name.c_str();

		IM->ImRead(file_name_ITK);
		//IM->ConvertReadImage();

		IM->ImShrink(general_para->getSh());

		//set focal point of the camera
		this->Renderer->GetActiveCamera()->SetFocalPoint(IM->SM/2,IM->SN/2,0);
		this->Renderer->GetActiveCamera()->SetPosition(IM->SM/2,IM->SN/2,100);
		this->Renderer->GetActiveCamera()->SetViewUp(0,-1,0);
		this->Renderer->GetActiveCamera()->Azimuth(180);

		tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
		//imageLabel->setPixmap(QPixmap::fromImage(convertITK2QT(IM->ImMaxProjection(IM->I), false)));
		//set slider range
		slider->setMinimum(1);
		slider->setMaximum(IM->I->GetLargestPossibleRegion().GetSize()[2]);
		slider->setSliderAvailable(true);
		slider->setCheckable(1);

		this->statusBar()->showMessage("Ready");

		nextButton->setEnabled(true);

		slider->setImageState(1);
	}
   
}
void QtTracer::loadImage()
{
	if( !batch_processI && !batch_processII )
	{
		if( !outside_preprocess )
		{
			file = QFileDialog::getOpenFileName(this , "Load Image", ".",
				tr("Trace Image ( *.tiff *.tif )"));

			file_raw = file;
		}
		this->statusBar()->showMessage("filename:\t" + file);
		std::cout<<"Image:"<< file.toStdString()<<std::endl;

		//3D Rendering
		drawVolume();

		if (!file.isEmpty())
		{
			std::string file_name = file.toStdString();
			const char *file_name_ITK = file_name.c_str();

			IM->ImRead(file_name_ITK);
			//IM->ConvertReadImage();

			IM->ImShrink(general_para->getSh());

			//set focal point of the camera
			this->Renderer->GetActiveCamera()->SetFocalPoint(IM->SM/2,IM->SN/2,0);
			this->Renderer->GetActiveCamera()->SetPosition(IM->SM/2,IM->SN/2,100);
			this->Renderer->GetActiveCamera()->SetViewUp(0,-1,0);
			this->Renderer->GetActiveCamera()->Azimuth(180);

			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
			//imageLabel->setPixmap(QPixmap::fromImage(convertITK2QT(IM->ImMaxProjection(IM->I), false)));
			//set slider range
			slider->setMinimum(1);
			slider->setMaximum(IM->I->GetLargestPossibleRegion().GetSize()[2]);
			slider->setSliderAvailable(true);
			slider->setCheckable(1);

			this->statusBar()->showMessage("Ready");

			nextButton->setEnabled(true);

			slider->setImageState(1);
		}
	}

	if( batch_processI )
	{
		if( !outside_preprocess )
		{
			file = images_dir.absoluteFilePath(string_array[current_idx]);
			std::cout<<current_idx+1<<"th"<<" "<<"Image:"<< file.toStdString()<<std::endl;
		}

		//drawVolume();

		this->statusBar()->showMessage("filename:\t" + file);

		if (!file.isEmpty())
		{
			std::string file_name = file.toStdString();
			const char *file_name_ITK = file_name.c_str();

			IM->ImRead(file_name_ITK);
			//IM->ConvertReadImage();

			IM->ImShrink(general_para->getSh());

			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
			//imageLabel->setPixmap(QPixmap::fromImage(convertITK2QT(IM->ImMaxProjection(IM->I), false)));
			//set slider range
			slider->setMinimum(1);
			slider->setMaximum(IM->I->GetLargestPossibleRegion().GetSize()[2]);
			slider->setSliderAvailable(true);
			slider->setCheckable(1);

			this->statusBar()->showMessage("Ready");

			slider->setImageState(1);
		}
	}

	if( batch_processII )
	{
		if( !outside_preprocess )
		{
			file = images_dir.absoluteFilePath(string_array[current_idx]);
			std::cout<<current_idx+1<<"th"<<" "<<"Image:"<< file.toStdString()<<std::endl;
		}

		//drawVolume();
		//drawVolume((coordinates.Pt[current_idx].x - min_x)/general_para->getSf(), (coordinates.Pt[current_idx].y)/general_para->getSf(), 0);


		this->statusBar()->showMessage("filename:\t" + file);

		if (!file.isEmpty())
		{
			std::string file_name = file.toStdString();
			const char *file_name_ITK = file_name.c_str();

			IM->ImRead(file_name_ITK);
			//IM->ConvertReadImage();

			IM->ImShrink(general_para->getSh());


			tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
			//imageLabel->setPixmap(QPixmap::fromImage(convertITK2QT(IM->ImMaxProjection(IM->I), false)));
			//set slider range
			slider->setMinimum(1);
			slider->setMaximum(IM->I->GetLargestPossibleRegion().GetSize()[2]);
			slider->setSliderAvailable(true);
			slider->setCheckable(1);

			this->statusBar()->showMessage("Ready");

			slider->setImageState(1);
		}
	}

}

void QtTracer::reloadImage()
{
	if (!file.isEmpty())
	{
		this->statusBar()->showMessage("filename:\t" + file);
		std::string file_name = file.toStdString();
		const char *file_name_ITK = file_name.c_str();

		IM->ImRead(file_name_ITK);
		IM->ConvertReadImage();

		//in case of using stacked image, image has already been shrinked
		if( !using_stacked_image )
		{
			IM->ImShrink(general_para->getSh());
		}

		tracingViewer->setDisplayImage(convertITK2QT(IM->ImMaxProjection(IM->I), false));	 
		//set slider range
		slider->setMinimum(1);
		slider->setMaximum(IM->I->GetLargestPossibleRegion().GetSize()[2]);
		this->statusBar()->showMessage("Ready");
		nextButton->setEnabled(true);
	}
}

void QtTracer::outputSeeds()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Seeds to txt file"),
		"seeds.txt",
		tr("Text File(*.txt)"));

	QFile seed_file(fileName);

	if (seed_file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream *out_txt;
		out_txt = new QTextStream(&seed_file);
		for( int i = 0; i < IM->SeedPt.GetSize(); i++ )
		{
			*out_txt<<IM->SeedPt.Pt[i].x * general_para->getSh()<<" ";
			*out_txt<<IM->SeedPt.Pt[i].y * general_para->getSh()<<" ";
			*out_txt<<IM->SeedPt.Pt[i].z;

			if( i != IM->SeedPt.GetSize() - 1 )
				*out_txt<<"\n";
		}
	}

}

void QtTracer::setRootPoint()
{
	if( Process_Stage == 3)
	{
		if( tracingViewer->rootSnakeSelected() )
		{
			set_root_dialog->setX( tracingViewer->getClickedPoint().x * general_para->getSh() );
			set_root_dialog->setY( tracingViewer->getClickedPoint().y * general_para->getSh() );
			set_root_dialog->setZ( tracingViewer->getClickedPoint().z * general_para->getSh() );
			convertSnakeTree();
		}
		else
		{
			set_root_dialog->show();
		}
	}
}

void QtTracer::clearSnakeTree()
{
	if( snake_tree_swc)
	{
		tracingViewer->removeSnakeTree();
	}

	if( snake_tree )
	{
		snake_tree->root_point.x = 0;
		snake_tree->root_point.y = 0;
		snake_tree->root_point.z = 0;
		snake_tree->parent_list.clear();
		//snake_tree->points.NP = 0;
		snake_tree->points.RemoveAllPts();
		snake_tree->snake_id.clear();
		//snake_tree->branch_point.NP = 0;
		snake_tree->branch_point.RemoveAllPts();
		tracingViewer->removeSnakeTree();

		//clear all the vtk actors for displaying the snake tree
		if( legend )
		{
			this->Renderer->RemoveActor(legend);
		}
		if( tree_line_actors.size() != 0 )
		{
			for( int i = 0; i < tree_line_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(tree_line_actors[i]);
			}
			tree_line_actors.clear();
		}
		if( tree_tube_actors.size() != 0 )
		{
			for( int i = 0; i < tree_tube_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(tree_tube_actors[i]);
			}
			tree_tube_actors.clear();
		}
		if( tree_branch_actors.size() != 0 )
		{
			for( int i = 0; i < tree_branch_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(tree_branch_actors[i]);
			}
			tree_branch_actors.clear();
		}
		this->QVTK->GetRenderWindow()->Render();

		//close the tree feature table
		tree_features.clear();
		table_sum->close();
	}
	if( tree_features.size() != 0 )
	{
		//clear all the vtk actors for displaying the snake tree
		if( legend )
		{
			this->Renderer->RemoveActor(legend);
		}
		if( tree_line_actors.size() != 0 )
		{
			for( int i = 0; i < tree_line_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(tree_line_actors[i]);
			}
			tree_line_actors.clear();
		}
		if( tree_tube_actors.size() != 0 )
		{
			for( int i = 0; i < tree_tube_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(tree_tube_actors[i]);
			}
			tree_tube_actors.clear();
		}
		if( tree_branch_actors.size() != 0 )
		{
			for( int i = 0; i < tree_branch_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(tree_branch_actors[i]);
			}
			tree_branch_actors.clear();
		}
		this->QVTK->GetRenderWindow()->Render();
		tree_features.clear();
		table_sum->close();
	}

	tracingViewer->removeInterestPoints();
	general_para3->output_swc_button->setEnabled(false);
}

void QtTracer::convertSnakeTree()
{

	float offset_x = general_para3->getX();
	float offset_y = general_para3->getY();
	float offset_z = general_para3->getZ();

	QString MSG("Root Point: ");
	QString X_NUM;
	X_NUM.setNum(set_root_dialog->getX());
	MSG.append(X_NUM);
	MSG.append(", ");
	QString Y_NUM;
	Y_NUM.setNum(set_root_dialog->getY());
	MSG.append(Y_NUM);
	MSG.append(", ");
	QString Z_NUM;
	Z_NUM.setNum(set_root_dialog->getZ());
	MSG.append(Z_NUM);
	this->statusBar()->showMessage(MSG);

	snake_tree = new SnakeTree;
	//snake_tree->points.SetN(100000);

	//snake_tree->root_point.x = (set_root_dialog->getX() - offset_x)/general_para->getSh();
	//snake_tree->root_point.y = (set_root_dialog->getY() - offset_y)/general_para->getSh();
	//snake_tree->root_point.z = (set_root_dialog->getZ() - offset_z);
	snake_tree->root_point.x = (set_root_dialog->getX())/general_para->getSh();
	snake_tree->root_point.y = (set_root_dialog->getY())/general_para->getSh();
	snake_tree->root_point.z = (set_root_dialog->getZ());

	//find the nearst snake and write the branches
	vnl_vector<float> dist( 2 * Tracer->SnakeList.NSnakes );
	for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
	{
		//ignore invalid snakes
		if( Tracer->SnakeList.valid_list[i] == 0 )
		{
			dist( i * 2 ) = 1000000;
			dist( i* 2 + 1 ) = 1000000;
		}
		else
		{
			dist( i * 2 ) = snake_tree->root_point.GetDistTo( Tracer->SnakeList.Snakes[i].Cu.GetFirstPt() );
			dist( i* 2 + 1 ) = snake_tree->root_point.GetDistTo( Tracer->SnakeList.Snakes[i].Cu.GetLastPt() );
		}
	}

	int idx = dist.arg_min();
	int snake_id = idx/2;
	vnl_vector<int> *snake_visit_label;
	snake_visit_label = new vnl_vector<int>(Tracer->SnakeList.NSnakes);
	snake_visit_label->fill(0);
	int *point_id;
	point_id = new int[1];
	point_id[0] = 1;

	std::cout<<".......Converting Raw Traces to Tree......."<<std::endl;
	findBranch(snake_id, -1, snake_tree->root_point, snake_visit_label, point_id, snake_tree);
	std::cout<<".......Convertion Finished......."<<std::endl;
	//send tree to tracing view
	general_para3->output_swc_button->setEnabled(true);
	tracingViewer->setSnakeTree(snake_tree);

	//deselect the traces
	tracingViewer->deselect();
	if( selected_line_actors.size() != 0 )
	{
		for( int i = 0; i < selected_line_actors.size(); i++ )
		{
				this->Renderer->RemoveActor(selected_line_actors[i]);
		}
	}

	//write tree to SWC file
	outputSWC_F();

	//load back for display
	loadSWC_F();

	/*double color[3];
	color[0] = 0;
	color[1] = 0;
	color[2] = 1;

	SnakeListClass SWCSnakes;
	for( int i = 0; i < snake_tree->snake_id.size(); i++ )
	{
	SWCSnakes.AddSnake(Tracer->SnakeList.Snakes[snake_tree->snake_id[i]]);
	}

	displaySWC(SWCSnakes, snake_tree->root_point, snake_tree->branch_point, color);*/

}

void QtTracer::output_swc_raw_slot()
{
	if( batch_processII )
	{
		outputSWC_Raw_MT();
	}
	else
	{
		outputSWC_Raw();
	}
}

void QtTracer::outputSWC()
{
	//convertSnakeTree();
	//int ratio = general_para3->getScale();
	int ratio = general_para->getSf() * general_para->getSh();

	float offset_x = general_para3->getX();
	float offset_y = general_para3->getY();
	float offset_z = general_para3->getZ();

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Snakes to swc file"),
		"snake_tracing.swc",
		tr("Text File(*.swc)"));

	QFile swc_file(fileName);

	if (swc_file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream *out_txt;
		out_txt = new QTextStream(&swc_file);
		//*out_txt<<"# SWC Generated by Open-Curve Snake Tracing System"<<"\n";

		for( int i = 0; i < snake_tree->points.GetSize(); i++ )
		{
			*out_txt<<i+1<<" ";

			//if soma exists, write soma and dendrite types instead
			if( IM->num_soma != 0 )
			{
				//check if the point is inside the soma
				typedef itk::LinearInterpolateImageFunction< 
					LabelImageType, float>  InterpolatorType;
				InterpolatorType::Pointer IL_interpolator = InterpolatorType::New();
				IL_interpolator->SetInputImage(IM->ISoma);
				InterpolatorType::ContinuousIndexType index;
				index[0] = snake_tree->points.Pt[i].x;
				index[1] = snake_tree->points.Pt[i].y;
				index[2] = snake_tree->points.Pt[i].z;
				int intensity = IL_interpolator->EvaluateAtContinuousIndex(index);

				if( intensity != 0 )
					*out_txt<<"1"<<" ";
				else
					*out_txt<<"3"<<" ";
			}
			else
			{
				*out_txt<<"2"<<" ";
			}

			*out_txt<<snake_tree->points.Pt[i].x * ratio + offset_x<<" ";
			*out_txt<<snake_tree->points.Pt[i].y * ratio + offset_y<<" ";
			*out_txt<<snake_tree->points.Pt[i].z + offset_z<<" ";
			//*out_txt<<snake_tree->Ru[i] * sqrt((double)2)<<" ";
			*out_txt<<snake_tree->Ru[i] * ratio<<" ";
			//*out_txt<<"0"<<" ";
			*out_txt<<snake_tree->parent_list[i];
			if( i != snake_tree->points.GetSize() - 1 )
				*out_txt<<"\n";
		}
	}

	this->statusBar()->showMessage("SWC Saved");
	/*Point3D root_point;

	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Snakes to swc file"),
	"snake_tracing.swc",
	tr("Text File(*.swc)"));

	QFile swc_file(fileName);

	if (swc_file.open(QFile::WriteOnly | QFile::Truncate))
	{
	QTextStream *out_txt;
	out_txt = new QTextStream(&swc_file);

	root_point.x = 30.979/general_para->getSh();
	root_point.y = 429.04/general_para->getSh();
	root_point.z = 0;

	//find the nearst snake and write the branches
	vnl_vector<float> dist( 2 * Tracer->SnakeList.NSnakes );
	for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
	{
	dist( i * 2 ) = root_point.GetDistTo( Tracer->SnakeList.Snakes[i].Cu.GetFirstPt() );
	dist( i* 2 + 1 ) = root_point.GetDistTo( Tracer->SnakeList.Snakes[i].Cu.GetLastPt() );
	}

	int idx = dist.arg_min();
	int snake_id = idx/2;
	vnl_vector<int> *snake_visit_label;
	snake_visit_label = new vnl_vector<int>(Tracer->SnakeList.NSnakes);
	snake_visit_label->fill(0);
	int *point_id;
	point_id = new int[1];
	point_id[0] = 1;
	writeBranch(snake_id, -1, snake_tree->root_point, snake_visit_label, out_txt, point_id);
	} */
}

void QtTracer::outputSWC_F() //save the swc in current dir
{
	//convertSnakeTree();
	//int ratio = general_para3->getScale();
	int ratio = general_para->getSf() * general_para->getSh();

	float offset_x = general_para3->getX();
	float offset_y = general_para3->getY();
	float offset_z = general_para3->getZ();

	QString fileName("snake_tracing.swc");
	QFile swc_file(fileName);

	if (swc_file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream *out_txt;
		out_txt = new QTextStream(&swc_file);
		//*out_txt<<"# SWC Generated by Open-Curve Snake Tracing System"<<"\n";
		for( int i = 0; i < snake_tree->points.GetSize(); i++ )
		{
			*out_txt<<i+1<<" ";

			//if soma exists, write soma and dendrite types instead
			if( IM->num_soma != 0 )
			{
				//check if the point is inside the soma
				typedef itk::LinearInterpolateImageFunction< 
					LabelImageType, float>  InterpolatorType;
				InterpolatorType::Pointer IL_interpolator = InterpolatorType::New();
				IL_interpolator->SetInputImage(IM->ISoma);
				InterpolatorType::ContinuousIndexType index;

				snake_tree->points.Pt[i].check_out_of_range_3D(IM->SM,IM->SN,IM->SZ);
				index[0] = snake_tree->points.Pt[i].x;
				index[1] = snake_tree->points.Pt[i].y;
				index[2] = snake_tree->points.Pt[i].z;
				int intensity = IL_interpolator->EvaluateAtContinuousIndex(index);

				if( intensity != 0 )
					*out_txt<<"1"<<" ";
				else
					*out_txt<<"3"<<" ";
			}
			else
			{
				*out_txt<<"2"<<" ";
			}

			*out_txt<<snake_tree->points.Pt[i].x * ratio + offset_x<<" ";
			*out_txt<<snake_tree->points.Pt[i].y * ratio + offset_y<<" ";
			*out_txt<<snake_tree->points.Pt[i].z + offset_z<<" ";
			//*out_txt<<snake_tree->Ru[i] * sqrt((double)2)<<" ";
			*out_txt<<snake_tree->Ru[i] * ratio<<" ";
			//*out_txt<<"0"<<" ";
			*out_txt<<snake_tree->parent_list[i];
			if( i != snake_tree->points.GetSize() - 1 )
				*out_txt<<"\n";
		}
	}

	this->statusBar()->showMessage("SWC Saved");
}


void QtTracer::outputSWC_Raw_MT()
{
	bool inter_swc = true;

	if( bpII_finished )
	{
		//GSnakeList = GSnakeList_BP;
		inter_swc = false;
	}

	//int close_branch_point_dist = 3;
	//int ratio = general_para3->getScale();

	float offset_x = general_para3->getX();
	float offset_y = general_para3->getY();
	float offset_z = general_para3->getZ();

	QString fileName;
	if( inter_swc )
	{
		QString fileName_temp("snake_tracing.swc");
		fileName_temp.prepend("/");
		fileName_temp.prepend(swcs_path);
		fileName = fileName_temp;
	}	 
	else
	{
		fileName = QFileDialog::getSaveFileName(this, tr("Save Snakes to swc file"),
			"snake_tracing.swc",
			tr("Text File(*.swc)"));
	}

	QFile swc_file(fileName);

	if (swc_file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream *out_txt;
		out_txt = new QTextStream(&swc_file);

		vnl_vector<int> *snake_visit_label;
		snake_visit_label = new vnl_vector<int>(GSnakeList.NSnakes);
		snake_visit_label->fill(0);
		int *point_id;
		point_id = new int[1];
		point_id[0] = 1;

		All_Pt.RemoveAllPts();

		std::vector<int> *branch_label;
		branch_label = new std::vector<int>[1];


		for( int i = 0; i < GSnakeList.NSnakes; i++ )
		{
			if( snake_visit_label[0](i) == 1 )
				continue;
			if( GSnakeList.valid_list[i] == 0 )
				continue;

			wrote_pt.RemoveAllPts();

			int snake_id = i;
			findBranch_Raw_MT( snake_id, -1, GSnakeList.Snakes[i].Cu.GetFirstPt(), snake_visit_label, point_id, out_txt, &wrote_pt, &All_Pt, branch_label );
		}

		if( bpII_finished )
		{
			tracingViewer->setSnakes(&GSnakeList);
			tracingViewer->SnakesChanged();
		}
	}

	std::cout<<"swc saved......................"<<std::endl;
	this->statusBar()->showMessage("SWC Saved");

}

void QtTracer::findBranch_Raw_MT(int snake_label, int root_label, Point3D root_point, vnl_vector<int>*snake_visit_label, int *point_id, QTextStream *out_txt, PointList3D *wrote_pt, PointList3D *all_pt, std::vector<int> *branch_label )
{
	int collision_dist = general_para3->getConnection();
	//int collision_dist = 3;
	//int ratio = general_para3->getScale();
	int ratio = 1;
	int min_length = general_para3->getMinLength();

	snake_visit_label[0](snake_label) = 1;

	if( root_point.GetDistTo(GSnakeList.Snakes[snake_label].Cu.GetLastPt()) < 
		root_point.GetDistTo(GSnakeList.Snakes[snake_label].Cu.GetFirstPt()))
	{
		GSnakeList.Snakes[snake_label].Ru = GSnakeList.Snakes[snake_label].Cu.Flip_4D(GSnakeList.Snakes[snake_label].Ru);
	}

	/*//check the root tip for possible merging first
	if( root_label == -1 )
	{

	for( int j = 0; j < GSnakeList.NSnakes; j++ )
	{

	if( snake_visit_label[0](j) == 1 )
	continue;
	if( GSnakeList.valid_list[j] == 0 )
	continue;

	vnl_vector<float> temp_dist(GSnakeList.Snakes[j].Cu.GetSize());
	for( int i = 0; i < GSnakeList.Snakes[j].Cu.GetSize(); i++ )
	{
	temp_dist(i) = GSnakeList.Snakes[snake_label].Cu.GetFirstPt().GetDistTo(GSnakeList.Snakes[j].Cu.Pt[i]);
	}

	if( temp_dist.min_value() <= collision_dist )
	{
	int pt_id = temp_dist.arg_min();

	if( pt_id != 0 && pt_id != GSnakeList.Snakes[j].Cu.GetSize() - 1 )
	{

	float L1 = GSnakeList.Snakes[j].Cu.GetPartLength(pt_id,0);
	float L2 = GSnakeList.Snakes[j].Cu.GetPartLength(pt_id,1);

	//rearrangement
	if( L1 >= min_length && L2 >= min_length )
	{
	//change root point
	root_point = GSnakeList.Snakes[snake_label].Cu.GetLastPt();
	//add points to current snake
	for( int im = pt_id; im < GSnakeList.Snakes[j].Cu.GetSize(); im++ )
	{
	GSnakeList.Snakes[snake_label].Cu.AddTailPt(GSnakeList.Snakes[j].Cu.Pt[im]);
	}
	//remove points for the snake

	//GSnakeList.Snakes[j].Cu.NP = pt_id + 1;
	GSnakeList.Snakes[j].Cu.Resize(pt_id+1);

	//GSnakeList.valid_list[j] = 0;
	//SnakeClass temp;
	//for( int k = 0; k < pt_id; k++ )
	//{
	// temp.Cu.AddPt( GSnakeList.Snakes[j].Cu.Pt[k] );
	//}
	//IM->ImCoding( temp.Cu, GSnakeList.NSnakes+1, false );
	//GSnakeList.AddSnake(temp);

	GSnakeList.Snakes[snake_label].Cu.Flip();
	}
	//merging
	else if( L1 < min_length && L2 >= min_length )
	{
	//change root point
	root_point = GSnakeList.Snakes[snake_label].Cu.GetLastPt();
	//add points to current snake
	for( int im = pt_id; im < GSnakeList.Snakes[j].Cu.GetSize(); im++ )
	{
	GSnakeList.Snakes[snake_label].Cu.AddTailPt(GSnakeList.Snakes[j].Cu.Pt[im]);
	}
	//delete the snake
	GSnakeList.valid_list[j] = 0;

	GSnakeList.Snakes[snake_label].Cu.Flip();
	}
	else if( L2 < min_length && L1 >= min_length )
	{
	//change root point
	root_point = GSnakeList.Snakes[snake_label].Cu.GetLastPt();
	//add points to current snake
	for( int im = pt_id; im > 0; im-- )
	{
	GSnakeList.Snakes[snake_label].Cu.AddTailPt(GSnakeList.Snakes[j].Cu.Pt[im]);
	}
	//delete the snake
	GSnakeList.valid_list[j] = 0;

	GSnakeList.Snakes[snake_label].Cu.Flip();
	}
	else
	{
	//delete the snake
	GSnakeList.valid_list[j] = 0;
	}
	}
	else if( pt_id == 0 )
	{
	//change root point
	root_point = GSnakeList.Snakes[snake_label].Cu.GetLastPt();
	for( int im = 0; im < GSnakeList.Snakes[j].Cu.GetSize(); im++ )
	{
	GSnakeList.Snakes[snake_label].Cu.AddTailPt(GSnakeList.Snakes[j].Cu.Pt[im]);
	}
	GSnakeList.valid_list[j] = 0;

	GSnakeList.Snakes[snake_label].Cu.Flip();
	}
	else if( pt_id == GSnakeList.Snakes[j].Cu.GetSize() - 1 )
	{
	//change root point
	root_point = GSnakeList.Snakes[snake_label].Cu.GetLastPt();
	for( int im = GSnakeList.Snakes[j].Cu.GetSize() - 1; im >= 0; im-- )
	{
	GSnakeList.Snakes[snake_label].Cu.AddTailPt(GSnakeList.Snakes[j].Cu.Pt[im]);
	}
	GSnakeList.valid_list[j] = 0;

	GSnakeList.Snakes[snake_label].Cu.Flip();
	}

	////change the label image for snake selection
	//IM->ImCoding( GSnakeList.Snakes[snake_label].Cu, snake_label + 1, false );
	break;
	}
	}
	} */

	//find out if the tip (not the root point) of current main branch is actually a branch point
	bool check_merging = true;

	//while( check_merging )
	//{
	check_merging = false;
	for( int j = 0; j < GSnakeList.NSnakes; j++ )
	{

		if( snake_visit_label[0](j) == 1 )
			continue;
		if( GSnakeList.valid_list[j] == 0 )
			continue;

		vnl_vector<float> temp_dist(GSnakeList.Snakes[j].Cu.GetSize());
		for( int i = 0; i < GSnakeList.Snakes[j].Cu.GetSize(); i++ )
		{
			temp_dist(i) = GSnakeList.Snakes[snake_label].Cu.GetLastPt().GetDistTo(GSnakeList.Snakes[j].Cu.Pt[i]);
		}

		if( temp_dist.min_value() <= collision_dist )
		{
			int pt_id = temp_dist.arg_min();

			if( pt_id != 0 && pt_id != GSnakeList.Snakes[j].Cu.GetSize() - 1 )
			{

				float L1 = GSnakeList.Snakes[j].Cu.GetPartLength(pt_id,0);
				float L2 = GSnakeList.Snakes[j].Cu.GetPartLength(pt_id,1);

				//rearrangement
				if( L1 >= min_length && L2 >= min_length )
				{
					GSnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					GSnakeList.Snakes[snake_label].Ru.pop_back();
					//add points to current snake
					for( int im = pt_id; im < GSnakeList.Snakes[j].Cu.GetSize(); im++ )
					{
						GSnakeList.Snakes[snake_label].Cu.AddPt(GSnakeList.Snakes[j].Cu.Pt[im]);
						GSnakeList.Snakes[snake_label].Ru.push_back(GSnakeList.Snakes[j].Ru[im]);
					}
					//remove points for the snake
					//GSnakeList.Snakes[j].Cu.NP = pt_id + 1;
					GSnakeList.Snakes[j].Cu.Resize(pt_id+1);
					GSnakeList.Snakes[j].Ru.resize(pt_id+1);
					//Tracer->SnakeList.valid_list[j] = 0;
					//SnakeClass temp;
					//for( int k = 0; k < pt_id; k++ )
					//{
					// temp.Cu.AddPt( GSnakeList.Snakes[j].Cu.Pt[k] );
					//}
					//IM->ImCoding( temp.Cu, GSnakeList.NSnakes+1, false );
					//GSnakeList.AddSnake(temp);
				}
				//merging
				else if( L1 < min_length && L2 >= min_length )
				{
					GSnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					GSnakeList.Snakes[snake_label].Ru.pop_back();
					//add points to current snake
					for( int im = pt_id; im < GSnakeList.Snakes[j].Cu.GetSize(); im++ )
					{
						GSnakeList.Snakes[snake_label].Cu.AddPt(GSnakeList.Snakes[j].Cu.Pt[im]);
						GSnakeList.Snakes[snake_label].Ru.push_back(GSnakeList.Snakes[j].Ru[im]);
					}
					//delete the snake
					GSnakeList.valid_list[j] = 0;

					check_merging = true;
				}
				else if( L2 < min_length && L1 >= min_length )
				{
					//add points to current snake
					for( int im = pt_id; im > 0; im-- )
					{
						GSnakeList.Snakes[snake_label].Cu.AddPt(GSnakeList.Snakes[j].Cu.Pt[im]);
						GSnakeList.Snakes[snake_label].Ru.push_back(GSnakeList.Snakes[j].Ru[im]);
					}
					//delete the snake
					GSnakeList.valid_list[j] = 0;

					check_merging = true;
				}
				else
				{
					//delete the snake
					GSnakeList.valid_list[j] = 0;
				}
			}
			else if( pt_id == 0 )
			{
				GSnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
				GSnakeList.Snakes[snake_label].Ru.pop_back();
				for( int im = 0; im < GSnakeList.Snakes[j].Cu.GetSize(); im++ )
				{
					GSnakeList.Snakes[snake_label].Cu.AddPt(GSnakeList.Snakes[j].Cu.Pt[im]);
					GSnakeList.Snakes[snake_label].Ru.push_back(GSnakeList.Snakes[j].Ru[im]);
				}
				GSnakeList.valid_list[j] = 0;

				check_merging = true;
			}
			else if( pt_id == GSnakeList.Snakes[j].Cu.GetSize() - 1 )
			{
				GSnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
				GSnakeList.Snakes[snake_label].Ru.pop_back();
				for( int im = GSnakeList.Snakes[j].Cu.GetSize() - 1; im >= 0; im-- )
				{
					GSnakeList.Snakes[snake_label].Cu.AddPt(GSnakeList.Snakes[j].Cu.Pt[im]);
					GSnakeList.Snakes[snake_label].Ru.push_back(GSnakeList.Snakes[j].Ru[im]);
				}
				GSnakeList.valid_list[j] = 0;

				check_merging = true;
			}

			////change the label image for snake selection
			//IM->ImCoding( GSnakeList.Snakes[snake_label].Cu, snake_label + 1, false );
			break;
		}
	}
	//}

	//record the point id of current main branch
	vnl_vector<int> snake_point_id(GSnakeList.Snakes[snake_label].Cu.GetSize());
	snake_point_id.fill(0);
	//write the points of current main branch to swc file
	for( int i = 0; i < GSnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
	{
		if( root_label == -1 )
		{

			*out_txt<<point_id[0]<<" ";
			*out_txt<<"2"<<" ";
			if( i == 0)
			{
				*out_txt<<root_point.x * ratio<<" ";
				*out_txt<<root_point.y * ratio<<" ";
				*out_txt<<root_point.z<<" ";
				*out_txt<<"0"<<" ";
				*out_txt<<root_label<<"\n";

				wrote_pt->AddPt(root_point.x, root_point.y, root_point.z);
				all_pt->AddPt(root_point.x, root_point.y, root_point.z);
				branch_label->push_back(1);
			}
			else
			{
				*out_txt<<GSnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
				*out_txt<<GSnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
				*out_txt<<GSnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";
				//*out_txt<<"0"<<" ";
				*out_txt<<GSnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

				*out_txt<<point_id[0]-1<<"\n";
				wrote_pt->AddPt(GSnakeList.Snakes[snake_label].Cu.Pt[i].x, GSnakeList.Snakes[snake_label].Cu.Pt[i].y, GSnakeList.Snakes[snake_label].Cu.Pt[i].z);
				all_pt->AddPt(GSnakeList.Snakes[snake_label].Cu.Pt[i].x, GSnakeList.Snakes[snake_label].Cu.Pt[i].y, GSnakeList.Snakes[snake_label].Cu.Pt[i].z);
				branch_label->push_back(0);
			}

			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
		else
		{
			if( i == 0 )
				continue;


			if( i == GSnakeList.Snakes[snake_label].Cu.GetSize() - 1 )
			{
				bool loop = false;
				for( int k = 0; k < wrote_pt->NP; k++ )
				{
					if( GSnakeList.Snakes[snake_label].Cu.Pt[i].GetDistTo( wrote_pt->Pt[k] ) <= collision_dist )
					{
						loop = true;
						break;
					}
				}
				if( loop )
					break;
			}

			*out_txt<<point_id[0]<<" ";
			*out_txt<<"2"<<" ";
			*out_txt<<GSnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
			*out_txt<<GSnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
			*out_txt<<GSnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";
			//*out_txt<<"0"<<" ";

			*out_txt<<GSnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

			wrote_pt->AddPt(GSnakeList.Snakes[snake_label].Cu.Pt[i].x, GSnakeList.Snakes[snake_label].Cu.Pt[i].y, GSnakeList.Snakes[snake_label].Cu.Pt[i].z);
			all_pt->AddPt(GSnakeList.Snakes[snake_label].Cu.Pt[i].x, GSnakeList.Snakes[snake_label].Cu.Pt[i].y, GSnakeList.Snakes[snake_label].Cu.Pt[i].z);
			branch_label->push_back(0);

			if( i == 1 )
				*out_txt<<root_label<<"\n";
			else
				*out_txt<<point_id[0]-1<<"\n";

			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
	}

	//find side branches and do recursion
	for( int j = 0; j < GSnakeList.NSnakes; j++ )
	{
		if( snake_visit_label[0](j) == 1 )
			continue;
		if( GSnakeList.valid_list[j] == 0 )
			continue;

		vnl_vector<float> tail_dist( GSnakeList.Snakes[snake_label].Cu.GetSize() );
		tail_dist.fill(100000);
		vnl_vector<float> head_dist( GSnakeList.Snakes[snake_label].Cu.GetSize() );
		head_dist.fill(100000);
		for( int i = 1; i < GSnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
		{ 
			if( snake_point_id(i) == 0 )
				continue;
			tail_dist(i) = GSnakeList.Snakes[j].Cu.GetFirstPt().GetDistTo(GSnakeList.Snakes[snake_label].Cu.Pt[i]);
			head_dist(i) = GSnakeList.Snakes[j].Cu.GetLastPt().GetDistTo(GSnakeList.Snakes[snake_label].Cu.Pt[i]);
		}

		if( tail_dist.min_value() <= collision_dist )
		{
			int id_temp = tail_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = GSnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);
			branch_label[0][new_root_label-1] += 1;
			findBranch_Raw_MT(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt, all_pt, branch_label);
		}
		else if( head_dist.min_value() <= collision_dist )
		{
			int id_temp = head_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = GSnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);
			branch_label[0][new_root_label-1] += 1;
			findBranch_Raw_MT(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt, all_pt, branch_label);
		}

	}

	return;
}

void QtTracer::outputSWC_MG(std::vector<int> stem_label)
{
	//int close_branch_point_dist = 3;

	int ratio = general_para3->getScale();
	float offset_x = general_para3->getX();
	float offset_y = general_para3->getY();
	float offset_z = general_para3->getZ();

	QString fileName;
	fileName.setNum(current_mg_cell);
	fileName.prepend("Tree");
	fileName.append(".swc");
	fileName.prepend("/");
	fileName.prepend(image_folder);

	QFile swc_file(fileName);

	PointList3D points_temp;

	if (swc_file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream *out_txt;
		out_txt = new QTextStream(&swc_file);

		vnl_vector<int> *snake_visit_label;
		snake_visit_label = new vnl_vector<int>(Tracer->SnakeList.NSnakes);
		snake_visit_label->fill(0);
		int *point_id;
		point_id = new int[1];
		point_id[0] = 1;

		//write the root point first
		*out_txt<<1<<" ";
		*out_txt<<"1"<<" ";
		*out_txt<<IM->Centroid.Pt[current_mg_cell-1].x * ratio<<" ";
		*out_txt<<IM->Centroid.Pt[current_mg_cell-1].y * ratio<<" ";
		*out_txt<<IM->Centroid.Pt[current_mg_cell-1].z<<" ";
		*out_txt<<IM->soma_radii[current_mg_cell-1]<<" ";
		*out_txt<<"-1"<<"\n";

		point_id[0] = 2;

		//std::cout<<"snake_visit_label:"<<snake_visit_label->size()<<std::endl;
		//std::cout<<"stem_label:"<<stem_label.size()<<std::endl;
		for( int i = 0; i < stem_label.size(); i++ )
		{
			//std::cout<<"stem_label[i]:"<<stem_label[i]<<std::endl;
			snake_visit_label[0](stem_label[i]) = 1;
		}

		for( int i = 0; i < stem_label.size(); i++ )
		{
			std::cout<<"stem_label"<<stem_label[i]<<std::endl;

			points_temp.RemoveAllPts();
			int snake_id = stem_label[i];
			findBranch_MG( snake_id, 1, IM->Centroid.Pt[current_mg_cell-1], snake_visit_label, point_id, out_txt, &points_temp );
		}
	}

	this->statusBar()->showMessage("SWC Saved");
}

void QtTracer::findBranch_MG(int snake_label, int root_label, Point3D root_point, vnl_vector<int>*snake_visit_label, int *point_id, QTextStream *out_txt, PointList3D *wrote_pt )
{
	int collision_dist = general_para3->getConnection();
	//int ratio = general_para3->getScale();
	int ratio = general_para->getSf() * general_para->getSh();
	int min_length = general_para3->getMinLength();

	snake_visit_label[0](snake_label) = 1;

	if( root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt()) < 
		root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetFirstPt()))
	{
		//Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
		Tracer->SnakeList.Snakes[snake_label].Ru = Tracer->SnakeList.Snakes[snake_label].Cu.Flip_4D(Tracer->SnakeList.Snakes[snake_label].Ru);
	}

	bool check_merging = true; 
	while( check_merging ) 
	{
		check_merging = false;
		//find out if the tip (not the root point) of current main branch is actually a branch point
		for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
		{
			if( snake_visit_label[0](j) == 1 )
				continue;
			if( Tracer->SnakeList.valid_list[j] == 0 )
				continue;

			vnl_vector<float> temp_dist(Tracer->SnakeList.Snakes[j].Cu.GetSize());
			for( int i = 0; i < Tracer->SnakeList.Snakes[j].Cu.GetSize(); i++ )
			{
				temp_dist(i) = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[j].Cu.Pt[i]);
			}

			if( temp_dist.min_value() <= collision_dist )
			{
				int pt_id = temp_dist.arg_min();

				if( pt_id != 0 && pt_id != Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
				{

					float L1 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,0);
					float L2 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,1);

					//rearrangement
					if( L1 >= min_length && L2 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//remove points for the snake
						//Tracer->SnakeList.Snakes[j].Cu.NP = pt_id + 1;
						Tracer->SnakeList.Snakes[j].Cu.Resize(pt_id+1);
						Tracer->SnakeList.Snakes[j].Ru.resize(pt_id+1);
						//Tracer->SnakeList.valid_list[j] = 0;
						//SnakeClass temp;
						//for( int k = 0; k < pt_id; k++ )
						//{
						// temp.Cu.AddPt( Tracer->SnakeList.Snakes[j].Cu.Pt[k] );
						//}
						//IM->ImCoding( temp.Cu, Tracer->SnakeList.NSnakes+1, false );
						//Tracer->SnakeList.AddSnake(temp);
					}
					//merging
					else if( L1 < min_length && L2 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;

						check_merging = true;
					}
					else if( L2 < min_length && L1 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im > 0; im-- )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;

						check_merging = true;
					}
					else
					{
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;
					}
				}
				else if( pt_id == 0 )
				{
					Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
					for( int im = 0; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
						Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
					}
					Tracer->SnakeList.valid_list[j] = 0;

					check_merging = true;
				}
				else if( pt_id == Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
				{
					Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
					for( int im = Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1; im >= 0; im-- )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
						Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
					}
					Tracer->SnakeList.valid_list[j] = 0;

					check_merging = true;
				}

				//change the label image for snake selection
				IM->ImCoding( Tracer->SnakeList.Snakes[snake_label].Cu, Tracer->SnakeList.Snakes[snake_label].Ru, snake_label + 1, false );
				break;
			}
		}
	}

	//record the point id of current main branch
	vnl_vector<int> snake_point_id(Tracer->SnakeList.Snakes[snake_label].Cu.GetSize());
	snake_point_id.fill(0);
	//write the points of current main branch to swc file
	for( int i = 0; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
	{
		if( root_label == -1 )
		{

			*out_txt<<point_id[0]<<" ";
			*out_txt<<"3"<<" ";
			if( i == 0)
			{
				*out_txt<<root_point.x * ratio<<" ";
				*out_txt<<root_point.y * ratio<<" ";
				*out_txt<<root_point.z<<" ";
				*out_txt<<"0"<<" ";
				*out_txt<<root_label<<"\n";

				wrote_pt->AddPt(root_point.x, root_point.y, root_point.z);
			}
			else
			{
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";

				//*out_txt<<"0"<<" ";
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

				*out_txt<<point_id[0]-1<<"\n";
				wrote_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
			}

			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
		else
		{
			if( i == 0 )
				continue;


			if( i == Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() - 1 )
			{
				bool loop = false;
				for( int k = 0; k < wrote_pt->NP; k++ )
				{
					if( Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].GetDistTo( wrote_pt->Pt[k] ) <= collision_dist )
					{
						loop = true;
						break;
					}
				}
				if( loop )
					break;
			}

			//std::cout<<point_id[0]<<" "<<"3"<<" "<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" "<<
			//	Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" "<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z<<" "<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * ratio<<" "<<std::endl;

			*out_txt<<point_id[0]<<" ";
			*out_txt<<"3"<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";
			//*out_txt<<"0"<<" ";
			//*out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * sqrt((double)2)<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

			wrote_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);

			if( i == 1 )
				*out_txt<<root_label<<"\n";
			else
				*out_txt<<point_id[0]-1<<"\n";


			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
	}

	//find side branches and do recursion
	for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
	{
		if( snake_visit_label[0](j) == 1 )
			continue;
		if( Tracer->SnakeList.valid_list[j] == 0 )
			continue;

		vnl_vector<float> tail_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
		tail_dist.fill(100000);
		vnl_vector<float> head_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
		head_dist.fill(100000);
		for( int i = 1; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
		{ 
			if( snake_point_id(i) == 0 )
				continue;
			tail_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetFirstPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
			head_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
		}

		if( tail_dist.min_value() <= collision_dist )
		{
			int id_temp = tail_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);

			findBranch_MG(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt);
		}
		else if( head_dist.min_value() <= collision_dist )
		{
			int id_temp = head_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);

			findBranch_MG(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt);
		}

	}
}


void QtTracer::outputSWC_Raw()
{

	//int close_branch_point_dist = 3;

	int ratio = general_para3->getScale();
	float offset_x = general_para3->getX();
	float offset_y = general_para3->getY();
	float offset_z = general_para3->getZ();

	QString fileName;
	if( !batch_processI && !batch_processII && !automated_process )
	{
		fileName = QFileDialog::getSaveFileName(this, tr("Save Snakes to swc file"),
			"snake_tracing.swc",
			tr("Text File(*.swc)"));
	}
	else if( !automated_process )
	{
		fileName = images_dir[current_idx];
		fileName.replace(QString("tiff"), QString("swc"));
		fileName.replace(QString("tif"), QString("swc"));
		fileName.prepend("/");
		fileName.prepend(swcs_path);
	}
	else
	{
		fileName.setNum(IM->SeedPt.NP);
		fileName.append(".swc");
	}

	QFile swc_file(fileName);

	if (swc_file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream *out_txt;
		out_txt = new QTextStream(&swc_file);

		vnl_vector<int> *snake_visit_label;
		snake_visit_label = new vnl_vector<int>(Tracer->SnakeList.NSnakes);
		snake_visit_label->fill(0);
		int *point_id;
		point_id = new int[1];
		point_id[0] = 1;


		All_Pt.RemoveAllPts();

		std::vector<int> *branch_label;
		branch_label = new std::vector<int>[1];

		for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
		{
			if( snake_visit_label[0](i) == 1 )
				continue;
			if( Tracer->SnakeList.valid_list[i] == 0 )
				continue;

			wrote_pt.RemoveAllPts();

			int snake_id = i;
			findBranch_Raw( snake_id, -1, Tracer->SnakeList.Snakes[i].Cu.GetFirstPt(), snake_visit_label, point_id, out_txt, &wrote_pt, &All_Pt, branch_label );
		}

		/*PointList3D interest_points;
		interest_points.NP = 0;

		for( unsigned int i = 0; i < branch_label->size(); i++ )
		{
		if( branch_label[0][i] == 0 )
		continue;
		if( branch_label[0][i] > 1 )
		{
		interest_points.AddPt( All_Pt.Pt[i] );
		continue;
		}

		//find if this branch point is close to another branch point
		for( unsigned int j = 0; j < branch_label->size(); j++ )
		{
		if( j == i || branch_label[0][j] == 0 )
		continue;
		float dist_temp = All_Pt.Pt[i].GetDistTo( All_Pt.Pt[j] );
		if( dist_temp <= close_branch_point_dist )
		{
		interest_points.AddPt( All_Pt.Pt[i] );
		break;;
		}
		}
		}

		tracingViewer->setInterestPoints(interest_points); */
	}

	this->statusBar()->showMessage("SWC Saved");

}

void QtTracer::findBranch_Raw(int snake_label, int root_label, Point3D root_point, vnl_vector<int>*snake_visit_label, int *point_id, QTextStream *out_txt, PointList3D *wrote_pt, PointList3D *all_pt, std::vector<int> *branch_label )
{
	int collision_dist = general_para3->getConnection();
	//int ratio = general_para3->getScale();
	int ratio = general_para->getSf() * general_para->getSh();
	int min_length = general_para3->getMinLength();

	snake_visit_label[0](snake_label) = 1;

	if( root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt()) < 
		root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetFirstPt()))
	{
		//Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
		Tracer->SnakeList.Snakes[snake_label].Ru = Tracer->SnakeList.Snakes[snake_label].Cu.Flip_4D(Tracer->SnakeList.Snakes[snake_label].Ru);
	}

	/*//check the root tip for possible merging first
	if( root_label == -1 )
	{

	for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
	{

	if( snake_visit_label[0](j) == 1 )
	continue;
	if( Tracer->SnakeList.valid_list[j] == 0 )
	continue;

	vnl_vector<float> temp_dist(Tracer->SnakeList.Snakes[j].Cu.GetSize());
	for( int i = 0; i < Tracer->SnakeList.Snakes[j].Cu.GetSize(); i++ )
	{
	temp_dist(i) = Tracer->SnakeList.Snakes[snake_label].Cu.GetFirstPt().GetDistTo(Tracer->SnakeList.Snakes[j].Cu.Pt[i]);
	}

	if( temp_dist.min_value() <= collision_dist )
	{
	int pt_id = temp_dist.arg_min();

	if( pt_id != 0 && pt_id != Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
	{

	float L1 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,0);
	float L2 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,1);

	//rearrangement
	if( L1 >= min_length && L2 >= min_length )
	{

	//change root point
	root_point = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt();
	//add points to current snake
	for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
	{
	Tracer->SnakeList.Snakes[snake_label].Cu.AddTailPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
	}
	//remove points for the snake
	//Tracer->SnakeList.Snakes[j].Cu.NP = pt_id + 1;
	Tracer->SnakeList.Snakes[j].Cu.Resize(pt_id+1);

	//Tracer->SnakeList.valid_list[j] = 0;
	//SnakeClass temp;
	//for( int k = 0; k < pt_id; k++ )
	//{
	// temp.Cu.AddPt( Tracer->SnakeList.Snakes[j].Cu.Pt[k] );
	//}
	//IM->ImCoding( temp.Cu, Tracer->SnakeList.NSnakes+1, false );
	//Tracer->SnakeList.AddSnake(temp);

	Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
	}
	//merging
	else if( L1 < min_length && L2 >= min_length )
	{
	//change root point
	root_point = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt();
	//add points to current snake
	for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
	{
	Tracer->SnakeList.Snakes[snake_label].Cu.AddTailPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
	}
	//delete the snake
	Tracer->SnakeList.valid_list[j] = 0;

	Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
	}
	else if( L2 < min_length && L1 >= min_length )
	{
	//change root point
	root_point = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt();
	//add points to current snake
	for( int im = pt_id; im > 0; im-- )
	{
	Tracer->SnakeList.Snakes[snake_label].Cu.AddTailPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
	}
	//delete the snake
	Tracer->SnakeList.valid_list[j] = 0;

	Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
	}
	else
	{
	//delete the snake
	Tracer->SnakeList.valid_list[j] = 0;
	}
	}
	else if( pt_id == 0 )
	{
	//change root point
	root_point = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt();
	for( int im = 0; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
	{
	Tracer->SnakeList.Snakes[snake_label].Cu.AddTailPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
	}
	Tracer->SnakeList.valid_list[j] = 0;

	Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
	}
	else if( pt_id == Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
	{
	//change root point
	root_point = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt();
	for( int im = Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1; im >= 0; im-- )
	{
	Tracer->SnakeList.Snakes[snake_label].Cu.AddTailPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
	}
	Tracer->SnakeList.valid_list[j] = 0;

	Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
	}

	////change the label image for snake selection
	//IM->ImCoding( Tracer->SnakeList.Snakes[snake_label].Cu, snake_label + 1, false );
	break;
	}
	}
	} */

	bool check_merging = true; 
	while( check_merging ) 
	{
		check_merging = false;
		//find out if the tip (not the root point) of current main branch is actually a branch point
		for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
		{
			if( snake_visit_label[0](j) == 1 )
				continue;
			if( Tracer->SnakeList.valid_list[j] == 0 )
				continue;

			vnl_vector<float> temp_dist(Tracer->SnakeList.Snakes[j].Cu.GetSize());
			for( int i = 0; i < Tracer->SnakeList.Snakes[j].Cu.GetSize(); i++ )
			{
				temp_dist(i) = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[j].Cu.Pt[i]);
			}

			if( temp_dist.min_value() <= collision_dist )
			{
				int pt_id = temp_dist.arg_min();

				if( pt_id != 0 && pt_id != Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
				{

					float L1 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,0);
					float L2 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,1);

					//rearrangement
					if( L1 >= min_length && L2 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//remove points for the snake
						//Tracer->SnakeList.Snakes[j].Cu.NP = pt_id + 1;
						Tracer->SnakeList.Snakes[j].Cu.Resize(pt_id+1);
						Tracer->SnakeList.Snakes[j].Ru.resize(pt_id+1);
						//Tracer->SnakeList.valid_list[j] = 0;
						//SnakeClass temp;
						//for( int k = 0; k < pt_id; k++ )
						//{
						// temp.Cu.AddPt( Tracer->SnakeList.Snakes[j].Cu.Pt[k] );
						//}
						//IM->ImCoding( temp.Cu, Tracer->SnakeList.NSnakes+1, false );
						//Tracer->SnakeList.AddSnake(temp);
					}
					//merging
					else if( L1 < min_length && L2 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;

						check_merging = true;
					}
					else if( L2 < min_length && L1 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im > 0; im-- )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;

						check_merging = true;
					}
					else
					{
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;
					}
				}
				else if( pt_id == 0 )
				{
					Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
					for( int im = 0; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
						Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
					}
					Tracer->SnakeList.valid_list[j] = 0;

					check_merging = true;
				}
				else if( pt_id == Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
				{
					Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
					for( int im = Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1; im >= 0; im-- )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
						Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
					}
					Tracer->SnakeList.valid_list[j] = 0;

					check_merging = true;
				}

				//change the label image for snake selection
				IM->ImCoding( Tracer->SnakeList.Snakes[snake_label].Cu, Tracer->SnakeList.Snakes[snake_label].Ru, snake_label + 1, false );
				break;
			}
		}
	}

	//record the point id of current main branch
	vnl_vector<int> snake_point_id(Tracer->SnakeList.Snakes[snake_label].Cu.GetSize());
	snake_point_id.fill(0);
	//write the points of current main branch to swc file
	for( int i = 0; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
	{
		if( root_label == -1 )
		{

			*out_txt<<point_id[0]<<" ";
			*out_txt<<"2"<<" ";
			if( i == 0)
			{
				*out_txt<<root_point.x * ratio<<" ";
				*out_txt<<root_point.y * ratio<<" ";
				*out_txt<<root_point.z<<" ";
				*out_txt<<"0"<<" ";
				*out_txt<<root_label<<"\n";

				wrote_pt->AddPt(root_point.x, root_point.y, root_point.z);
				all_pt->AddPt(root_point.x, root_point.y, root_point.z);
				branch_label->push_back(1);
			}
			else
			{
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";

				//*out_txt<<"0"<<" ";
				*out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

				*out_txt<<point_id[0]-1<<"\n";
				wrote_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
				all_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
				branch_label->push_back(0);
			}

			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
		else
		{
			if( i == 0 )
				continue;


			if( i == Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() - 1 )
			{
				bool loop = false;
				for( int k = 0; k < wrote_pt->NP; k++ )
				{
					if( Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].GetDistTo( wrote_pt->Pt[k] ) <= collision_dist )
					{
						loop = true;
						break;
					}
				}
				if( loop )
					break;
			}

			*out_txt<<point_id[0]<<" ";
			*out_txt<<"2"<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x * ratio<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y * ratio<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z<<" ";
			//*out_txt<<"0"<<" ";
			//*out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * sqrt((double)2)<<" ";
			*out_txt<<Tracer->SnakeList.Snakes[snake_label].Ru[i] * ratio<<" ";

			wrote_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
			all_pt->AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y, Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
			branch_label->push_back(0);

			if( i == 1 )
				*out_txt<<root_label<<"\n";
			else
				*out_txt<<point_id[0]-1<<"\n";

			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
	}

	//find side branches and do recursion
	for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
	{
		if( snake_visit_label[0](j) == 1 )
			continue;
		if( Tracer->SnakeList.valid_list[j] == 0 )
			continue;

		vnl_vector<float> tail_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
		tail_dist.fill(100000);
		vnl_vector<float> head_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
		head_dist.fill(100000);
		for( int i = 1; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
		{ 
			if( snake_point_id(i) == 0 )
				continue;
			tail_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetFirstPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
			head_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
		}

		if( tail_dist.min_value() <= collision_dist )
		{
			int id_temp = tail_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);
			branch_label[0][new_root_label-1] += 1;
			findBranch_Raw(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt, all_pt, branch_label);
		}
		else if( head_dist.min_value() <= collision_dist )
		{
			int id_temp = head_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);
			branch_label[0][new_root_label-1] += 1;
			findBranch_Raw(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, out_txt, wrote_pt, all_pt, branch_label);
		}

	}
}

void QtTracer::findBranch(int snake_label, int root_label, Point3D root_point, vnl_vector<int>*snake_visit_label, int *point_id, SnakeTree *snake_tree)
{
	int collision_dist = general_para3->getConnection();
	int min_length = general_para3->getMinLength();

	snake_visit_label[0](snake_label) = 1;
	snake_tree->snake_id.push_back(snake_label);

	if( root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt()) < 
		root_point.GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.GetFirstPt()))
	{  
		//Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
		Tracer->SnakeList.Snakes[snake_label].Ru  = Tracer->SnakeList.Snakes[snake_label].Cu.Flip_4D(Tracer->SnakeList.Snakes[snake_label].Ru);
	}

	//find out if the tip (not the root point) of current main branch is actually a branch point
	bool check_merging = true; 
	while( check_merging ) 
	{
		check_merging = false;
		//find out if the tip (not the root point) of current main branch is actually a branch point
		for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
		{
			if( snake_visit_label[0](j) == 1 )
				continue;
			if( Tracer->SnakeList.valid_list[j] == 0 )
				continue;

			vnl_vector<float> temp_dist(Tracer->SnakeList.Snakes[j].Cu.GetSize());
			for( int i = 0; i < Tracer->SnakeList.Snakes[j].Cu.GetSize(); i++ )
			{
				temp_dist(i) = Tracer->SnakeList.Snakes[snake_label].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[j].Cu.Pt[i]);
			}

			if( temp_dist.min_value() <= collision_dist )
			{
				int pt_id = temp_dist.arg_min();

				if( pt_id != 0 && pt_id != Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
				{

					float L1 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,0);
					float L2 = Tracer->SnakeList.Snakes[j].Cu.GetPartLength(pt_id,1);

					//rearrangement
					if( L1 >= min_length && L2 >= min_length )
					{

						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//remove points for the snake
						//Tracer->SnakeList.Snakes[j].Cu.NP = pt_id + 1;
						Tracer->SnakeList.Snakes[j].Cu.Resize(pt_id+1);
						Tracer->SnakeList.Snakes[j].Ru.resize(pt_id+1);

						//Tracer->SnakeList.valid_list[j] = 0;
						//SnakeClass temp;
						//for( int k = 0; k < pt_id; k++ )
						//{
						// temp.Cu.AddPt( Tracer->SnakeList.Snakes[j].Cu.Pt[k] );
						//}
						//IM->ImCoding( temp.Cu, Tracer->SnakeList.NSnakes+1, false );
						//Tracer->SnakeList.AddSnake(temp);
					}
					//merging
					else if( L1 < min_length && L2 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;

						check_merging = true;
					}
					else if( L2 < min_length && L1 >= min_length )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
						Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
						//add points to current snake
						for( int im = pt_id; im > 0; im-- )
						{
							Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
							Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
						}
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;

						check_merging = true;
					}
					else
					{
						//delete the snake
						Tracer->SnakeList.valid_list[j] = 0;
					}
				}
				else if( pt_id == 0 )
				{
					Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
					for( int im = 0; im < Tracer->SnakeList.Snakes[j].Cu.GetSize(); im++ )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
						Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
					}
					Tracer->SnakeList.valid_list[j] = 0;

					check_merging = true;
				}
				else if( pt_id == Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1 )
				{
					Tracer->SnakeList.Snakes[snake_label].Cu.RemovePt(); //remove the last point
					Tracer->SnakeList.Snakes[snake_label].Ru.pop_back();
					for( int im = Tracer->SnakeList.Snakes[j].Cu.GetSize() - 1; im >= 0; im-- )
					{
						Tracer->SnakeList.Snakes[snake_label].Cu.AddPt(Tracer->SnakeList.Snakes[j].Cu.Pt[im]);
						Tracer->SnakeList.Snakes[snake_label].Ru.push_back(Tracer->SnakeList.Snakes[j].Ru[im]);
					}
					Tracer->SnakeList.valid_list[j] = 0;

					check_merging = true;
				}

				//change the label image for snake selection
				IM->ImCoding( Tracer->SnakeList.Snakes[snake_label].Cu, Tracer->SnakeList.Snakes[snake_label].Ru, snake_label + 1, false );
				break;
			}
		}
	}

	//record the point id of current main branch
	vnl_vector<int> snake_point_id(Tracer->SnakeList.Snakes[snake_label].Cu.GetSize());
	snake_point_id.fill(0);
	//write the points of current main branch to swc file
	for( int i = 0; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
	{
		if( root_label == -1 )
		{
			if( i == 0 )
			{
				snake_tree->points.AddPt(root_point.x, root_point.y, root_point.z);
				snake_tree->parent_list.push_back(root_label);
				snake_tree->Ru.push_back(Tracer->SnakeList.Snakes[snake_label].Ru[0]);

			}
			else
			{
				snake_tree->points.AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x,
					Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y,
					Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
				snake_tree->Ru.push_back(Tracer->SnakeList.Snakes[snake_label].Ru[i]);
				snake_tree->parent_list.push_back(point_id[0]-1);
			}

			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
		else
		{
			if( i == 0 )
				continue;


			if( i == Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() - 1 )
			{
				bool loop = false;
				for( int k = 0; k < snake_tree->points.NP; k++ )
				{
					if( Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].GetDistTo( snake_tree->points.Pt[k] ) <= collision_dist )
					{
						loop = true;
						break;
					}
				}
				if( loop )
					continue;
			}

			snake_tree->points.AddPt(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].x,
				Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].y,
				Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].z);
			snake_tree->Ru.push_back(Tracer->SnakeList.Snakes[snake_label].Ru[i]);
			if( i == 1 )
				snake_tree->parent_list.push_back(root_label);
			else
				snake_tree->parent_list.push_back(point_id[0]-1);
			snake_point_id(i) = point_id[0];
			point_id[0]++;
		}
	}

	//find side branches and do recursion
	for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
	{
		if( snake_visit_label[0](j) == 1 )
			continue;
		if( Tracer->SnakeList.valid_list[j] == 0 )
			continue;

		vnl_vector<float> tail_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
		tail_dist.fill(100000);
		vnl_vector<float> head_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
		head_dist.fill(100000);
		for( int i = 1; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); i++ )
		{ 
			if( snake_point_id(i) == 0 )
				continue;

			tail_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetFirstPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
			head_dist(i) = Tracer->SnakeList.Snakes[j].Cu.GetLastPt().GetDistTo(Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]);
		}

		if( tail_dist.min_value() <= collision_dist )
		{
			int id_temp = tail_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);
			snake_tree->branch_point.AddPt(new_root_point);
			findBranch(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, snake_tree);
		}
		else if( head_dist.min_value() <= collision_dist )
		{
			int id_temp = head_dist.arg_min();
			int new_snake_id = j;
			Point3D new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[id_temp]; 
			int new_root_label = snake_point_id(id_temp);
			snake_tree->branch_point.AddPt(new_root_point);
			findBranch(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, snake_tree);
		}

	}

	/*Point3D new_root_point;
	for( int i = 1; i < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() - 1; i++ )
	{
	int new_snake_id = -1;
	new_root_point = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i]; 
	int new_root_label = snake_point_id(i);

	for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
	{
	if( snake_visit_label[0](j) == 1 )
	continue;
	if( Tracer->SnakeList.valid_list[j] == 0 )
	continue;

	float head_dist = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].GetDistTo(Tracer->SnakeList.Snakes[j].Cu.GetLastPt());
	float tail_dist = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[i].GetDistTo(Tracer->SnakeList.Snakes[j].Cu.GetFirstPt());
	if( head_dist <= collision_dist || tail_dist <= collision_dist)
	{
	//the point must be nearest to the tip point of another snake
	if( head_dist < tail_dist )
	{
	vnl_vector<float> tip_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
	for( int z = 0; z < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); z++ )
	{
	tip_dist(z) = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[z].GetDistTo(Tracer->SnakeList.Snakes[j].Cu.GetLastPt());
	}
	//if current point is not the nearest, break;
	if( head_dist != tip_dist.min_value() )
	break;
	}
	if( tail_dist < head_dist )
	{
	vnl_vector<float> tip_dist( Tracer->SnakeList.Snakes[snake_label].Cu.GetSize() );
	for( int z = 0; z < Tracer->SnakeList.Snakes[snake_label].Cu.GetSize(); z++ )
	{
	tip_dist(z) = Tracer->SnakeList.Snakes[snake_label].Cu.Pt[z].GetDistTo(Tracer->SnakeList.Snakes[j].Cu.GetFirstPt());
	}
	//if current point is not the nearest, continue;
	if( tail_dist != tip_dist.min_value() )
	break;
	}

	new_snake_id = j;

	//if( head_dist < tail_dist )
	// Tracer->SnakeList.Snakes[j].Cu.Flip();

	break;
	}
	}

	//recursion
	if( new_snake_id != -1 )
	{
	//add branch point to tree
	snake_tree->branch_point.AddPt(new_root_point);
	findBranch(new_snake_id, new_root_label, new_root_point, snake_visit_label, point_id, snake_tree);
	}
	} */


	/* int collision_dist = 2;
	int ratio = general_para->getSh();

	SnakeClass snake;
	snake = Tracer->SnakeList.Snakes[snake_label];

	snake_visit_label[0](snake_label) = 1;
	if( root_point.GetDistTo(snake.Cu.GetLastPt()) < 
	root_point.GetDistTo(snake.Cu.GetFirstPt()))
	{
	snake.Cu.Flip();
	Tracer->SnakeList.Snakes[snake_label].Cu.Flip();
	}

	//record the point id of current main branch
	vnl_vector<int> snake_point_id(snake.Cu.GetSize());
	//write the points of current main branch to swc file
	for( int i = 0; i < snake.Cu.GetSize(); i++ )
	{
	*out_txt<<point_id[0]<<" ";
	*out_txt<<"2"<<" ";
	if( i == 0)
	{
	*out_txt<<root_point.x * ratio<<" ";
	*out_txt<<root_point.y * ratio<<" ";
	*out_txt<<root_point.z<<" ";
	}
	else
	{
	*out_txt<<snake.Cu.Pt[i].x * ratio<<" ";
	*out_txt<<snake.Cu.Pt[i].y * ratio<<" ";
	*out_txt<<snake.Cu.Pt[i].z<<" ";
	}
	*out_txt<<"0"<<" ";
	if( i == 0 )
	*out_txt<<root_label<<"\n";
	else
	*out_txt<<point_id[0]-1<<"\n";

	snake_point_id(i) = point_id[0];

	point_id[0]++;
	}

	//find side branches and do recursion
	Point3D new_root_point;
	for( int i = 0; i < snake.Cu.GetSize(); i++ )
	{
	int new_snake_id = -1;
	new_root_point = snake.Cu.Pt[i]; 
	int new_root_label = snake_point_id(i);

	for( int j = 0; j < Tracer->SnakeList.NSnakes; j++ )
	{
	if( snake_visit_label[0](j) == 1 )
	continue;

	float head_dist = snake.Cu.Pt[i].GetDistTo(Tracer->SnakeList.Snakes[j].Cu.GetLastPt());
	float tail_dist = snake.Cu.Pt[i].GetDistTo(Tracer->SnakeList.Snakes[j].Cu.GetFirstPt());
	if( head_dist <= collision_dist || tail_dist <= collision_dist)
	{
	new_snake_id = j;
	if( head_dist < tail_dist )
	Tracer->SnakeList.Snakes[j].Cu.Flip();
	continue;
	}

	}

	//recursion
	if( new_snake_id != -1 )
	{
	writeBranch(new_snake_id, new_root_label, new_root_point, snake_visit_label, out_txt, point_id);
	}
	} */
}


void QtTracer::outputVTK()
{

	int collision_dist = 2;

	//int ratio = general_para3->getScale();
	int ratio = general_para->getSf() * general_para->getSh();
	int offset_x = general_para3->getX();
	int offset_y = general_para3->getY();
	int offset_z = general_para3->getZ();

	bool int_point = false;

	QString fileName;
	if( !batch_processI && !batch_processII )
	{
		fileName = QFileDialog::getSaveFileName(this, tr("Save Snakes to vtk file"),
			"snake_tracing.vtk",
			tr("Text File(*.vtk)"));
	}
	else
	{
		fileName = images_dir[current_idx];
		fileName.replace(QString("tiff"), QString("swc"));
		fileName.replace(QString("tif"), QString("swc"));
		fileName.prepend("/");
		fileName.prepend(swcs_path);
	}



	QFile tracing_result(fileName);


	if (tracing_result.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream out_vtk(&tracing_result);
		out_vtk<<"# vtk DataFile Version 3.0"<<"\n";
		out_vtk<<"Tracing Result"<<"\n";
		out_vtk<<"ASCII"<<"\n";
		out_vtk<<"DATASET POLYDATA"<<"\n";


		int Num_P = 0;

		for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
		{
			for( int j = 0; j < Tracer->SnakeList.Snakes[i].Cu.GetSize(); j++ )
			{
				if( int_point )
				{ 
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].x = ceil(Tracer->SnakeList.Snakes[i].Cu.Pt[j].x);
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].y = ceil(Tracer->SnakeList.Snakes[i].Cu.Pt[j].y);
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].z = ceil(Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
				}
				Num_P++;
			}
		}


		PointList3D wrote_Pt;
		//wrote_Pt.SetN(Num_P);

		PointList3D end_Pt;
		//end_Pt.SetN(Num_P);

		PointList3D end_ID;
		//end_ID.SetN(Num_P);

		vnl_vector<int> *Snake_ID;
		Snake_ID = new vnl_vector<int>[Tracer->SnakeList.NSnakes];

		SnakeListClass SnakeList = Tracer->SnakeList;


		for( int i = 0;  i < SnakeList.NSnakes; i++ )
		{  
			SnakeClass snake = SnakeList.Snakes[i];

			int NP = snake.Cu.GetSize();

			vnl_vector<int> temp_snake_id(NP);

			if( snake.Cu.GetSize() == 0 || SnakeList.valid_list[i] == 0 )
			{
				temp_snake_id.fill(-1);
				Snake_ID[i] = temp_snake_id;
				continue;
			}

			if( end_Pt.NP == 0 )
			{
				end_Pt.AddPt(snake.Cu.GetFirstPt());
				end_Pt.AddPt(snake.Cu.GetLastPt());
				end_ID.AddPt(0,0,0);
				end_ID.AddPt(NP-1,NP-1,NP-1);
				for( int k = 0; k < NP; k++ )
				{
					wrote_Pt.AddPt(snake.Cu.Pt[k]);
					temp_snake_id(k) = k;
				}
				Snake_ID[i] = temp_snake_id;
				continue;
			}

			PointList3D wrote_Pt_old;
			//wrote_Pt_old.SetN(Num_P);
			wrote_Pt_old = wrote_Pt;

			//body	
			for( int j = 1; j < NP-1; j++)
		 {
			 vnl_vector<float> distance(end_Pt.NP);
			 for( int k = 0; k < end_Pt.NP; k++)
			 {
				 distance(k) = snake.Cu.Pt[j].GetDistTo(end_Pt.Pt[k]); 

			 }
			 if( distance.min_value() <= collision_dist )
			 {
				 int temp_idx = distance.arg_min();
				 temp_snake_id(j) = end_ID.Pt[temp_idx].x;
				 end_Pt.RemovePt(temp_idx);
				 end_ID.RemovePt(temp_idx);
			 }
			 else
			 {
				 temp_snake_id(j) = wrote_Pt.NP;
				 wrote_Pt.AddPt(snake.Cu.Pt[j]);
			 }
		 }

			vnl_vector<float>distance(wrote_Pt_old.NP);

			//tail
			for( int j = 0; j < wrote_Pt_old.NP; j++ )
		 {
			 distance(j) = snake.Cu.Pt[0].GetDistTo(wrote_Pt_old.Pt[j]);
		 }

			if( distance.min_value() <= collision_dist )
		 {
			 int temp_idx = distance.arg_min();
			 temp_snake_id(0) = temp_idx;
		 }
			else
		 {
			 temp_snake_id(0) = wrote_Pt.NP;
			 end_Pt.AddPt(snake.Cu.Pt[0]);
			 end_ID.AddPt(temp_snake_id(0),temp_snake_id(0),temp_snake_id(0));
			 wrote_Pt.AddPt(snake.Cu.Pt[0]);
		 }

			//head
			for( int j = 0; j < wrote_Pt_old.NP; j++ )
		 {
			 distance(j) = snake.Cu.Pt[NP-1].GetDistTo(wrote_Pt_old.Pt[j]);
		 }

			if( distance.min_value() <= collision_dist )
		 {
			 int temp_idx = distance.arg_min();
			 temp_snake_id(NP-1) = temp_idx;
		 }
			else
		 {
			 temp_snake_id(NP-1) = wrote_Pt.NP;
			 end_Pt.AddPt(snake.Cu.Pt[NP-1]);
			 end_ID.AddPt(temp_snake_id(NP-1),temp_snake_id(NP-1),temp_snake_id(NP-1));
			 wrote_Pt.AddPt(snake.Cu.Pt[NP-1]);
		 }

			Snake_ID[i] = temp_snake_id;
		}


		//wrote points
		out_vtk<<"POINTS"<<" "<<wrote_Pt.NP<<" "<<"float"<<"\n";
		for( int i = 0; i< wrote_Pt.NP; i++ )
		{
			out_vtk<<wrote_Pt.Pt[i].x * ratio + offset_x<<" "<<wrote_Pt.Pt[i].y * ratio + offset_y<<" "<<wrote_Pt.Pt[i].z + offset_z;
			out_vtk<<"\n";
		}

		int Line_No = 0;
		int Line_Entry = 0;
		for( int i = 0; i< SnakeList.NSnakes; i++ )
		{ 
			Line_No += SnakeList.Snakes[i].Cu.GetSize() - 1;
		}
		Line_Entry += Line_No * 3;

		out_vtk<<"LINES"<<" "<<Line_No<<" "<<Line_Entry;
		out_vtk<<"\n";

		int const_num = 2;

		for( int i = 0; i< SnakeList.NSnakes; i++ )
		{ 
			if( Snake_ID[i].size() == 1)
				continue;

			for(unsigned int j = 0; j < Snake_ID[i].size() - 1; j++ )
			{
				out_vtk<<const_num<<" ";
				out_vtk<<Snake_ID[i](j)<<" ";
				out_vtk<<Snake_ID[i](j+1)<<" ";
				if( i != SnakeList.NSnakes - 1 || j != Snake_ID[i].size() - 2 )
					out_vtk<<"\n";
			}
		}

	}
}


void QtTracer::outputXLS()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Tree Features"),
		"TreeFeatures.xls",
		tr("Text File(*.xls)"));

	QFile Feature_XLS(fileName);
	if (Feature_XLS.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream out_XLS(&Feature_XLS);


		//out_XLS<<"Tree Features"<<"\t";
		//for(int j = 0; j < model_sum->columnCount(); j++)
		//{
		//	out_XLS <<model_sum->headerData(j, Qt::Horizontal).toString()<<"\t";
		//}
		//out_XLS<<"\n";

		for( int i = 0; i < tree_features.size(); i++ )
		{
			out_XLS<<i+1<<"\t";
			out_XLS<<tree_features[i].SomaVolume<<"\t";
			out_XLS<<tree_features[i].N_Stems<<"\t";
			out_XLS<<tree_features[i].TotalLength<<"\t";
			out_XLS<<tree_features[i].N_Branches<<"\t";
			out_XLS<<tree_features[i].N_Bifs<<"\t";
			out_XLS<<tree_features[i].N_Tips<<"\t";
			out_XLS<<tree_features[i].A_Diameter<<"\t";
			out_XLS<<tree_features[i].A_SectionArea<<"\t";
			out_XLS<<tree_features[i].Volume<<"\t";
			out_XLS<<tree_features[i].Surface<<"\t";
			out_XLS<<tree_features[i].A_BranchOrder<<"\t";
			out_XLS<<tree_features[i].A_BifAngle<<"\t";
			out_XLS<<"\n";

		}
	}
}

void QtTracer::load_settings()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("Load Settings"),
		".", tr("Text File (*.txt)"));
	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) 
	{
		QMessageBox::information(this, tr("Warning"), tr("Cannot load this file"));
		return;
	}
	else
	{
		QTextStream in(&file);
		QStringList fields;

		QString line = in.readLine();
		line = in.readLine();
		fields = line.split(' ');
		general_para->setSf(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para->setSh(fields.takeLast().toInt());

		line = in.readLine();

		line = in.readLine();
		fields = line.split(' ');
		general_para12->setNoiseLevel(fields.takeLast().toDouble());
		line = in.readLine();
		fields = line.split(' ');
		general_para12->setNumIteration(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para12->setThreshold(fields.takeLast().toDouble());
		line = in.readLine();
		fields = line.split(' ');
		general_para12->setSeedAdjustment(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para12->setSeedRadius(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para12->setSmoothingScale(fields.takeLast().toInt());

		line = in.readLine();
		line = in.readLine();
		fields = line.split(' ');
		general_para2->setCurrentTracing(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para2->setCurrentForce(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para2->setStretchRatio(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para2->setGamma(fields.takeLast().toDouble());
		line = in.readLine();
		fields = line.split(' ');
		general_para2->setPtDistance(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para2->setMinimumLength(fields.takeLast().toInt());

		line = in.readLine();
		fields = line.split(' ');
		general_para2->setCollisionDist(fields.takeLast().toInt());

		line = in.readLine();
		fields = line.split(' ');
		general_para2->setSigmaRatio(fields.takeLast().toFloat());

		line = in.readLine();
		fields = line.split(' ');
		general_para2->setRemoveSeedRange(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para2->setAutomaticMerging(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para2->general_para21->setMaxAngle(fields.takeLast().toInt());

		line = in.readLine();
		line = in.readLine();
		fields = line.split(' ');
		general_para3->setConnection(fields.takeLast().toInt());
		line = in.readLine();
		fields = line.split(' ');
		general_para3->setMinLength(fields.takeLast().toInt());

	} 
}

void QtTracer::save_settings()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Save Settings"),
		"Settings.txt",
		tr("Text File(*.txt)"));
	QFile file(fileName);

	if (file.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream out(&file);

		out << "#Processing Methods:\n";
		out<<"-Scale Factor "<<general_para->getSf()<<endl;
		out<<"-Shrink Factor "<<general_para->getSh()<<endl;

		out << "#Preprocessing:\n";
		out<<"-Noise Level "<<general_para12->getNoiseLevel()<<endl;
		out<<"-Iteration Number "<<general_para12->getNumIteration()<<endl;
		out<<"-Threshold "<<general_para12->getThreshold()<<endl;
		out<<"-Seed Adjustment "<<general_para12->getSeedAdjustment()<<endl;
		out<<"-Seed Radius "<<general_para12->getSeedRadius()<<endl;
		out<<"-Smoothing Scale "<<general_para12->getSmoothingScale()<<endl;

		out << "#Tracing:\n";
		out<<"-Tracing Model "<<general_para2->getCurrentTracing()<<endl;
		out<<"-Stretching Force "<<general_para2->getCurrentForce()<<endl;
		out<<"-Stretch Ratio "<<general_para2->getStretchRatio()<<endl;
		out<<"-Gamma "<<general_para2->getGamma()<<endl;
		out<<"-Unit Length "<<general_para2->getPtDistance()<<endl;
		out<<"-Minimum Length "<<general_para2->getMinimumLength()<<endl;
		out<<"-Collision Distance "<<general_para2->getCollisionDist()<<endl;
		out<<"-Sigma Ratio "<<general_para2->getSigmaRatio()<<endl;
		out<<"-Radius for Removing Seed "<<general_para2->getRemoveSeedRange()<<endl;
		out<<"-Automatic Merging "<<general_para2->getAutomaticMerging()<<endl; 
		out<<"-Automatic Merging Max Angle "<<general_para2->general_para21->getMaxAngle()<<endl; 

		out << "#Output:\n";
		out<<"-Connection "<<general_para3->getConnection()<<endl;
		out<<"-Minimum Length "<<general_para3->getMinLength()<<endl;

	}
}


void QtTracer::setSelected_sels()
{
	if(selections)
	{
		QItemSelection itemSels;
		for(int i = 0; i < model->rowCount(); i++)
		{
			if(selections->isSelected(i))
			{
				QItemSelection itemSels_temp;
				QModelIndex index_start = model->index(i,0,QModelIndex());
				QModelIndex index_end = model->index(i,model->columnCount() - 1,QModelIndex());
				itemSels_temp.select(index_start,index_end);
				itemSels.merge(itemSels_temp,QItemSelectionModel::Select);
			}
		}

		disconnect(selectionModel,SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(setSelected(const QItemSelection &, const QItemSelection &)));

		selectionModel->select(itemSels, QItemSelectionModel::ClearAndSelect);

		connect(selectionModel,SIGNAL(selectionChanged(const QItemSelection &, const QItemSelection &)),
			this, SLOT(setSelected(const QItemSelection &, const QItemSelection &)));
	}
}

void QtTracer::setSelected(const QItemSelection &, const QItemSelection &)
{
	int *selectedRow = new int[model->rowCount()];
	//selections->clear();
	std::set<long int> ids;
	ids.clear();
	for(int i = 0; i < model->rowCount(); i++)
	{
		if(selectionModel->isRowSelected(i, QModelIndex()))
		{
			//selectedRow[i] = 1;
			ids.insert(i);
		}

		//else
		//selectedRow[i] = 0;
	}   

	disconnect(selections,SIGNAL(changed(void)), this, SLOT(setSelected_sels(void)));
	selections->select(ids);
	connect(selections,SIGNAL(changed(void)), this, SLOT(setSelected_sels(void)));
	//emit sendSelected(selectedRow);
}

void QtTracer::displayFeatures()
{
	float pi = 3.1415926;

	typedef itk::LinearInterpolateImageFunction< 
		ProbImageType, float>  InterpolatorType;
	typedef itk::NearestNeighborInterpolateImageFunction< 
		ImageType, float>  InterpolatorType1;
	InterpolatorType::Pointer IV_interpolator = InterpolatorType::New();
	IV_interpolator->SetInputImage(IM->IVessel);
	InterpolatorType1::Pointer I_interpolator = InterpolatorType1::New();
	I_interpolator->SetInputImage(IM->I);

	int NS = 0;
	for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
	{
		if( Tracer->SnakeList.valid_list[i] == 0 )
			continue;
		NS++;
	}

	model->setRowCount(NS);

	int snake_id = 0;

	for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
	{
		if( Tracer->SnakeList.valid_list[i] == 0 )
			continue;

		float Length = Tracer->SnakeList.Snakes[i].Cu.GetLength();
		model->setData(model->index(snake_id, 0, QModelIndex()), Length);

		//compute average radius, average intensity and average vesselness
		float Ru = 0.0;
		float Intensity = 0.0;
		float Vesselness = 0.0;
		int N = Tracer->SnakeList.Snakes[i].Ru.size();
		for( int j = 0; j < N; j++ )
		{
			Ru += Tracer->SnakeList.Snakes[i].Ru[j]/N;
			InterpolatorType::ContinuousIndexType index;
			index[0] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x;
			index[1] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y;
			index[2] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z;
			Intensity += I_interpolator->EvaluateAtContinuousIndex(index)/N;
			Vesselness += IV_interpolator->EvaluateAtContinuousIndex(index)/N;
		}
		model->setData(model->index(snake_id, 1, QModelIndex()),Ru);

		//compute the volume, surface, and section area
		float Volume = 0.0;
		float Surface = 0.0;
		float Section_Area = 0.0;


		for( int j = 0; j < N-1; j++ )
		{
			Volume += pow(Tracer->SnakeList.Snakes[i].Ru[j],2) * pi * Tracer->SnakeList.Snakes[i].Cu.Pt[j].GetDistTo(Tracer->SnakeList.Snakes[i].Cu.Pt[j+1]);
			Surface += 2 * pi * Tracer->SnakeList.Snakes[i].Ru[j] * Tracer->SnakeList.Snakes[i].Cu.Pt[j].GetDistTo(Tracer->SnakeList.Snakes[i].Cu.Pt[j+1]);
			Section_Area += pow(Tracer->SnakeList.Snakes[i].Ru[j],2) * pi/N;
		}
		model->setData(model->index(snake_id, 2, QModelIndex()), Volume);
		model->setData(model->index(snake_id, 3, QModelIndex()), Surface);
		model->setData(model->index(snake_id, 4, QModelIndex()), Section_Area);
		model->setData(model->index(snake_id, 5, QModelIndex()), N);

		/*//compute the curvature   
		float Curvature = 0.0;
		for( int j = 1; j < N-1; j++ )
		{ 
		float x1 = 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j].x - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].x) + 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].x - Tracer->SnakeList.Snakes[i].Cu.Pt[j].x);
		float y1 = 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j].y - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].y) + 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].y - Tracer->SnakeList.Snakes[i].Cu.Pt[j].y);
		float z1 = 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j].z - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].z) + 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].z - Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
		float x2 = 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].x + Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].x - 2 * Tracer->SnakeList.Snakes[i].Cu.Pt[j].x);
		float y2 = 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].y + Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].y - 2 * Tracer->SnakeList.Snakes[i].Cu.Pt[j].y);
		float z2 = 0.5 * (Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].z + Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].z - 2 * Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
		float temp = sqrt(pow(pow(x1,2)+pow(y1,2)+pow(z1,2),3));
		Curvature += sqrt(pow(z2*y1 - y2*z1,2) + pow(x2*z1 - z2*x1,2) + pow(y2*x1 - x2*y1,2))/temp;
		}
		Curvature /= N;*/
		//compute the tortuosity
		float Tortuosity = 0.0;
		Tortuosity = Length/Tracer->SnakeList.Snakes[i].Cu.GetFirstPt().GetDistTo(Tracer->SnakeList.Snakes[i].Cu.GetLastPt());

		model->setData(model->index(snake_id, 6, QModelIndex()), Tortuosity );
		model->setData(model->index(snake_id, 7, QModelIndex()), Intensity);
		model->setData(model->index(snake_id, 8, QModelIndex()), Vesselness);

		snake_id++;
	}

	scatterPlot->Refresh();
}


void QtTracer::showScatter()
{
	ScatterPlotDialog *dialog = new ScatterPlotDialog;
	dialog->setWindowTitle("Select X-Y Axes");
	dialog->show();

	scatterPlot->show();
	scatterPlot->setGeometry(QRect(50,50,450,450));
	connect(dialog, SIGNAL(x_changed(int)),
		scatterPlot, SLOT(SetColForX(int)));
	connect(dialog, SIGNAL(y_changed(int)),
		scatterPlot, SLOT(SetColForY(int)));
	show_table_scatter = true;
	displayFeatures();
}
void QtTracer::showTable()
{
	table->show();
	show_table_scatter = true;
	displayFeatures();
}


void QtTracer::loadSWC()
{

	QStringList files = QFileDialog::getOpenFileNames(
		this,
		"Select one or more SWC files to open",
		".",
		tr("SWC File (*.swc)"));

	QStringList::Iterator it = files.begin();
	while(it != files.end()) 
	{
		QFile file(*it);

		if (!file.open(QIODevice::ReadOnly)) 
		{
			QMessageBox::information(this, tr("Warning"), tr("Cannot load this file"));
			return;
		}
		else
		{
			float Pi = 3.1415926;

			SnakeListClass SWCSnakes;
			PointList3D BranchPt;

			PointList3D CenterPt;
			Point3D RootPt;
			float SomaRadius;
			std::vector<float> Ru;
			std::vector<int> Type;
			std::vector<int> ID;
			std::vector<int> PID;
			Point3D temp;
			QTextStream in(&file);
			QStringList fields;

			std::vector<int> break_id;
			std::vector<int> branch_id;

			float SomaVolume = 0.0;
			int N_Stems = 0;
			float TotalLength = 0.0;
			int N_Branches = 0;
			int N_Bifs = 0;
			int N_Tips = 0;
			float A_Diameter = 0.0;
			float A_SectionArea = 0.0;
			float Volume = 0.0;
			float Surface = 0.0;
			float A_BranchOrder = 0.0;
			float A_BifAngle = 0.0;

			std::vector<float> BranchOrders;
			std::vector<float> BirAngles;

			std::string str1("#");
			int blank = 0;
			while( blank < 2 )
			{
				QString line = in.readLine();

				fields = line.split(' ',QString::SkipEmptyParts);

				if( fields.size() == 0 )
				{
					//end_of_txt = true;
					//break;
					blank++;
					continue;
				}

				if( fields.at(0).toStdString().compare(str1) == 0 )
				{
					continue;
				}


				ID.push_back(fields.at(0).toInt());
				Type.push_back(fields.at(1).toInt());
				temp.x = fields.at(2).toFloat();
				temp.y = fields.at(3).toFloat();
				temp.z = fields.at(4).toFloat();

				CenterPt.AddPt(temp);
				Ru.push_back(fields.at(5).toFloat());
				PID.push_back(fields.at(6).toInt());


				if( fields.at(1).toInt() == 1 )
					SomaVolume += 4 * Pi * pow(fields.at(5).toFloat(),3)/3;

				if( fields.at(6).toInt() == -1 )
				{
					RootPt = temp;
					SomaRadius = fields.at(5).toFloat();
					break_id.push_back(0);
					branch_id.push_back(0);
				}
				else
				{
					//when the type change from 1 to 3, count it as the stem
					if( fields.at(1).toInt() == 3 && Type[fields.at(6).toInt()-1] == 1 )
						N_Stems++;

					if( fields.at(6).toInt() != fields.at(0).toInt() - 1 && fields.at(6).toInt() != 1 )
					{
						break_id.push_back(1);
						break_id[fields.at(6).toInt()-1] = 1;
						branch_id.push_back(0);
						branch_id[fields.at(6).toInt()-1] = 1;

						BranchPt.AddPt(CenterPt.Pt[fields.at(6).toInt()-1]);


						Vector3D V1(temp.x - CenterPt.Pt[fields.at(6).toInt()-1].x,
							temp.y - CenterPt.Pt[fields.at(6).toInt()-1].y,
							temp.z - CenterPt.Pt[fields.at(6).toInt()-1].z);

						Vector3D V2(CenterPt.Pt[fields.at(6).toInt()].x - CenterPt.Pt[fields.at(6).toInt()-1].x,
							CenterPt.Pt[fields.at(6).toInt()].y - CenterPt.Pt[fields.at(6).toInt()-1].y,
							CenterPt.Pt[fields.at(6).toInt()].z - CenterPt.Pt[fields.at(6).toInt()-1].z);

						float angle = V1.GetAngleTo(V2) * 360/(2*Pi);

						BirAngles.push_back(angle);

						N_Bifs++;
						N_Tips++;
					}
					else
					{
						break_id.push_back(0);
						branch_id.push_back(0);
					}
				}
			}


			for( int i = 0; i < BirAngles.size(); i++ )
				A_BifAngle += BirAngles[i]/BirAngles.size();

			for( int i = 0; i < Ru.size(); i++ )
			{
				A_Diameter += Ru[i];
				A_SectionArea += Pi * pow(Ru[i],2);
			}
			A_Diameter /= Ru.size();
			A_SectionArea /= Ru.size();

			N_Tips++;

			PointList3D temp_Cu;
			std::vector<float> temp_Ru;
			//std::vector<float> temp_Type;
			SnakeClass temp_Snake;
			for( int i = 0; i < CenterPt.NP; i++ )
			{
				if( PID[i] == 1 && i != 1 )  //new stem
				{
					temp_Snake.Cu = temp_Cu;
					temp_Snake.Ru = temp_Ru;
					//temp_Snake.Type = temp_Type;
					SWCSnakes.AddSnake(temp_Snake);

					//find the branch order of the added branch
					int temp_order = 0;
					int current_id = i - 1;
					if( current_id > 0)
					{
						while( PID[current_id] != -1 )
						{
							current_id = PID[current_id]-1;
							if( branch_id[current_id] == 1 )
								temp_order++;
						}
					}
					BranchOrders.push_back(temp_order);

					temp_Cu.RemoveAllPts();
					temp_Ru.clear();
					//temp_Type.clear();

					temp_Cu.AddPt(CenterPt.Pt[PID[i]-1]);
					temp_Ru.push_back(Ru[PID[i]-1]);
					//temp_Type.push_back(Type[PID[i]-1]);
					temp_Cu.AddPt(CenterPt.Pt[i]);
					temp_Ru.push_back(Ru[i]);

					continue;
				}

				if( i == CenterPt.NP-1 )  //end of the swc file
				{
					temp_Snake.Cu = temp_Cu;
					temp_Snake.Ru = temp_Ru;
					//temp_Snake.Type = temp_Type;
					SWCSnakes.AddSnake(temp_Snake);
					//find the branch order of the added branch
					int temp_order = 0;
					int current_id = i - 1;
					if( current_id > 0)
					{
						while( PID[current_id] != -1 )
						{
							current_id = PID[current_id]-1;
							if( branch_id[current_id] == 1 )
								temp_order++;
						}
					}
					BranchOrders.push_back(temp_order);
					continue;
				}

				if( break_id[i] == 0 )
				{
					temp_Cu.AddPt(CenterPt.Pt[i]);
					temp_Ru.push_back(Ru[i]);
					//temp_Type.push_back(Type[i]);
				}
				else
				{
					if( PID[i] == ID[i] - 1 )
					{		  
						temp_Cu.AddPt(CenterPt.Pt[i]);
						temp_Ru.push_back(Ru[i]);
						//temp_Type.push_back(Type[i]);
						temp_Snake.Cu = temp_Cu;
						temp_Snake.Ru = temp_Ru;
						//temp_Snake.Type = temp_Type;
						SWCSnakes.AddSnake(temp_Snake);

						//find the branch order of the added branch
						int temp_order = 0;
						int current_id = i - 1;
						if( current_id > 0)
						{
							while( PID[current_id] != -1 )
							{
								current_id = PID[current_id]-1;
								if( branch_id[current_id] == 1 )
									temp_order++;
							}
						}
						BranchOrders.push_back(temp_order);

						temp_Cu.RemoveAllPts();
						temp_Ru.clear();
						//temp_Type.clear();

						temp_Cu.AddPt(CenterPt.Pt[i]);
						temp_Ru.push_back(Ru[i]);
						//temp_Type.push_back(Type[i]);
					}
					else
					{
						temp_Snake.Cu = temp_Cu;
						temp_Snake.Ru = temp_Ru;
						//temp_Snake.Type = temp_Type;
						SWCSnakes.AddSnake(temp_Snake);

						//find the branch order of the added branch
						int temp_order = 0;
						int current_id = i - 1;
						if( current_id > 0)
						{
							while( PID[current_id] != -1 )
							{
								current_id = PID[current_id]-1;
								if( branch_id[current_id] == 1 )
									temp_order++;
							}
						}
						BranchOrders.push_back(temp_order);

						temp_Cu.RemoveAllPts();
						temp_Ru.clear();
						//temp_Type.clear();

						temp_Cu.AddPt(CenterPt.Pt[PID[i]-1]);
						temp_Ru.push_back(Ru[PID[i]-1]);
						//temp_Type.push_back(Type[PID[i]-1]);
						temp_Cu.AddPt(CenterPt.Pt[i]);
						temp_Ru.push_back(Ru[i]);
						//temp_Type.push_back(Type[i]);
					}
				}

			}

			for( int i = 0; i < BranchOrders.size(); i++ )
			{
				A_BranchOrder += BranchOrders[i]/BranchOrders.size();
			}

			for( int i = 0; i < SWCSnakes.NSnakes; i++ )
			{
				TotalLength += SWCSnakes.Snakes[i].Cu.GetLength();
				for( int j = 0; j < SWCSnakes.Snakes[i].Cu.NP - 1 ; j++ )
				{
					Volume += Pi * pow(SWCSnakes.Snakes[i].Ru[j],2) * SWCSnakes.Snakes[i].Cu.Pt[j].GetDistTo(SWCSnakes.Snakes[i].Cu.Pt[j+1]);
					Surface += 2 * Pi * SWCSnakes.Snakes[i].Ru[j] * SWCSnakes.Snakes[i].Cu.Pt[j].GetDistTo(SWCSnakes.Snakes[i].Cu.Pt[j+1]);
				}
			}

			N_Branches = SWCSnakes.NSnakes;

			//compute the features
			TreeFeature features;
			features.SomaVolume = SomaVolume;
			features.N_Stems = N_Stems;
			features.TotalLength = TotalLength;
			features.N_Branches = N_Branches;
			features.N_Bifs = N_Bifs;
			features.N_Tips = N_Tips;
			features.A_Diameter = A_Diameter;
			features.A_SectionArea = A_SectionArea;
			features.Volume = Volume;
			features.Surface = Surface;
			features.A_BranchOrder = A_BranchOrder;
			features.A_BifAngle = A_BifAngle;

			tree_features.push_back(features);

			double color[3];
			if( tree_features.size() == 1 )
			{
				color[0] = 0;
				color[1] = 0;
				color[2] = 1;
			}
			else if( tree_features.size() == 2 )
			{
				color[0] = 1;
				color[1] = 0;
				color[2] = 0;
			}
			/*else if( tree_features.size() == 2 )
			{
			color[0] = 0;
			color[1] = 1;
			color[2] = 0;
			}
			else if( tree_features.size() == 3 )
			{
			color[0] = 1;
			color[1] = 0;
			color[2] = 0;
			}*/

			displaySWC(SWCSnakes, RootPt, SomaRadius, BranchPt, color);

			snake_tree_swc = new SnakeTree_SWC;
			snake_tree_swc->Snakes = SWCSnakes;
			snake_tree_swc->RootPt = RootPt;
			snake_tree_swc->soma_radius = SomaRadius;
			snake_tree_swc->BranchPt = BranchPt;

			tracingViewer->setSnakeTree_SWC(snake_tree_swc);

			createTreeTables();
		} 
		++it;
	}
}

void QtTracer::loadSWC_F() //load the swc saved automatically after setting root point and compute the tree features
{
	QString fileName("snake_tracing.swc");

	QFile file(fileName);
	if (!file.open(QIODevice::ReadOnly)) 
	{
		QMessageBox::information(this, tr("Warning"), tr("Cannot load this file"));
		return;
	}
	else
	{
		float Pi = 3.1415926;

		SnakeListClass SWCSnakes;
		PointList3D BranchPt;

		PointList3D CenterPt;
		Point3D RootPt;
		float SomaRadius;
		std::vector<float> Ru;
		std::vector<int> Type;
		std::vector<int> ID;
		std::vector<int> PID;
		Point3D temp;
		QTextStream in(&file);
		QStringList fields;

		std::vector<int> break_id;
		std::vector<int> branch_id;

		float SomaVolume = 0.0;
		int N_Stems = 0;
		float TotalLength = 0.0;
		int N_Branches = 0;
		int N_Bifs = 0;
		int N_Tips = 0;
		float A_Diameter = 0.0;
		float A_SectionArea = 0.0;
		float Volume = 0.0;
		float Surface = 0.0;
		float A_BranchOrder = 0.0;
		float A_BifAngle = 0.0;

		std::vector<float> BranchOrders;
		std::vector<float> BirAngles;

		std::string str1("#");
		int blank = 0;
		while( blank < 2 )
		{
			QString line = in.readLine();

			fields = line.split(' ',QString::SkipEmptyParts);

			if( fields.size() == 0 )
			{
				//end_of_txt = true;
				//break;
				blank++;
				continue;
			}

			if( fields.at(0).toStdString().compare(str1) == 0 )
			{
				continue;
			}


			ID.push_back(fields.at(0).toInt());
			Type.push_back(fields.at(1).toInt());
			temp.x = fields.at(2).toFloat();
			temp.y = fields.at(3).toFloat();
			temp.z = fields.at(4).toFloat();

			CenterPt.AddPt(temp);
			Ru.push_back(fields.at(5).toFloat());
			PID.push_back(fields.at(6).toInt());


			if( fields.at(1).toInt() == 1 )
				SomaVolume += 4 * Pi * pow(fields.at(5).toFloat(),3)/3;

			if( fields.at(6).toInt() == -1 )
			{
				RootPt = temp;
				SomaRadius = fields.at(5).toFloat();
				break_id.push_back(0);
				branch_id.push_back(0);
			}
			else
			{
				//when the type change from 1 to 3, count it as the stem
				if( fields.at(1).toInt() == 3 && Type[fields.at(6).toInt()-1] == 1 )
					N_Stems++;

				if( fields.at(6).toInt() != fields.at(0).toInt() - 1 && fields.at(6).toInt() != 1 )
				{
					break_id.push_back(1);
					break_id[fields.at(6).toInt()-1] = 1;
					branch_id.push_back(0);
					branch_id[fields.at(6).toInt()-1] = 1;

					BranchPt.AddPt(CenterPt.Pt[fields.at(6).toInt()-1]);


					Vector3D V1(temp.x - CenterPt.Pt[fields.at(6).toInt()-1].x,
						temp.y - CenterPt.Pt[fields.at(6).toInt()-1].y,
						temp.z - CenterPt.Pt[fields.at(6).toInt()-1].z);

					Vector3D V2(CenterPt.Pt[fields.at(6).toInt()].x - CenterPt.Pt[fields.at(6).toInt()-1].x,
						CenterPt.Pt[fields.at(6).toInt()].y - CenterPt.Pt[fields.at(6).toInt()-1].y,
						CenterPt.Pt[fields.at(6).toInt()].z - CenterPt.Pt[fields.at(6).toInt()-1].z);

					float angle = V1.GetAngleTo(V2) * 360/(2*Pi);

					BirAngles.push_back(angle);

					N_Bifs++;
					N_Tips++;
				}
				else
				{
					break_id.push_back(0);
					branch_id.push_back(0);
				}
			}
		}


		for( int i = 0; i < BirAngles.size(); i++ )
			A_BifAngle += BirAngles[i]/BirAngles.size();

		for( int i = 0; i < Ru.size(); i++ )
		{
			A_Diameter += Ru[i];
			A_SectionArea += Pi * pow(Ru[i],2);
		}
		A_Diameter /= Ru.size();
		A_SectionArea /= Ru.size();

		N_Tips++;

		PointList3D temp_Cu;
		std::vector<float> temp_Ru;
		//std::vector<float> temp_Type;
		SnakeClass temp_Snake;
		for( int i = 0; i < CenterPt.NP; i++ )
		{
			if( PID[i] == 1 && i != 1 )  //new stem
			{
				temp_Snake.Cu = temp_Cu;
				temp_Snake.Ru = temp_Ru;
				//temp_Snake.Type = temp_Type;
				SWCSnakes.AddSnake(temp_Snake);

				//find the branch order of the added branch
				int temp_order = 0;
				int current_id = i - 1;
				if( current_id > 0)
				{
					while( PID[current_id] != -1 )
					{
						current_id = PID[current_id]-1;
						if( branch_id[current_id] == 1 )
							temp_order++;
					}
				}
				BranchOrders.push_back(temp_order);

				temp_Cu.RemoveAllPts();
				temp_Ru.clear();
				//temp_Type.clear();

				temp_Cu.AddPt(CenterPt.Pt[PID[i]-1]);
				temp_Ru.push_back(Ru[PID[i]-1]);
				//temp_Type.push_back(Type[PID[i]-1]);
				temp_Cu.AddPt(CenterPt.Pt[i]);
				temp_Ru.push_back(Ru[i]);

				continue;
			}

			if( i == CenterPt.NP-1 )  //end of the swc file
			{
				temp_Snake.Cu = temp_Cu;
				temp_Snake.Ru = temp_Ru;
				//temp_Snake.Type = temp_Type;
				SWCSnakes.AddSnake(temp_Snake);
				//find the branch order of the added branch
				int temp_order = 0;
				int current_id = i - 1;
				if( current_id > 0)
				{
					while( PID[current_id] != -1 )
					{
						current_id = PID[current_id]-1;
						if( branch_id[current_id] == 1 )
							temp_order++;
					}
				}
				BranchOrders.push_back(temp_order);
				continue;
			}

			if( break_id[i] == 0 )
			{
				temp_Cu.AddPt(CenterPt.Pt[i]);
				temp_Ru.push_back(Ru[i]);
				//temp_Type.push_back(Type[i]);
			}
			else
			{
				if( PID[i] == ID[i] - 1 )
				{		  
					temp_Cu.AddPt(CenterPt.Pt[i]);
					temp_Ru.push_back(Ru[i]);
					//temp_Type.push_back(Type[i]);
					temp_Snake.Cu = temp_Cu;
					temp_Snake.Ru = temp_Ru;
					//temp_Snake.Type = temp_Type;
					SWCSnakes.AddSnake(temp_Snake);

					//find the branch order of the added branch
					int temp_order = 0;
					int current_id = i - 1;
					if( current_id > 0)
					{
						while( PID[current_id] != -1 )
						{
							current_id = PID[current_id]-1;
							if( branch_id[current_id] == 1 )
								temp_order++;
						}
					}
					BranchOrders.push_back(temp_order);

					temp_Cu.RemoveAllPts();
					temp_Ru.clear();
					//temp_Type.clear();

					temp_Cu.AddPt(CenterPt.Pt[i]);
					temp_Ru.push_back(Ru[i]);
					//temp_Type.push_back(Type[i]);
				}
				else
				{
					temp_Snake.Cu = temp_Cu;
					temp_Snake.Ru = temp_Ru;
					//temp_Snake.Type = temp_Type;
					SWCSnakes.AddSnake(temp_Snake);

					//find the branch order of the added branch
					int temp_order = 0;
					int current_id = i - 1;
					if( current_id > 0)
					{
						while( PID[current_id] != -1 )
						{
							current_id = PID[current_id]-1;
							if( branch_id[current_id] == 1 )
								temp_order++;
						}
					}
					BranchOrders.push_back(temp_order);

					temp_Cu.RemoveAllPts();
					temp_Ru.clear();
					//temp_Type.clear();

					temp_Cu.AddPt(CenterPt.Pt[PID[i]-1]);
					temp_Ru.push_back(Ru[PID[i]-1]);
					//temp_Type.push_back(Type[PID[i]-1]);
					temp_Cu.AddPt(CenterPt.Pt[i]);
					temp_Ru.push_back(Ru[i]);
					//temp_Type.push_back(Type[i]);
				}
			}

		}

		for( int i = 0; i < BranchOrders.size(); i++ )
		{
			A_BranchOrder += BranchOrders[i]/BranchOrders.size();
		}

		for( int i = 0; i < SWCSnakes.NSnakes; i++ )
		{
			TotalLength += SWCSnakes.Snakes[i].Cu.GetLength();
			for( int j = 0; j < SWCSnakes.Snakes[i].Cu.NP - 1 ; j++ )
			{
				Volume += Pi * pow(SWCSnakes.Snakes[i].Ru[j],2) * SWCSnakes.Snakes[i].Cu.Pt[j].GetDistTo(SWCSnakes.Snakes[i].Cu.Pt[j+1]);
				Surface += 2 * Pi * SWCSnakes.Snakes[i].Ru[j] * SWCSnakes.Snakes[i].Cu.Pt[j].GetDistTo(SWCSnakes.Snakes[i].Cu.Pt[j+1]);
			}
		}

		N_Branches = SWCSnakes.NSnakes;

		//compute the features
		TreeFeature features;
		features.SomaVolume = SomaVolume;
		features.N_Stems = N_Stems;
		features.TotalLength = TotalLength;
		features.N_Branches = N_Branches;
		features.N_Bifs = N_Bifs;
		features.N_Tips = N_Tips;
		features.A_Diameter = A_Diameter;
		features.A_SectionArea = A_SectionArea;
		features.Volume = Volume;
		features.Surface = Surface;
		features.A_BranchOrder = A_BranchOrder;
		features.A_BifAngle = A_BifAngle;

		tree_features.push_back(features);

		double color[3];
		if( tree_features.size() == 1 )
		{
			color[0] = 0;
			color[1] = 0;
			color[2] = 1;
		}
		else if( tree_features.size() == 2 )
		{
			color[0] = 1;
			color[1] = 0;
			color[2] = 0;
		}
		/*else if( tree_features.size() == 2 )
		{
		color[0] = 0;
		color[1] = 1;
		color[2] = 0;
		}
		else if( tree_features.size() == 3 )
		{
		color[0] = 1;
		color[1] = 0;
		color[2] = 0;
		}*/

		displaySWC(SWCSnakes, RootPt, SomaRadius, BranchPt, color);

		createTreeTables();
	} 
}


void QtTracer::createTreeTables()
{
	if( tree_features.size() == 0 )
		return;

	//create a table showing summarizing features
	//create model 

	QStandardItemModel *model_sum = new QStandardItemModel(tree_features.size(), 12, this);
	model_sum->setHeaderData(0, Qt::Horizontal, tr("Soma Volume"));
	model_sum->setHeaderData(1, Qt::Horizontal, tr("#Stems"));
	model_sum->setHeaderData(2, Qt::Horizontal, tr("Total Length"));
	model_sum->setHeaderData(3, Qt::Horizontal, tr("#Branches"));
	model_sum->setHeaderData(4, Qt::Horizontal, tr("#Bifurcations"));
	model_sum->setHeaderData(5, Qt::Horizontal, tr("#Tips"));
	model_sum->setHeaderData(6, Qt::Horizontal, tr("Average Diameter"));
	model_sum->setHeaderData(7, Qt::Horizontal, tr("Average Section Area"));
	model_sum->setHeaderData(8, Qt::Horizontal, tr("Volume"));
	model_sum->setHeaderData(9, Qt::Horizontal, tr("Surface"));
	model_sum->setHeaderData(10, Qt::Horizontal, tr("Average Branch Order"));
	model_sum->setHeaderData(11, Qt::Horizontal, tr("Average Bifurcation Angle"));

	//compute the tree features
	QStringList temp_header;

	for( int i = 0; i < tree_features.size(); i++ )
	{
		/*if( i == 0 )
		{
		QString header("Open Snakes");
		temp_header<<header;
		}
		else if( i == 1)
		{
		QString header("Ground Truth");
		temp_header<<header;
		}
		else if( i >= 2 )
		{
		QString header("Competing Algorithms");
		temp_header<<header;	
		}*/

		QString header;
		header.setNum(i+1);
		header.prepend("Tree_");
		temp_header<<header; 

		model_sum->setVerticalHeaderLabels(temp_header);
		model_sum->setData(model_sum->index(i, 0, QModelIndex()), tree_features[i].SomaVolume);
		model_sum->setData(model_sum->index(i, 1, QModelIndex()), tree_features[i].N_Stems);
		model_sum->setData(model_sum->index(i, 2, QModelIndex()), tree_features[i].TotalLength);
		model_sum->setData(model_sum->index(i, 3, QModelIndex()), tree_features[i].N_Branches);
		model_sum->setData(model_sum->index(i, 4, QModelIndex()), tree_features[i].N_Bifs);
		model_sum->setData(model_sum->index(i, 5, QModelIndex()), tree_features[i].N_Tips);
		model_sum->setData(model_sum->index(i, 6, QModelIndex()), tree_features[i].A_Diameter);
		model_sum->setData(model_sum->index(i, 7, QModelIndex()), tree_features[i].A_SectionArea);
		model_sum->setData(model_sum->index(i, 8, QModelIndex()), tree_features[i].Volume);
		model_sum->setData(model_sum->index(i, 9, QModelIndex()), tree_features[i].Surface);
		model_sum->setData(model_sum->index(i, 10, QModelIndex()), tree_features[i].A_BranchOrder);
		model_sum->setData(model_sum->index(i, 11, QModelIndex()), tree_features[i].A_BifAngle);
	}

	//create table and scatter views
	table_sum->setModel(model_sum);

	table_sum->setGeometry(QRect(400,400,800,100));
	table_sum->setWindowTitle("Tree Features");
	table_sum->show();

}


//VTK Rendering Functions
void QtTracer::displaySWC(SnakeListClass Snakes, Point3D RootPt, float soma_radius, PointList3D BranchPt, double *color)
{

	if( slider->getColorDisplay() )
	{
		double color1[3] = {(double)(rand()%10)/(double)10, (double)(rand()%10)/(double)10, (double)(rand()%10)/(double)10};
		color = color1;
	}

	//tree_colors.push_back(color);
	vnl_matrix<double> temp_colors(tree_features.size(),3);

	if(tree_features.size() != 1)
		temp_colors.update(tree_colors,0,0);

	temp_colors(tree_features.size()-1,0) = color[0];
	temp_colors(tree_features.size()-1,1) = color[1];
	temp_colors(tree_features.size()-1,2) = color[2];
	tree_colors = temp_colors; 

	if( tree_features.size() != 0 )
	{
		if( legend )
		{
			this->Renderer->RemoveActor(legend);
		}
		legend = vtkSmartPointer<vtkLegendBoxActor>::New();
		legend->SetNumberOfEntries(tree_features.size());
		double color1[3] = {0,0,1};
		double color2[3] = {0,1,0};
		double color3[3] = {1,0,0};

		for( int i = 0; i < tree_features.size(); i++ )
		{
			QString MSG("Tree_");
			QString MSG_NUM;
			MSG_NUM.setNum(i+1);
			MSG.append(MSG_NUM);
			vtkSmartPointer<vtkSphereSource> legendSphereSource = 
				vtkSmartPointer<vtkSphereSource>::New();
			vtkSmartPointer<vtkPolyData> legendSphere = legendSphereSource->GetOutput();
			double new_color[3] = {tree_colors.get_row(i)(0), tree_colors.get_row(i)(1), tree_colors.get_row(i)(2)};
			legend->SetEntry(i, legendSphere, MSG.toStdString().c_str(), new_color);
		}
		this->Renderer->AddActor(legend); 

		/*if( tree_features.size() == 1 )
		{
			vtkSmartPointer<vtkSphereSource> legendSphereSource = 
				vtkSmartPointer<vtkSphereSource>::New();
			vtkSmartPointer<vtkPolyData> legendSphere = legendSphereSource->GetOutput();
			legend->SetEntry(0, legendSphere, "Open-Snake", color1);
			this->Renderer->AddActor(legend);
		}
		else if( tree_features.size() == 2 )
		{
			vtkSmartPointer<vtkSphereSource> legendSphereSource = 
				vtkSmartPointer<vtkSphereSource>::New();
			vtkSmartPointer<vtkPolyData> legendSphere = legendSphereSource->GetOutput();
			legend->SetEntry(0, legendSphere, "Open-Snake", color1);

			vtkSmartPointer<vtkSphereSource> legendSphereSource1 = 
				vtkSmartPointer<vtkSphereSource>::New();
			vtkSmartPointer<vtkPolyData> legendSphere1 = legendSphereSource1->GetOutput();
			legend->SetEntry(1, legendSphere1, "Ground Truth", color3);
			this->Renderer->AddActor(legend);
		}
		else if( tree_features.size() == 3 )
		{
			vtkSmartPointer<vtkSphereSource> legendSphereSource = 
				vtkSmartPointer<vtkSphereSource>::New();
			vtkSmartPointer<vtkPolyData> legendSphere = legendSphereSource->GetOutput();
			legend->SetEntry(0, legendSphere, "Open-Snake", color1);

			vtkSmartPointer<vtkSphereSource> legendSphereSource1 = 
				vtkSmartPointer<vtkSphereSource>::New();
			vtkSmartPointer<vtkPolyData> legendSphere1 = legendSphereSource1->GetOutput();
			legend->SetEntry(1, legendSphere1, "Ground Truth", color3);

			vtkSmartPointer<vtkSphereSource> legendSphereSource2 = 
				vtkSmartPointer<vtkSphereSource>::New();
			vtkSmartPointer<vtkPolyData> legendSphere2 = legendSphereSource2->GetOutput();
			legend->SetEntry(2, legendSphere2, "NeuronStudio", color2);
			this->Renderer->AddActor(legend);
		}*/
	} 


	float offset_x = general_para3->getX();
	float offset_y = general_para3->getY();
	float offset_z = general_para3->getZ();

	//VTK rendering
	//draw branch points
	for( int i = 0; i <BranchPt.NP; i++ )
	{
		vtkSmartPointer<vtkSphereSource> sphereSource = 
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter((BranchPt.Pt[i].x + offset_x) / general_para->getSf() ,
			(BranchPt.Pt[i].y + offset_y) / general_para->getSf(),
			BranchPt.Pt[i].z + offset_z);
		sphereSource->SetRadius(2);

		//Create a mapper and actor
		vtkSmartPointer<vtkPolyDataMapper> branch_mapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
		branch_mapper->SetInputConnection(sphereSource->GetOutputPort());

		vtkSmartPointer<vtkActor> branch_actor = 
			vtkSmartPointer<vtkActor>::New();
		branch_actor->SetMapper(branch_mapper);
		branch_actor->GetProperty()->SetOpacity(0.6);
		branch_actor->GetProperty()->SetColor(color[0],color[1],color[2]);
		tree_branch_actors.push_back(branch_actor); 
		this->Renderer->AddActor(branch_actor);
	}

	//draw root points
	//Create a sphere
	vtkSmartPointer<vtkSphereSource> sphereSource = 
		vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->SetCenter((RootPt.x + offset_x) / general_para->getSf(),
		(RootPt.y + offset_y) / general_para->getSf(),
		RootPt.z + offset_z);
	if(soma_radius <= 3)
		soma_radius = 3;
	sphereSource->SetRadius(soma_radius);

	//Create a mapper and actor
	vtkSmartPointer<vtkPolyDataMapper> root_mapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	root_mapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkSmartPointer<vtkActor> root_actor = 
		vtkSmartPointer<vtkActor>::New();
	root_actor->SetMapper(root_mapper);
	//root_actor->GetProperty()->SetColor(color[0],color[1],color[2]);
	//root_actor->GetProperty()->SetColor(0.9,0.9,0.9);

	root_actor->GetProperty()->SetDiffuseColor(color[0],color[1],color[2]);
	root_actor->GetProperty()->SetSpecularColor(1, 1, 1);
	root_actor->GetProperty()->SetSpecular(0.4);
	root_actor->GetProperty()->SetSpecularPower(50);

	root_actor->GetProperty()->SetOpacity(0.8);

	tree_branch_actors.push_back(root_actor);
	this->Renderer->AddActor(root_actor);


	for( int i = 0; i < Snakes.NSnakes; i++ )
	{
		vtkSmartPointer<vtkPoints> line_points;
		vtkSmartPointer<vtkCellArray> line_cells;
		vtkSmartPointer<vtkPolyData> PolyTraces;
		line_points = vtkSmartPointer<vtkPoints>::New();
		line_cells = vtkSmartPointer<vtkCellArray>::New();
		PolyTraces = vtkSmartPointer<vtkPolyData>::New();


		//draw lines
		for( int j = 0; j <Snakes.Snakes[i].Cu.NP; j++ )
		{
			line_points->InsertPoint(j, (Snakes.Snakes[i].Cu.Pt[j].x + offset_x) / general_para->getSf(), 
				(Snakes.Snakes[i].Cu.Pt[j].y + offset_y) / general_para->getSf(),
				Snakes.Snakes[i].Cu.Pt[j].z + offset_z);
		}

		line_cells->InsertNextCell(Snakes.Snakes[i].Cu.NP);
		for (int j = 0; j < Snakes.Snakes[i].Cu.NP; j++)
		{
			line_cells->InsertCellPoint(j);
		}

		PolyTraces->SetPoints(line_points);
		PolyTraces->SetLines(line_cells);

		//draw tubes with varying radius
		if( slider->getTubeDisplay() )
		{
			vtkSmartPointer<vtkDoubleArray> tubeRadius =
				vtkSmartPointer<vtkDoubleArray>::New();
			tubeRadius->SetName("TubeRadius");
			tubeRadius->SetNumberOfTuples(Snakes.Snakes[i].Cu.NP);

			vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
			colors->SetName("Colors");
			colors->SetNumberOfComponents(3);
			colors->SetNumberOfTuples(Snakes.Snakes[i].Cu.NP);

			for (int j=0 ;j<Snakes.Snakes[i].Cu.NP ; j++)
			{
				//tubeRadius->SetTuple1(j,Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
				tubeRadius->SetTuple1(j,Snakes.Snakes[i].Ru[j] / general_para->getSf());
				//if( Snakes.Snakes[i].Type[j] == 1)
				// colors->InsertTuple3(j,int(0 * 255),int(0 * 255),int(1 * 255));
				//else
				//colors->InsertTuple3(j,int(1 * 255),int(1 * 255),int(0 * 255));
			}

			PolyTraces->GetPointData()->AddArray(tubeRadius);
			//PolyTraces->GetPointData()->AddArray(colors);
			PolyTraces->GetPointData()->SetActiveScalars("TubeRadius");

			vtkSmartPointer<vtkTubeFilter> tubeFilter = 
				vtkSmartPointer<vtkTubeFilter>::New();
			tubeFilter->SetInput(PolyTraces);
			tubeFilter->SetNumberOfSides(10);
			tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
			tubeFilter->Update();

			vtkSmartPointer<vtkPolyDataMapper> tubeMapper = 
				vtkSmartPointer<vtkPolyDataMapper>::New();
			tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
			tubeMapper->ScalarVisibilityOn();
			tubeMapper->SetScalarModeToUsePointFieldData();
			tubeMapper->SelectColorArray("Colors");

			vtkSmartPointer<vtkActor> tubeActor = 
				vtkSmartPointer<vtkActor>::New();
			tubeActor->SetMapper(tubeMapper);

			tubeActor->GetProperty()->SetDiffuseColor(color[0],color[1],color[2]);
			tubeActor->GetProperty()->SetSpecularColor(1, 1, 1);
			tubeActor->GetProperty()->SetSpecular(0.5);
			tubeActor->GetProperty()->SetSpecularPower(100);

			//tubeActor->GetProperty()->SetColor(0.6,0.9,0.1);
			tree_tube_actors.push_back(tubeActor);
			this->Renderer->AddActor(tubeActor);
		}

		vtkSmartPointer<vtkPolyDataMapper> polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
		polymap->SetInput(PolyTraces);

		vtkSmartPointer<vtkActor> line_actor = vtkSmartPointer<vtkActor>::New();
		line_actor->SetMapper(polymap);

		line_actor->GetProperty()->SetColor(color[0],color[1],color[2]);

		line_actor->GetProperty()->SetPointSize(2);
		line_actor->GetProperty()->SetLineWidth(LineWidth);
		tree_line_actors.push_back(line_actor);
		this->Renderer->AddActor(line_actor);
	}
}

void QtTracer::initializeRendering()
{
	this->ImageActors = new ImageRenderActors();

	this->Renderer = vtkSmartPointer<vtkRenderer>::New();
	this->Renderer->SetBackground(1,1,1);
	this->Camera = vtkSmartPointer<vtkCamera>::New();
	this->Renderer->SetActiveCamera(Camera);
	this->QVTK->GetRenderWindow()->AddRenderer(this->Renderer);

	//draw x y z labels
	vtkSmartPointer<vtkAxesActor> axes = 
		vtkSmartPointer<vtkAxesActor>::New();

	vtkTextProperty* axisXLabelTextProperty = vtkTextProperty::New(); 
	axisXLabelTextProperty->SetFontFamilyToArial(); 
	axisXLabelTextProperty->SetColor( 0.9300, 0.5700, 0.1300 ); 
	axisXLabelTextProperty->SetFontSize( 4 ); 
	//axisXLabelTextProperty->ShadowOn(); 
	axes->GetXAxisCaptionActor2D()->SetCaptionTextProperty( axisXLabelTextProperty );
	axes->GetYAxisCaptionActor2D()->SetCaptionTextProperty( axisXLabelTextProperty );
	axes->GetZAxisCaptionActor2D()->SetCaptionTextProperty( axisXLabelTextProperty );
	axisXLabelTextProperty->Delete(); 

	this->AxesWidget = vtkSmartPointer<vtkOrientationMarkerWidget>::New();
	this->AxesWidget->SetOutlineColor( 0.9300, 0.5700, 0.1300 );
	this->AxesWidget->SetOrientationMarker( axes );
	this->AxesWidget->SetInteractor( this->QVTK->GetRenderWindow()->GetInteractor() );
	this->AxesWidget->SetViewport( 0.0, 0.0, 0.2, 0.2 );
	this->AxesWidget->SetEnabled( 1 );
}

void QtTracer::vtk_removePoint()
{
	if( !soma_seeding )
	{
		if( pick_sphere_actors.size() != 0 )
		{
			for( int i = 0; i < pick_sphere_actors.size(); i++ )
				this->Renderer->RemoveActor( pick_sphere_actors[i] );
		}
	}
}

void QtTracer::vtk_left_pick(vtkObject * obj)
{
	//std::cout<< "vtk_left_pick"<<std::endl;
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	// get event location
	iren->GetPicker()->Pick(iren->GetEventPosition()[0], 
		iren->GetEventPosition()[1], 
		0,  // always zero.
		iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

	int* sz = iren->GetSize();
	double position[3];
	iren->GetPicker()->GetPickPosition(position);

	if( this->QVTK->GetRenderWindow()->GetInteractor()->GetControlKey() == 1 )
	{
		tracingViewer->vtk_mousePress(position, true);
	}
	else
	{
		tracingViewer->vtk_mousePress(position, false);
	}
}

void QtTracer::vtk_right_pick(vtkObject * obj)
{
	//std::cout<< "vtk_right_pick"<<std::endl;
	// get interactor
	vtkRenderWindowInteractor* iren = vtkRenderWindowInteractor::SafeDownCast(obj);
	// get event location
	iren->GetPicker()->Pick(iren->GetEventPosition()[0], 
		iren->GetEventPosition()[1], 
		0,  // always zero.
		iren->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

	int* sz = iren->GetSize();
	double position[3];
	iren->GetPicker()->GetPickPosition(position);

	std::cout<<"picked manual point for tracing:"<<position[0]<<","<<position[1]<<","<<position[2]<<std::endl;
	PointList3D seeds;
	seeds.AddPt(position[0] * (float)1/(float)general_para->getSh(),position[1] * (float)1/(float)general_para->getSh(),position[2] * (float)1/(float)general_para->getSh());

	if( soma_seeding )
	{
		picked_pts.AddPt(position[0],position[1],position[2]);
		//draw blue sphere
		vtkSmartPointer<vtkSphereSource> sphereSource = 
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter(position[0],position[1],position[2]);
		sphereSource->SetRadius(2);

		vtkSmartPointer<vtkPolyDataMapper> pick_sphere_mapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
		pick_sphere_mapper->SetInputConnection(sphereSource->GetOutputPort());

		vtkSmartPointer<vtkActor> pick_sphere_actor =  vtkSmartPointer<vtkActor>::New();
		pick_sphere_actor->SetMapper(pick_sphere_mapper);
		pick_sphere_actor->GetProperty()->SetColor(0,0,1);
		pick_sphere_actor->GetProperty()->SetOpacity(0.9);
		pick_sphere_actors.push_back(pick_sphere_actor);

		this->Renderer->AddActor(pick_sphere_actor);
	}
	else
	{
		picked_pts.RemoveAllPts();
		picked_pts.AddPt(position[0] * (float)1/(float)general_para->getSh(),position[1] * (float)1/(float)general_para->getSh(),position[2] * (float)1/(float)general_para->getSh());

		//remove all previous spheres
		for( int i = 0; i < pick_sphere_actors.size(); i++ )
			this->Renderer->RemoveActor( pick_sphere_actors[i] );
		//draw blue sphere
		vtkSmartPointer<vtkSphereSource> sphereSource = 
			vtkSmartPointer<vtkSphereSource>::New();
		sphereSource->SetCenter(position[0],position[1],position[2]);
		sphereSource->SetRadius(2);

		vtkSmartPointer<vtkPolyDataMapper> pick_sphere_mapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
		pick_sphere_mapper->SetInputConnection(sphereSource->GetOutputPort());

		vtkSmartPointer<vtkActor> pick_sphere_actor =  vtkSmartPointer<vtkActor>::New();
		pick_sphere_actor->SetMapper(pick_sphere_mapper);
		pick_sphere_actor->GetProperty()->SetColor(0,0,1);
		pick_sphere_actor->GetProperty()->SetOpacity(0.9);
		pick_sphere_actors.push_back(pick_sphere_actor);

		seeding = true;
		m_seed.RemoveAllPts();
		Manual_Seed_Tracing(seeds);
	}

	std::cout<<"Number of Clicked Point:"<<picked_pts.NP<<std::endl;
}

void QtTracer::drawClickedTraces()
{
	if( slider->getDisplaySnakes() )
	{
		if( selected_line_actors.size() != 0 )
		{
			for( int i = 0; i < selected_line_actors.size(); i++ )
			{
				this->Renderer->RemoveActor(selected_line_actors[i]);
			}
		}

		vnl_vector<int> selected_snake_id =tracingViewer->getSelectedSnakes();
		for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
		{

			if( Tracer->SnakeList.valid_list[i] == 0)
				continue;

			if(selected_snake_id(i) != 1)
				continue;

			vtkSmartPointer<vtkPoints> line_points;
			vtkSmartPointer<vtkCellArray> line_cells;
			vtkSmartPointer<vtkPolyData> PolyTraces;
			line_points = vtkSmartPointer<vtkPoints>::New();
			line_cells = vtkSmartPointer<vtkCellArray>::New();
			PolyTraces = vtkSmartPointer<vtkPolyData>::New();

			double color[3];
			color[0] = 0;
			color[1] = 1;
			color[2] = 0;

			for( int j = 0; j <Tracer->SnakeList.Snakes[i].Cu.NP; j++ )
			{
				line_points->InsertPoint(j, Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh(), 
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
			}

			line_cells->InsertNextCell(Tracer->SnakeList.Snakes[i].Cu.NP);
			for (int j = 0; j < Tracer->SnakeList.Snakes[i].Cu.NP; j++)
			{
				line_cells->InsertCellPoint(j);
			}

			PolyTraces->SetPoints(line_points);
			PolyTraces->SetLines(line_cells);

			//draw tubes with varying radius

			vtkSmartPointer<vtkPolyDataMapper> polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
			polymap->SetInput(PolyTraces);

			vtkSmartPointer<vtkActor> line_actor = vtkSmartPointer<vtkActor>::New();
			line_actor->SetMapper(polymap);

			line_actor->GetProperty()->SetColor(1,0,0);
			line_actor->GetProperty()->SetPointSize(1);
			line_actor->GetProperty()->SetLineWidth(LineWidth * 1.5);

			this->Renderer->AddActor(line_actor);

			selected_line_actors.push_back(line_actor);
		}
	}
}

void QtTracer::drawNew3DTrace()
{
	//vnl_vector<int> selected_snake_id =tracingViewer->getSelectedSnakes();
	int i = Tracer->SnakeList.NSnakes-1;

	//bool selected = false;
	//if(selected_snake_id(i) == 1)
	//   selected = true;

	vtkSmartPointer<vtkPoints> line_points;
	vtkSmartPointer<vtkCellArray> line_cells;
	vtkSmartPointer<vtkPolyData> PolyTraces;
	line_points = vtkSmartPointer<vtkPoints>::New();
	line_cells = vtkSmartPointer<vtkCellArray>::New();
	PolyTraces = vtkSmartPointer<vtkPolyData>::New();

	double color[3];
	color[0] = 0;
	color[1] = 1;
	color[2] = 0;

	/*for( int j = 0; j <Tracer->SnakeList.Snakes[i].Cu.NP-1; j++ )
	{
	double point[3];
	double point1[3];

	point[0] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh(); 
	point[1] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(); 
	point[2] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z;

	point1[0] = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].x * general_para->getSh(); 
	point1[1] = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].y * general_para->getSh(); 
	point1[2] = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].z;

	unsigned int return_id = line_points->InsertNextPoint(point);
	unsigned int return_id1 = line_points->InsertNextPoint(point1);

	if( j == 0 )
	{
	unsigned int r_id = line_tips->InsertNextPoint(point);
	vtkIdType con[1]; 
	con[0] = r_id;
	point_cells->InsertNextCell(1,con);
	}
	if( j == seed_snakes.Snakes[i].Cu.NP - 2 )
	{
	unsigned int r_id = line_tips->InsertNextPoint(point1);
	vtkIdType con[1]; 
	con[0] = r_id;
	point_cells->InsertNextCell(1,con);
	}

	vtkIdType connectivity[2]; 
	connectivity[0] = return_id; 
	connectivity[1] = return_id1; 
	line_cells->InsertNextCell(2,connectivity); 
	}*/

	for( int j = 0; j <Tracer->SnakeList.Snakes[i].Cu.NP; j++ )
	{
		line_points->InsertPoint(j, Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh(), 
			Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
			Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
	}

	line_cells->InsertNextCell(Tracer->SnakeList.Snakes[i].Cu.NP);
	for (int j = 0; j < Tracer->SnakeList.Snakes[i].Cu.NP; j++)
	{
		line_cells->InsertCellPoint(j);
	}

	PolyTraces->SetPoints(line_points);
	PolyTraces->SetLines(line_cells);

	//draw tubes with varying radius
	if( slider->getTubeDisplay() )
	{
				// Varying tube radius 
				vtkSmartPointer<vtkDoubleArray> tubeRadius =
					vtkSmartPointer<vtkDoubleArray>::New();
				tubeRadius->SetName("TubeRadius");
				tubeRadius->SetNumberOfTuples(Tracer->SnakeList.Snakes[i].Cu.NP);

				vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
				colors->SetName("Colors");
				colors->SetNumberOfComponents(3);
				colors->SetNumberOfTuples(Tracer->SnakeList.Snakes[i].Cu.NP);

				for (int j=0 ;j<Tracer->SnakeList.Snakes[i].Cu.NP ; j++)
				{
					//tubeRadius->SetTuple1(j,Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					tubeRadius->SetTuple1(j,Tracer->SnakeList.Snakes[i].Ru[j] * general_para->getSh());
					//colors->InsertTuple3(j,int(1 * 255),int(1 * 255),int(0 * 255));
					//colors->InsertTuple3(j,int((Tracer->SnakeList.Snakes[i].Ru[j] * 30)),0,int((255 - Tracer->SnakeList.Snakes[i].Ru[j] * 30)));
				}

				PolyTraces->GetPointData()->AddArray(tubeRadius);
				//PolyTraces->GetPointData()->AddArray(colors);
				PolyTraces->GetPointData()->SetActiveScalars("TubeRadius");

				vtkSmartPointer<vtkTubeFilter> tubeFilter = 
					vtkSmartPointer<vtkTubeFilter>::New();
				tubeFilter->SetInput(PolyTraces);
				tubeFilter->SetNumberOfSides(10);
				tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
				tubeFilter->Update();

				vtkSmartPointer<vtkPolyDataMapper> tubeMapper = 
					vtkSmartPointer<vtkPolyDataMapper>::New();
				tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
				tubeMapper->ScalarVisibilityOn();
				tubeMapper->SetScalarModeToUsePointFieldData();
				tubeMapper->SelectColorArray("Colors");

				vtkSmartPointer<vtkActor> tubeActor = 
					vtkSmartPointer<vtkActor>::New();
				tubeActor->SetMapper(tubeMapper);

				tubeActor->GetProperty()->SetDiffuseColor(double(72)/255,double(118)/255,1);
				tubeActor->GetProperty()->SetSpecularColor(1, 1, 1);
				tubeActor->GetProperty()->SetSpecular(0.4);
				tubeActor->GetProperty()->SetSpecularPower(50);

				//tubeActor->GetProperty()->SetColor(0.6,0.9,0.1);
				this->Renderer->AddActor(tubeActor);

				tube_actors.push_back(tubeActor);
	}

	vtkSmartPointer<vtkPolyDataMapper> polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
	polymap->SetInput(PolyTraces);

	vtkSmartPointer<vtkActor> line_actor = vtkSmartPointer<vtkActor>::New();
	line_actor->SetMapper(polymap);


	if( slider->getColorDisplay() )
	{
		/*if( i%6 == 0 )
		line_actor->GetProperty()->SetColor(1,0,0);
		else if( i%6 == 1 )
		line_actor->GetProperty()->SetColor(0,1,0);
		else if( i%6 == 2 )
		line_actor->GetProperty()->SetColor(0,0,1);
		else if( i%6 == 3 )
		line_actor->GetProperty()->SetColor(1,1,0);
		else if( i%6 == 4 )
		line_actor->GetProperty()->SetColor(1,0,1);
		else if( i%6 == 5 )
		line_actor->GetProperty()->SetColor(0,1,1);*/
		//assign the color for traces grown from soma
		if( IM->num_soma != 0 )
		{
			LabelImageType::IndexType new_index;
			new_index[0] = Tracer->SnakeList.Snakes[i].Cu.GetMiddlePt().x;
			new_index[1] = Tracer->SnakeList.Snakes[i].Cu.GetMiddlePt().y; 
			new_index[2] = Tracer->SnakeList.Snakes[i].Cu.GetMiddlePt().z; 

			int label = IM->IVoronoi->GetPixel(new_index);

			if( label != 0 )
			{
				color[0] = soma_color(label-1,0);
				color[1] = soma_color(label-1,1);
				color[2] = soma_color(label-1,2);
			}
			line_actor->GetProperty()->SetColor(color[0],color[1],color[2]);
		}
		else
		{
			line_actor->GetProperty()->SetColor((double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10);
		}
	}
	else
		line_actor->GetProperty()->SetColor(color[0],color[1],color[2]);


	//if( selected )
	//{
	//	line_actor->GetProperty()->SetColor(1,0,0);
	//   line_actor->GetProperty()->SetPointSize(1);
	//   line_actor->GetProperty()->SetLineWidth(5);
	//}
	//else
	//{
	line_actor->GetProperty()->SetPointSize(1);
	line_actor->GetProperty()->SetLineWidth(LineWidth);
	//}
	this->Renderer->AddActor(line_actor);

	line_actors.push_back(line_actor);


}

void QtTracer::clearTraces()
{
	//delete actors
	if( tube_actors.size() != 0 )
	{
		for( int i = 0; i < tube_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(tube_actors[i]);
		}
		tube_actors.clear();
	}
	if( line_actors.size() != 0 )
	{
		for( int i = 0; i < line_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(line_actors[i]);
		}
		line_actors.clear();

		this->Renderer->RemoveActor(this->point_actor);
	}
	if( branch_actors.size() != 0 )
	{
		for( int i = 0; i < branch_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(branch_actors[i]);
		}
		branch_actors.clear();
	}
	if( boundary_actors.size() != 0 )
	{
		for( int i = 0; i < boundary_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(boundary_actors[i]);
		}
		boundary_actors.clear();
	}
}

void QtTracer::draw3DTraces()
{
	//std::cout<<"3D rendering.........."<<std::endl;
	if( bpII_finished )
	{
		draw3DTraces_Global();
		return;
	}

	if( tracing_line_actor )
		this->Renderer->RemoveActor(tracing_line_actor);

	vtkSmartPointer<vtkPoints> line_tips;
	vtkSmartPointer<vtkCellArray> point_cells;
	vtkSmartPointer<vtkPolyData> PolyPoints;
	line_tips = vtkSmartPointer<vtkPoints>::New();
	point_cells = vtkSmartPointer<vtkCellArray>::New();
	PolyPoints = vtkSmartPointer<vtkPolyData>::New();

	//draw seeds
	if( slider->getDisplaySeed() )
	{
		vtkSmartPointer<vtkPoints> seed_points;
		vtkSmartPointer<vtkCellArray> seed_point_cells;
		vtkSmartPointer<vtkPolyData> seed_PolyPoints;
		seed_points = vtkSmartPointer<vtkPoints>::New();
		seed_point_cells = vtkSmartPointer<vtkCellArray>::New();
		seed_PolyPoints = vtkSmartPointer<vtkPolyData>::New();
		for( int i = 0; i < IM->SeedPt.NP; i++ )
		{
			if ( IM->visit_label(i) == 1 )
				continue;

			double point[3];
			point[0] = IM->SeedPt.Pt[i].x * general_para->getSh(); 
			point[1] = IM->SeedPt.Pt[i].y * general_para->getSh(); 
			point[2] = IM->SeedPt.Pt[i].z;
			unsigned int r_id = seed_points->InsertNextPoint(point);
			vtkIdType con[1]; 
			con[0] = r_id;
			seed_point_cells->InsertNextCell(1,con);
		}
		seed_PolyPoints->SetPoints(seed_points);
		seed_PolyPoints->SetVerts(seed_point_cells);

		if(this->seed_actor)
		{
			this->Renderer->RemoveActor(this->seed_actor);
		}

		vtkSmartPointer<vtkPolyDataMapper> seed_polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
		seed_polymap->SetInput(seed_PolyPoints);
		this->seed_actor = vtkSmartPointer<vtkActor>::New();
		this->seed_actor->SetMapper(seed_polymap);
		this->seed_actor->GetProperty()->SetColor(0,0,1);


		this->seed_actor->GetProperty()->SetPointSize(3);
		this->seed_actor->GetProperty()->SetEdgeColor(1,1,1);

		this->Renderer->AddActor(this->seed_actor);
	}
	else if(this->seed_actor)
	{
		this->Renderer->RemoveActor(this->seed_actor);
	}


	//delete previous actors
	if( tube_actors.size() != 0 )
	{
		for( int i = 0; i < tube_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(tube_actors[i]);
		}
		tube_actors.clear();
	}
	if( line_actors.size() != 0 )
	{
		for( int i = 0; i < line_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(line_actors[i]);
		}
		line_actors.clear();

		this->Renderer->RemoveActor(this->point_actor);
	}
	if( branch_actors.size() != 0 )
	{
		for( int i = 0; i < branch_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(branch_actors[i]);
		}
		branch_actors.clear();
	}
	if( boundary_actors.size() != 0 )
	{
		for( int i = 0; i < boundary_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(boundary_actors[i]);
		}
		boundary_actors.clear();
	}

	Vector3D temp;
	float unit_length = general_para2->getPtDistance();
	/* float color1 = (double)(rand()%10)/(double)10;
	float color2 = (double)(rand()%10)/(double)10;
	float color3 = (double)(rand()%10)/(double)10;
	std::cout<<"color:"<<color1<<","<<color2<<","<<color3<<std::endl; */
	//draw traces
	if( slider->getDisplaySnakes() )
	{
		//draw the tube
		/*if( slider->getTubeDisplay() )
		{
		for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
		{   
		if( Tracer->SnakeList.valid_list[i] == 0)
		continue;
		for( int j = 0; j <Tracer->SnakeList.Snakes[i].Cu.NP-1; j++ )
		{   
		vtkSmartPointer<vtkLineSource> lineSource = 
		vtkSmartPointer<vtkLineSource>::New();

		if( j == 0 )
		{
		float x1, y1, z1;
		x1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x;
		y1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y;
		z1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z;
		lineSource->SetPoint1(x1,y1,z1);
		float x2, y2, z2;
		temp.x = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].x - Tracer->SnakeList.Snakes[i].Cu.Pt[j].x;
		temp.y = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].y - Tracer->SnakeList.Snakes[i].Cu.Pt[j].y;
		temp.z = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].z - Tracer->SnakeList.Snakes[i].Cu.Pt[j].z;
		temp.ConvertUnit();
		x2 = x1 + temp.x * unit_length * 2/3;
		y2 = y1 + temp.y * unit_length * 2/3;
		z2 = z1 + temp.z * unit_length * 2/3;
		lineSource->SetPoint2(x2,y2,z2);
		}
		else if( j == Tracer->SnakeList.Snakes[i].Cu.NP-1 )
		{
		float x1, y1, z1;
		x1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x;
		y1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y;
		z1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z;
		lineSource->SetPoint1(x1,y1,z1);
		float x2, y2, z2;
		temp.x = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].x;
		temp.y = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].y;
		temp.z = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].z;
		temp.ConvertUnit();
		x2 = x1 - temp.x * unit_length * 2/3;
		y2 = y1 - temp.y * unit_length * 2/3;
		z2 = z1 - temp.z * unit_length * 2/3;
		lineSource->SetPoint2(x2,y2,z2);
		}
		else
		{
		temp.x = (Tracer->SnakeList.Snakes[i].Cu.Pt[j].x - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].x)/2 + 
		(Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].x - Tracer->SnakeList.Snakes[i].Cu.Pt[j].x)/2;
		temp.y = (Tracer->SnakeList.Snakes[i].Cu.Pt[j].y - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].y)/2 + 
		(Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].y - Tracer->SnakeList.Snakes[i].Cu.Pt[j].y)/2;
		temp.z = (Tracer->SnakeList.Snakes[i].Cu.Pt[j].z - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].z)/2 + 
		(Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].z - Tracer->SnakeList.Snakes[i].Cu.Pt[j].z)/2;
		temp.ConvertUnit();
		float x1, y1, z1;
		x1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x - temp.x * unit_length * 2/3;
		y1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y - temp.y * unit_length * 2/3;
		z1 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z - temp.z * unit_length * 2/3;
		lineSource->SetPoint1(x1,y1,z1);
		float x2, y2, z2;
		x2 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x + temp.x * unit_length * 2/3;
		y2 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y + temp.y * unit_length * 2/3;
		z2 = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z + temp.z * unit_length * 2/3;
		lineSource->SetPoint2(x2,y2,z2);
		}

		vtkSmartPointer<vtkTubeFilter> tubeFilter = 
		vtkSmartPointer<vtkTubeFilter>::New();
		tubeFilter->SetInputConnection(lineSource->GetOutputPort());
		tubeFilter->SetRadius(Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
		tubeFilter->SetNumberOfSides(10);
		tubeFilter->Update();

		vtkSmartPointer<vtkPolyDataMapper> tubeMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
		tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
		vtkSmartPointer<vtkActor> tubeActor = 
		vtkSmartPointer<vtkActor>::New();
		tubeActor->SetMapper(tubeMapper);

		tubeActor->GetProperty()->SetColor(0.6,0.9,0.1);
		this->Renderer->AddActor(tubeActor);
		tube_actors.push_back(tubeActor);
		}
		}
		} */


		//vnl_vector<int> selected_snake_id =tracingViewer->getSelectedSnakes();
		for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
		{

			if( Tracer->SnakeList.valid_list[i] == 0)
				continue;

			//bool selected = false;
			//if(selected_snake_id(i) == 1)
			//   selected = true;

			vtkSmartPointer<vtkPoints> line_points;
			vtkSmartPointer<vtkCellArray> line_cells;
			vtkSmartPointer<vtkPolyData> PolyTraces;
			line_points = vtkSmartPointer<vtkPoints>::New();
			line_cells = vtkSmartPointer<vtkCellArray>::New();
			PolyTraces = vtkSmartPointer<vtkPolyData>::New();

			double color[3];
			color[0] = 0;
			color[1] = 1;
			color[2] = 0;


			/*for( int j = 0; j <Tracer->SnakeList.Snakes[i].Cu.NP-1; j++ )
			{
			double point[3];
			double point1[3];

			point[0] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh(); 
			point[1] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(); 
			point[2] = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z;

			point1[0] = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].x * general_para->getSh(); 
			point1[1] = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].y * general_para->getSh(); 
			point1[2] = Tracer->SnakeList.Snakes[i].Cu.Pt[j+1].z;

			unsigned int return_id = line_points->InsertNextPoint(point);
			unsigned int return_id1 = line_points->InsertNextPoint(point1);

			if( j == 0 )
			{
			unsigned int r_id = line_tips->InsertNextPoint(point);
			vtkIdType con[1]; 
			con[0] = r_id;
			point_cells->InsertNextCell(1,con);
			}
			if( j == seed_snakes.Snakes[i].Cu.NP - 2 )
			{
			unsigned int r_id = line_tips->InsertNextPoint(point1);
			vtkIdType con[1]; 
			con[0] = r_id;
			point_cells->InsertNextCell(1,con);
			}

			vtkIdType connectivity[2]; 
			connectivity[0] = return_id; 
			connectivity[1] = return_id1; 
			line_cells->InsertNextCell(2,connectivity); 
			}*/

			for( int j = 0; j <Tracer->SnakeList.Snakes[i].Cu.NP; j++ )
			{
				line_points->InsertPoint(j, Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh(), 
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
			}

			line_cells->InsertNextCell(Tracer->SnakeList.Snakes[i].Cu.NP);
			for (int j = 0; j < Tracer->SnakeList.Snakes[i].Cu.NP; j++)
			{
				line_cells->InsertCellPoint(j);
			}

			PolyTraces->SetPoints(line_points);
			PolyTraces->SetLines(line_cells);

			//draw tubes with varying radius
			if( slider->getTubeDisplay() )
			{
				// Varying tube radius 
				vtkSmartPointer<vtkDoubleArray> tubeRadius =
					vtkSmartPointer<vtkDoubleArray>::New();
				tubeRadius->SetName("TubeRadius");
				tubeRadius->SetNumberOfTuples(Tracer->SnakeList.Snakes[i].Cu.NP);

				vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
				colors->SetName("Colors");
				colors->SetNumberOfComponents(3);
				colors->SetNumberOfTuples(Tracer->SnakeList.Snakes[i].Cu.NP);

				for (int j=0 ;j<Tracer->SnakeList.Snakes[i].Cu.NP ; j++)
				{
					//tubeRadius->SetTuple1(j,Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					tubeRadius->SetTuple1(j,Tracer->SnakeList.Snakes[i].Ru[j] * general_para->getSh());
					//colors->InsertTuple3(j,int(1 * 255),int(1 * 255),int(0 * 255));
					//colors->InsertTuple3(j,int((Tracer->SnakeList.Snakes[i].Ru[j] * 30)),0,int((255 - Tracer->SnakeList.Snakes[i].Ru[j] * 30)));
				}

				PolyTraces->GetPointData()->AddArray(tubeRadius);
				//PolyTraces->GetPointData()->AddArray(colors);
				PolyTraces->GetPointData()->SetActiveScalars("TubeRadius");

				vtkSmartPointer<vtkTubeFilter> tubeFilter = 
					vtkSmartPointer<vtkTubeFilter>::New();
				tubeFilter->SetInput(PolyTraces);
				tubeFilter->SetNumberOfSides(10);
				tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
				tubeFilter->Update();

				vtkSmartPointer<vtkPolyDataMapper> tubeMapper = 
					vtkSmartPointer<vtkPolyDataMapper>::New();
				tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
				tubeMapper->ScalarVisibilityOn();
				tubeMapper->SetScalarModeToUsePointFieldData();
				tubeMapper->SelectColorArray("Colors");

				vtkSmartPointer<vtkActor> tubeActor = 
					vtkSmartPointer<vtkActor>::New();
				tubeActor->SetMapper(tubeMapper);

				tubeActor->GetProperty()->SetDiffuseColor(double(72)/255,double(118)/255,1);
				tubeActor->GetProperty()->SetSpecularColor(1, 1, 1);
				tubeActor->GetProperty()->SetSpecular(0.4);
				tubeActor->GetProperty()->SetSpecularPower(50);

				//tubeActor->GetProperty()->SetColor(0.6,0.9,0.1);
				this->Renderer->AddActor(tubeActor);

				tube_actors.push_back(tubeActor);
			}

			vtkSmartPointer<vtkPolyDataMapper> polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
			polymap->SetInput(PolyTraces);

			vtkSmartPointer<vtkActor> line_actor = vtkSmartPointer<vtkActor>::New();
			line_actor->SetMapper(polymap);


			if( slider->getColorDisplay() )
			{
				/*if( i%6 == 0 )
				line_actor->GetProperty()->SetColor(1,0,0);
				else if( i%6 == 1 )
				line_actor->GetProperty()->SetColor(0,1,0);
				else if( i%6 == 2 )
				line_actor->GetProperty()->SetColor(0,0,1);
				else if( i%6 == 3 )
				line_actor->GetProperty()->SetColor(1,1,0);
				else if( i%6 == 4 )
				line_actor->GetProperty()->SetColor(1,0,1);
				else if( i%6 == 5 )
				line_actor->GetProperty()->SetColor(0,1,1);*/

				//assign the color for traces grown from soma
				if( IM->num_soma != 0 )
				{
					LabelImageType::IndexType new_index;
					new_index[0] = Tracer->SnakeList.Snakes[i].Cu.GetMiddlePt().x;
					new_index[1] = Tracer->SnakeList.Snakes[i].Cu.GetMiddlePt().y; 
					new_index[2] = Tracer->SnakeList.Snakes[i].Cu.GetMiddlePt().z; 

					int label = IM->IVoronoi->GetPixel(new_index);

					if( label != 0 )
					{
						color[0] = soma_color(label-1,0);
						color[1] = soma_color(label-1,1);
						color[2] = soma_color(label-1,2);
					}
					line_actor->GetProperty()->SetColor(color[0],color[1],color[2]);
				}
				else
				{
					line_actor->GetProperty()->SetColor((double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10);
				}
			}
			else
				line_actor->GetProperty()->SetColor(color[0],color[1],color[2]);


			//if( selected )
			//{
			//	line_actor->GetProperty()->SetColor(1,0,0);
			//   line_actor->GetProperty()->SetPointSize(1);
			//   line_actor->GetProperty()->SetLineWidth(5);
			//}
			//else
			//{
			line_actor->GetProperty()->SetPointSize(1);
			line_actor->GetProperty()->SetLineWidth(LineWidth);
			//}
			this->Renderer->AddActor(line_actor);

			line_actors.push_back(line_actor);
		}


		//draw the tips
		/*PolyPoints->SetPoints(line_tips);
		PolyPoints->SetVerts(point_cells);

		vtkSmartPointer<vtkPolyDataMapper> polymap1 = vtkSmartPointer<vtkPolyDataMapper>::New();
		polymap1->SetInput(PolyPoints);

		this->point_actor = vtkSmartPointer<vtkActor>::New();
		this->point_actor->SetMapper(polymap1);
		this->point_actor->GetProperty()->SetColor(1,0,0);
		this->point_actor->GetProperty()->SetPointSize(5);
		this->Renderer->AddActor(this->point_actor);*/

		//draw branch points
		for( int i = 0; i < Tracer->SnakeList.branch_points.NP; i++ )
		{
			vtkSmartPointer<vtkSphereSource> sphereSource = 
				vtkSmartPointer<vtkSphereSource>::New();
			sphereSource->SetCenter(Tracer->SnakeList.branch_points.Pt[i].x * general_para->getSh() ,
				Tracer->SnakeList.branch_points.Pt[i].y * general_para->getSh(),
				Tracer->SnakeList.branch_points.Pt[i].z);
			sphereSource->SetRadius(2);

			//Create a mapper and actor
			vtkSmartPointer<vtkPolyDataMapper> branch_mapper = 
				vtkSmartPointer<vtkPolyDataMapper>::New();
			branch_mapper->SetInputConnection(sphereSource->GetOutputPort());

			vtkSmartPointer<vtkActor> branch_actor = 
				vtkSmartPointer<vtkActor>::New();
			branch_actor->SetMapper(branch_mapper);
			branch_actor->GetProperty()->SetOpacity(0.6);
			branch_actors.push_back(branch_actor);

			this->Renderer->AddActor(branch_actor);
		}

		//int N_Snakes = 0;
		for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
		{
			if( Tracer->SnakeList.valid_list[i] == 0)
				continue;
			//N_Snakes++;
			/*for( int j = 0; j < Tracer->SnakeList.Snakes[i].BranchPt.NP; j++ )
			{
			//Create a sphere
			vtkSmartPointer<vtkSphereSource> sphereSource = 
			vtkSmartPointer<vtkSphereSource>::New();
			sphereSource->SetCenter(Tracer->SnakeList.Snakes[i].BranchPt.Pt[j].x * general_para->getSh() ,
			Tracer->SnakeList.Snakes[i].BranchPt.Pt[j].y * general_para->getSh(),
			Tracer->SnakeList.Snakes[i].BranchPt.Pt[j].z);
			sphereSource->SetRadius(2);

			//Create a mapper and actor
			vtkSmartPointer<vtkPolyDataMapper> branch_mapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
			branch_mapper->SetInputConnection(sphereSource->GetOutputPort());

			vtkSmartPointer<vtkActor> branch_actor = 
			vtkSmartPointer<vtkActor>::New();
			branch_actor->SetMapper(branch_mapper);
			branch_actors.push_back(branch_actor);

			this->Renderer->AddActor(branch_actor);
			}*/

			//draw root points
			for( int j = 0; j < Tracer->SnakeList.Snakes[i].RootPt.NP; j++ )
			{
				//Create a sphere
				vtkSmartPointer<vtkSphereSource> sphereSource = 
					vtkSmartPointer<vtkSphereSource>::New();
				sphereSource->SetCenter(Tracer->SnakeList.Snakes[i].RootPt.Pt[j].x * general_para->getSh() ,
					Tracer->SnakeList.Snakes[i].RootPt.Pt[j].y * general_para->getSh(),
					Tracer->SnakeList.Snakes[i].RootPt.Pt[j].z);
				sphereSource->SetRadius(3);

				//Create a mapper and actor
				vtkSmartPointer<vtkPolyDataMapper> root_mapper = 
					vtkSmartPointer<vtkPolyDataMapper>::New();
				root_mapper->SetInputConnection(sphereSource->GetOutputPort());

				vtkSmartPointer<vtkActor> root_actor = 
					vtkSmartPointer<vtkActor>::New();
				root_actor->SetMapper(root_mapper);

				root_actor->GetProperty()->SetColor(1,1,0);

				branch_actors.push_back(root_actor);


				this->Renderer->AddActor(root_actor);
			}
		}

		//draw text in the upper left corner
		/*QString im_size("Size:");
		QString temp;
		temp.setNum(IM->SM);
		im_size.append(temp);
		im_size.append(",");
		temp.setNum(IM->SN);
		im_size.append(temp);
		im_size.append(",");
		temp.setNum(IM->SZ);
		im_size.append(temp);

		QString text;
		text.setNum(N_Snakes);
		text.prepend("\n #Snakes:");

		im_size.append(text);

		cornerAnnotation->SetText( 2, im_size.toStdString().c_str());*/

		Vector3D v1, v2, v3, vtemp;

		//draw boundary as spheres
		if( slider->getRadiusDisplay() == 2 || slider->getRadiusDisplay() == 3 )
		{
			int sample = 1;
			for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
			{
				if( Tracer->SnakeList.valid_list[i] == 0)
					continue;
				for( int j = 0; j < Tracer->SnakeList.Snakes[i].Ru.size(); j += sample )
				{

					if( j == 0 )
					{
						v1.x = Tracer->SnakeList.Snakes[i].Cu.Pt[0].x - Tracer->SnakeList.Snakes[i].Cu.Pt[1].x;
						v1.y = Tracer->SnakeList.Snakes[i].Cu.Pt[0].y - Tracer->SnakeList.Snakes[i].Cu.Pt[1].y;
						v1.z = Tracer->SnakeList.Snakes[i].Cu.Pt[0].z - Tracer->SnakeList.Snakes[i].Cu.Pt[1].z;
						v1.ConvertUnit();
					}
					else
					{
						v1.x = Tracer->SnakeList.Snakes[i].Cu.Pt[j].x - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].x;
						v1.y = Tracer->SnakeList.Snakes[i].Cu.Pt[j].y - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].y;
						v1.z = Tracer->SnakeList.Snakes[i].Cu.Pt[j].z - Tracer->SnakeList.Snakes[i].Cu.Pt[j-1].z;
						v1.ConvertUnit();
					}

					// Create a circle
					if( Tracer->SnakeList.Snakes[i].Ru1.size() == 0 )
					{
						vtkSmartPointer<vtkRegularPolygonSource> polygonSource =
							vtkSmartPointer<vtkRegularPolygonSource>::New();

						//polygonSource->GeneratePolygonOff();
						polygonSource->SetNumberOfSides(20);
						polygonSource->SetGeneratePolygon(0);
						//polygonSource->SetRadius(Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
						polygonSource->SetRadius(Tracer->SnakeList.Snakes[i].Ru[j] * general_para->getSh());
						polygonSource->SetCenter(Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh() ,
							Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
							Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
						polygonSource->SetNormal(v1.x,v1.y,v1.z);
						polygonSource->Update();


						vtkSmartPointer<vtkPolyDataMapper> boundary_mapper = 
							vtkSmartPointer<vtkPolyDataMapper>::New();
						boundary_mapper->SetInputConnection(polygonSource->GetOutputPort());

						vtkSmartPointer<vtkActor> boundary_actor = 
							vtkSmartPointer<vtkActor>::New();
						boundary_actor->SetMapper(boundary_mapper);
						boundary_actor->GetProperty()->SetColor(0,0,1);
						boundary_actor->GetProperty()->SetLineWidth(2);

						boundary_actors.push_back(boundary_actor);

						this->Renderer->AddActor(boundary_actor);
					}
					else
					{
						int N_points = 16;
						vtkSmartPointer<vtkPoints> C_points =
							vtkSmartPointer<vtkPoints>::New();
						vtkSmartPointer<vtkPolyLine> C_polygon =
							vtkSmartPointer<vtkPolyLine>::New();
						C_polygon->GetPointIds()->SetNumberOfIds(N_points+1);

						float pi = 3.1415926;
						v2.x = -1 * v1.x * v1.z;
						v2.y = -1 * v1.y * v1.z;
						v2.z = pow(v1.x,2) + pow(v1.y,2);
						v2.ConvertUnit();
						v3.x = -1 * v1.y;
						v3.y = v1.x;
						v3.z = 0;
						v3.ConvertUnit();
						for( int angle = 0; angle <= N_points; angle++ )
						{
							float theta = (2 * pi * angle)/N_points;

							vtemp.x = Tracer->SnakeList.Snakes[i].Ru1[j] * v2.x * cos(theta) + Tracer->SnakeList.Snakes[i].Ru[j] * v3.x * sin(theta) * general_para->getSh();
							vtemp.y = Tracer->SnakeList.Snakes[i].Ru1[j] * v2.y * cos(theta) + Tracer->SnakeList.Snakes[i].Ru[j] * v3.y * sin(theta) * general_para->getSh();
							vtemp.z = Tracer->SnakeList.Snakes[i].Ru1[j] * v2.z * cos(theta) + Tracer->SnakeList.Snakes[i].Ru[j] * v3.z * sin(theta) * general_para->getSh();
							double p0[3] = {Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh() + vtemp.x, Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh() + vtemp.y, Tracer->SnakeList.Snakes[i].Cu.Pt[j].z + vtemp.z};
							C_points->InsertNextPoint(p0);
							C_polygon->GetPointIds()->SetId(angle, angle);
						}


						vtkSmartPointer<vtkCellArray> C_cells = 
							vtkSmartPointer<vtkCellArray>::New();
						C_cells->InsertNextCell(C_polygon);

						// Create a polydata to store everything in
						vtkSmartPointer<vtkPolyData> C_polyData = 
							vtkSmartPointer<vtkPolyData>::New();

						// Add the points to the dataset
						C_polyData->SetPoints(C_points);

						// Add the lines to the dataset
						//C_polyData->SetPolys(C_cells);
						C_polyData->SetLines(C_cells);

						vtkSmartPointer<vtkPolyDataMapper> boundary_mapper1 = 
							vtkSmartPointer<vtkPolyDataMapper>::New();
						boundary_mapper1->SetInput(C_polyData);

						vtkSmartPointer<vtkActor> boundary_actor1 = 
							vtkSmartPointer<vtkActor>::New();
						boundary_actor1->SetMapper(boundary_mapper1);
						boundary_actor1->GetProperty()->SetColor(0,0,1);
						boundary_actor1->GetProperty()->SetLineWidth(2);

						boundary_actors.push_back(boundary_actor1);

						this->Renderer->AddActor(boundary_actor1);


					}

					/*// Create another circle
					if( Tracer->SnakeList.Snakes[i].Ru1.size() != 0 )
					{
					vtkSmartPointer<vtkRegularPolygonSource> polygonSource =
					vtkSmartPointer<vtkRegularPolygonSource>::New();

					//polygonSource->GeneratePolygonOff();
					polygonSource->SetNumberOfSides(20);
					polygonSource->SetGeneratePolygon(0);
					//polygonSource->SetRadius(Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					polygonSource->SetRadius(Tracer->SnakeList.Snakes[i].Ru1[j] * general_para->getSh());
					polygonSource->SetCenter(Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh() ,
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
					Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
					polygonSource->SetNormal(v1.x,v1.y,v1.z);
					polygonSource->Update();


					vtkSmartPointer<vtkPolyDataMapper> boundary_mapper = 
					vtkSmartPointer<vtkPolyDataMapper>::New();
					boundary_mapper->SetInputConnection(polygonSource->GetOutputPort());

					vtkSmartPointer<vtkActor> boundary_actor = 
					vtkSmartPointer<vtkActor>::New();
					boundary_actor->SetMapper(boundary_mapper);
					boundary_actor->GetProperty()->SetColor(1,0,0);
					boundary_actor->GetProperty()->SetLineWidth(2);

					boundary_actors.push_back(boundary_actor);

					this->Renderer->AddActor(boundary_actor);
					}*/

				}
			}
		}
		if( slider->getRadiusDisplay() == 1 || slider->getRadiusDisplay() == 3  )
		{
			int sample = 1;
			for( int i = 0; i < Tracer->SnakeList.NSnakes; i++ )
			{
				if( Tracer->SnakeList.valid_list[i] == 0)
					continue;
				for( int j = 1; j < Tracer->SnakeList.Snakes[i].Ru.size()-1; j += sample )
				{
					//Create a sphere
					vtkSmartPointer<vtkSphereSource> sphereSource = 
						vtkSmartPointer<vtkSphereSource>::New();
					sphereSource->SetCenter(Tracer->SnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh() ,
						Tracer->SnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
						Tracer->SnakeList.Snakes[i].Cu.Pt[j].z);
					//sphereSource->SetRadius(Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					sphereSource->SetRadius(Tracer->SnakeList.Snakes[i].Ru[j] * general_para->getSh());

					//Create a mapper and actor
					vtkSmartPointer<vtkPolyDataMapper> boundary_mapper = 
						vtkSmartPointer<vtkPolyDataMapper>::New();
					boundary_mapper->SetInputConnection(sphereSource->GetOutputPort());

					vtkSmartPointer<vtkActor> boundary_actor = 
						vtkSmartPointer<vtkActor>::New();
					boundary_actor->SetMapper(boundary_mapper);
					//boundary_actor->GetProperty()->SetColor(0,1,0);
					//boundary_actor->GetProperty()->SetDiffuseColor(0,1,0);
					//boundary_actor->GetProperty()->SetSpecularColor(1, 1, 1);
					//boundary_actor->GetProperty()->SetSpecular(0.4);
					//boundary_actor->GetProperty()->SetSpecularPower(50);

					boundary_actor->GetProperty()->SetDiffuseColor(double(72)/255,double(118)/255,1);
					boundary_actor->GetProperty()->SetSpecularColor(1, 1, 1);
					boundary_actor->GetProperty()->SetSpecular(0.4);
					boundary_actor->GetProperty()->SetSpecularPower(50);

					boundary_actors.push_back(boundary_actor);

					this->Renderer->AddActor(boundary_actor);
				}
			}
		}
	}

	/*else
	{

	if( line_actors.size() != 0 )
	{
	for( int i = 0; i < line_actors.size(); i++ )
	{
	this->Renderer->RemoveActor(line_actors[i]);
	}
	line_actors.clear();

	this->Renderer->RemoveActor(this->point_actor);
	}
	}*/

	this->QVTK->GetRenderWindow()->Render();
}

void QtTracer::draw3DTraces_Global()
{
	if( tracing_line_actor )
		this->Renderer->RemoveActor(tracing_line_actor);

	vtkSmartPointer<vtkPoints> line_tips;
	vtkSmartPointer<vtkCellArray> point_cells;
	vtkSmartPointer<vtkPolyData> PolyPoints;
	line_tips = vtkSmartPointer<vtkPoints>::New();
	point_cells = vtkSmartPointer<vtkCellArray>::New();
	PolyPoints = vtkSmartPointer<vtkPolyData>::New();

	//draw seeds
	if( slider->getDisplaySeed() )
	{
		vtkSmartPointer<vtkPoints> seed_points;
		vtkSmartPointer<vtkCellArray> seed_point_cells;
		vtkSmartPointer<vtkPolyData> seed_PolyPoints;
		seed_points = vtkSmartPointer<vtkPoints>::New();
		seed_point_cells = vtkSmartPointer<vtkCellArray>::New();
		seed_PolyPoints = vtkSmartPointer<vtkPolyData>::New();
		for( int i = 0; i < IM->SeedPt.NP; i++ )
		{
			if ( IM->visit_label(i) == 1 )
				continue;

			double point[3];
			point[0] = IM->SeedPt.Pt[i].x * general_para->getSh(); 
			point[1] = IM->SeedPt.Pt[i].y * general_para->getSh(); 
			point[2] = IM->SeedPt.Pt[i].z;
			unsigned int r_id = seed_points->InsertNextPoint(point);
			vtkIdType con[1]; 
			con[0] = r_id;
			seed_point_cells->InsertNextCell(1,con);
		}
		seed_PolyPoints->SetPoints(seed_points);
		seed_PolyPoints->SetVerts(seed_point_cells);

		if(this->seed_actor)
		{
			this->Renderer->RemoveActor(this->seed_actor);
		}

		vtkSmartPointer<vtkPolyDataMapper> seed_polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
		seed_polymap->SetInput(seed_PolyPoints);
		this->seed_actor = vtkSmartPointer<vtkActor>::New();
		this->seed_actor->SetMapper(seed_polymap);
		this->seed_actor->GetProperty()->SetColor(0,0,1);


		this->seed_actor->GetProperty()->SetPointSize(3);
		this->seed_actor->GetProperty()->SetEdgeColor(1,1,1);

		this->Renderer->AddActor(this->seed_actor);
	}
	else if(this->seed_actor)
	{
		this->Renderer->RemoveActor(this->seed_actor);
	}


	//delete previous actors
	if( tube_actors.size() != 0 )
	{
		for( int i = 0; i < tube_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(tube_actors[i]);
		}
		tube_actors.clear();
	}
	if( line_actors.size() != 0 )
	{
		for( int i = 0; i < line_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(line_actors[i]);
		}
		line_actors.clear();

		this->Renderer->RemoveActor(this->point_actor);
	}
	if( branch_actors.size() != 0 )
	{
		for( int i = 0; i < branch_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(branch_actors[i]);
		}
		branch_actors.clear();
	}
	if( boundary_actors.size() != 0 )
	{
		for( int i = 0; i < boundary_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(boundary_actors[i]);
		}
		boundary_actors.clear();
	}

	Vector3D temp;
	float unit_length = general_para2->getPtDistance();
	/* float color1 = (double)(rand()%10)/(double)10;
	float color2 = (double)(rand()%10)/(double)10;
	float color3 = (double)(rand()%10)/(double)10;
	std::cout<<"color:"<<color1<<","<<color2<<","<<color3<<std::endl; */
	//draw traces
	if( slider->getDisplaySnakes() )
	{
		//draw the tube
		/*if( slider->getTubeDisplay() )
		{
		for( int i = 0; i < GSnakeList.NSnakes; i++ )
		{   
		if( GSnakeList.valid_list[i] == 0)
		continue;
		for( int j = 0; j <GSnakeList.Snakes[i].Cu.NP-1; j++ )
		{   
		vtkSmartPointer<vtkLineSource> lineSource = 
		vtkSmartPointer<vtkLineSource>::New();

		if( j == 0 )
		{
		float x1, y1, z1;
		x1 = GSnakeList.Snakes[i].Cu.Pt[j].x;
		y1 = GSnakeList.Snakes[i].Cu.Pt[j].y;
		z1 = GSnakeList.Snakes[i].Cu.Pt[j].z;
		lineSource->SetPoint1(x1,y1,z1);
		float x2, y2, z2;
		temp.x = GSnakeList.Snakes[i].Cu.Pt[j+1].x - GSnakeList.Snakes[i].Cu.Pt[j].x;
		temp.y = GSnakeList.Snakes[i].Cu.Pt[j+1].y - GSnakeList.Snakes[i].Cu.Pt[j].y;
		temp.z = GSnakeList.Snakes[i].Cu.Pt[j+1].z - GSnakeList.Snakes[i].Cu.Pt[j].z;
		temp.ConvertUnit();
		x2 = x1 + temp.x * unit_length * 2/3;
		y2 = y1 + temp.y * unit_length * 2/3;
		z2 = z1 + temp.z * unit_length * 2/3;
		lineSource->SetPoint2(x2,y2,z2);
		}
		else if( j == GSnakeList.Snakes[i].Cu.NP-1 )
		{
		float x1, y1, z1;
		x1 = GSnakeList.Snakes[i].Cu.Pt[j].x;
		y1 = GSnakeList.Snakes[i].Cu.Pt[j].y;
		z1 = GSnakeList.Snakes[i].Cu.Pt[j].z;
		lineSource->SetPoint1(x1,y1,z1);
		float x2, y2, z2;
		temp.x = GSnakeList.Snakes[i].Cu.Pt[j].x - GSnakeList.Snakes[i].Cu.Pt[j-1].x;
		temp.y = GSnakeList.Snakes[i].Cu.Pt[j].y - GSnakeList.Snakes[i].Cu.Pt[j-1].y;
		temp.z = GSnakeList.Snakes[i].Cu.Pt[j].z - GSnakeList.Snakes[i].Cu.Pt[j-1].z;
		temp.ConvertUnit();
		x2 = x1 - temp.x * unit_length * 2/3;
		y2 = y1 - temp.y * unit_length * 2/3;
		z2 = z1 - temp.z * unit_length * 2/3;
		lineSource->SetPoint2(x2,y2,z2);
		}
		else
		{
		temp.x = (GSnakeList.Snakes[i].Cu.Pt[j].x - GSnakeList.Snakes[i].Cu.Pt[j-1].x)/2 + 
		(GSnakeList.Snakes[i].Cu.Pt[j+1].x - GSnakeList.Snakes[i].Cu.Pt[j].x)/2;
		temp.y = (GSnakeList.Snakes[i].Cu.Pt[j].y - GSnakeList.Snakes[i].Cu.Pt[j-1].y)/2 + 
		(GSnakeList.Snakes[i].Cu.Pt[j+1].y - GSnakeList.Snakes[i].Cu.Pt[j].y)/2;
		temp.z = (GSnakeList.Snakes[i].Cu.Pt[j].z - GSnakeList.Snakes[i].Cu.Pt[j-1].z)/2 + 
		(GSnakeList.Snakes[i].Cu.Pt[j+1].z - GSnakeList.Snakes[i].Cu.Pt[j].z)/2;
		temp.ConvertUnit();
		float x1, y1, z1;
		x1 = GSnakeList.Snakes[i].Cu.Pt[j].x - temp.x * unit_length * 2/3;
		y1 = GSnakeList.Snakes[i].Cu.Pt[j].y - temp.y * unit_length * 2/3;
		z1 = GSnakeList.Snakes[i].Cu.Pt[j].z - temp.z * unit_length * 2/3;
		lineSource->SetPoint1(x1,y1,z1);
		float x2, y2, z2;
		x2 = GSnakeList.Snakes[i].Cu.Pt[j].x + temp.x * unit_length * 2/3;
		y2 = GSnakeList.Snakes[i].Cu.Pt[j].y + temp.y * unit_length * 2/3;
		z2 = GSnakeList.Snakes[i].Cu.Pt[j].z + temp.z * unit_length * 2/3;
		lineSource->SetPoint2(x2,y2,z2);
		}

		vtkSmartPointer<vtkTubeFilter> tubeFilter = 
		vtkSmartPointer<vtkTubeFilter>::New();
		tubeFilter->SetInputConnection(lineSource->GetOutputPort());
		tubeFilter->SetRadius(GSnakeList.Snakes[i].Ru[j] * sqrt((double)2));
		tubeFilter->SetNumberOfSides(10);
		tubeFilter->Update();

		vtkSmartPointer<vtkPolyDataMapper> tubeMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
		tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
		vtkSmartPointer<vtkActor> tubeActor = 
		vtkSmartPointer<vtkActor>::New();
		tubeActor->SetMapper(tubeMapper);

		tubeActor->GetProperty()->SetColor(0.6,0.9,0.1);
		this->Renderer->AddActor(tubeActor);
		tube_actors.push_back(tubeActor);
		}
		}
		} */


		//vnl_vector<int> selected_snake_id =tracingViewer->getSelectedSnakes();
		for( int i = 0; i < GSnakeList.NSnakes; i++ )
		{

			if( GSnakeList.valid_list[i] == 0)
				continue;

			//bool selected = false;
			//if(selected_snake_id(i) == 1)
			//   selected = true;

			vtkSmartPointer<vtkPoints> line_points;
			vtkSmartPointer<vtkCellArray> line_cells;
			vtkSmartPointer<vtkPolyData> PolyTraces;
			line_points = vtkSmartPointer<vtkPoints>::New();
			line_cells = vtkSmartPointer<vtkCellArray>::New();
			PolyTraces = vtkSmartPointer<vtkPolyData>::New();

			double color[3];
			color[0] = 0;
			color[1] = 1;
			color[2] = 0;


			/*for( int j = 0; j <GSnakeList.Snakes[i].Cu.NP-1; j++ )
			{
			double point[3];
			double point1[3];

			point[0] = GSnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh(); 
			point[1] = GSnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(); 
			point[2] = GSnakeList.Snakes[i].Cu.Pt[j].z;

			point1[0] = GSnakeList.Snakes[i].Cu.Pt[j+1].x * general_para->getSh(); 
			point1[1] = GSnakeList.Snakes[i].Cu.Pt[j+1].y * general_para->getSh(); 
			point1[2] = GSnakeList.Snakes[i].Cu.Pt[j+1].z;

			unsigned int return_id = line_points->InsertNextPoint(point);
			unsigned int return_id1 = line_points->InsertNextPoint(point1);

			if( j == 0 )
			{
			unsigned int r_id = line_tips->InsertNextPoint(point);
			vtkIdType con[1]; 
			con[0] = r_id;
			point_cells->InsertNextCell(1,con);
			}
			if( j == seed_snakes.Snakes[i].Cu.NP - 2 )
			{
			unsigned int r_id = line_tips->InsertNextPoint(point1);
			vtkIdType con[1]; 
			con[0] = r_id;
			point_cells->InsertNextCell(1,con);
			}

			vtkIdType connectivity[2]; 
			connectivity[0] = return_id; 
			connectivity[1] = return_id1; 
			line_cells->InsertNextCell(2,connectivity); 
			}*/

			for( int j = 0; j <GSnakeList.Snakes[i].Cu.NP; j++ )
			{
				line_points->InsertPoint(j, GSnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh(), 
					GSnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
					GSnakeList.Snakes[i].Cu.Pt[j].z);
			}

			line_cells->InsertNextCell(GSnakeList.Snakes[i].Cu.NP);
			for (int j = 0; j < GSnakeList.Snakes[i].Cu.NP; j++)
			{
				line_cells->InsertCellPoint(j);
			}

			PolyTraces->SetPoints(line_points);
			PolyTraces->SetLines(line_cells);

			//draw tubes with varying radius
			if( slider->getTubeDisplay() )
			{
				// Varying tube radius 
				vtkSmartPointer<vtkDoubleArray> tubeRadius =
					vtkSmartPointer<vtkDoubleArray>::New();
				tubeRadius->SetName("TubeRadius");
				tubeRadius->SetNumberOfTuples(GSnakeList.Snakes[i].Cu.NP);

				vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
				colors->SetName("Colors");
				colors->SetNumberOfComponents(3);
				colors->SetNumberOfTuples(GSnakeList.Snakes[i].Cu.NP);

				for (int j=0 ;j<GSnakeList.Snakes[i].Cu.NP ; j++)
				{
					//tubeRadius->SetTuple1(j,GSnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					tubeRadius->SetTuple1(j,GSnakeList.Snakes[i].Ru[j] * general_para->getSh());
					colors->InsertTuple3(j,int(1 * 255),int(1 * 255),int(0 * 255));
					//colors->InsertTuple3(j,int((GSnakeList.Snakes[i].Ru[j] * 30)),0,int((255 - GSnakeList.Snakes[i].Ru[j] * 30)));
				}

				PolyTraces->GetPointData()->AddArray(tubeRadius);
				PolyTraces->GetPointData()->AddArray(colors);
				PolyTraces->GetPointData()->SetActiveScalars("TubeRadius");

				vtkSmartPointer<vtkTubeFilter> tubeFilter = 
					vtkSmartPointer<vtkTubeFilter>::New();
				tubeFilter->SetInput(PolyTraces);
				tubeFilter->SetNumberOfSides(10);
				tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
				tubeFilter->Update();

				vtkSmartPointer<vtkPolyDataMapper> tubeMapper = 
					vtkSmartPointer<vtkPolyDataMapper>::New();
				tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
				tubeMapper->ScalarVisibilityOn();
				tubeMapper->SetScalarModeToUsePointFieldData();
				tubeMapper->SelectColorArray("Colors");

				vtkSmartPointer<vtkActor> tubeActor = 
					vtkSmartPointer<vtkActor>::New();
				tubeActor->SetMapper(tubeMapper);
				//tubeActor->GetProperty()->SetColor(0.6,0.9,0.1);
				this->Renderer->AddActor(tubeActor);

				tube_actors.push_back(tubeActor);
			}

			vtkSmartPointer<vtkPolyDataMapper> polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
			polymap->SetInput(PolyTraces);

			vtkSmartPointer<vtkActor> line_actor = vtkSmartPointer<vtkActor>::New();
			line_actor->SetMapper(polymap);


			if( slider->getColorDisplay() )
			{
				/*if( i%6 == 0 )
				line_actor->GetProperty()->SetColor(1,0,0);
				else if( i%6 == 1 )
				line_actor->GetProperty()->SetColor(0,1,0);
				else if( i%6 == 2 )
				line_actor->GetProperty()->SetColor(0,0,1);
				else if( i%6 == 3 )
				line_actor->GetProperty()->SetColor(1,1,0);
				else if( i%6 == 4 )
				line_actor->GetProperty()->SetColor(1,0,1);
				else if( i%6 == 5 )
				line_actor->GetProperty()->SetColor(0,1,1);*/

				//assign the color for traces grown from soma
				if( IM->num_soma != 0 )
				{
					LabelImageType::IndexType new_index;
					new_index[0] = GSnakeList.Snakes[i].Cu.GetMiddlePt().x;
					new_index[1] = GSnakeList.Snakes[i].Cu.GetMiddlePt().y; 
					new_index[2] = GSnakeList.Snakes[i].Cu.GetMiddlePt().z; 

					int label = IM->IVoronoi->GetPixel(new_index);

					if( label != 0 )
					{
						color[0] = soma_color(label-1,0);
						color[1] = soma_color(label-1,1);
						color[2] = soma_color(label-1,2);
					}
					line_actor->GetProperty()->SetColor(color[0],color[1],color[2]);
				}
				else
				{
					line_actor->GetProperty()->SetColor((double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10);
				}
			}
			else
				line_actor->GetProperty()->SetColor(color[0],color[1],color[2]);


			//if( selected )
			//{
			//	line_actor->GetProperty()->SetColor(1,0,0);
			//   line_actor->GetProperty()->SetPointSize(1);
			//   line_actor->GetProperty()->SetLineWidth(5);
			//}
			//else
			//{
			line_actor->GetProperty()->SetPointSize(1);
			line_actor->GetProperty()->SetLineWidth(LineWidth);
			//}
			this->Renderer->AddActor(line_actor);

			line_actors.push_back(line_actor);
		}


		//draw the tips
		/*PolyPoints->SetPoints(line_tips);
		PolyPoints->SetVerts(point_cells);

		vtkSmartPointer<vtkPolyDataMapper> polymap1 = vtkSmartPointer<vtkPolyDataMapper>::New();
		polymap1->SetInput(PolyPoints);

		this->point_actor = vtkSmartPointer<vtkActor>::New();
		this->point_actor->SetMapper(polymap1);
		this->point_actor->GetProperty()->SetColor(1,0,0);
		this->point_actor->GetProperty()->SetPointSize(5);
		this->Renderer->AddActor(this->point_actor);*/

		//draw branch points
		for( int i = 0; i < GSnakeList.branch_points.NP; i++ )
		{
			vtkSmartPointer<vtkSphereSource> sphereSource = 
				vtkSmartPointer<vtkSphereSource>::New();
			sphereSource->SetCenter(GSnakeList.branch_points.Pt[i].x * general_para->getSh() ,
				GSnakeList.branch_points.Pt[i].y * general_para->getSh(),
				GSnakeList.branch_points.Pt[i].z);
			sphereSource->SetRadius(2);

			//Create a mapper and actor
			vtkSmartPointer<vtkPolyDataMapper> branch_mapper = 
				vtkSmartPointer<vtkPolyDataMapper>::New();
			branch_mapper->SetInputConnection(sphereSource->GetOutputPort());

			vtkSmartPointer<vtkActor> branch_actor = 
				vtkSmartPointer<vtkActor>::New();
			branch_actor->SetMapper(branch_mapper);
			branch_actor->GetProperty()->SetOpacity(0.6);
			branch_actors.push_back(branch_actor);

			this->Renderer->AddActor(branch_actor);
		}

		//int N_Snakes = 0;
		for( int i = 0; i < GSnakeList.NSnakes; i++ )
		{
			if( GSnakeList.valid_list[i] == 0)
				continue;
			//N_Snakes++;
			/*for( int j = 0; j < GSnakeList.Snakes[i].BranchPt.NP; j++ )
			{
			//Create a sphere
			vtkSmartPointer<vtkSphereSource> sphereSource = 
			vtkSmartPointer<vtkSphereSource>::New();
			sphereSource->SetCenter(GSnakeList.Snakes[i].BranchPt.Pt[j].x * general_para->getSh() ,
			GSnakeList.Snakes[i].BranchPt.Pt[j].y * general_para->getSh(),
			GSnakeList.Snakes[i].BranchPt.Pt[j].z);
			sphereSource->SetRadius(2);

			//Create a mapper and actor
			vtkSmartPointer<vtkPolyDataMapper> branch_mapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
			branch_mapper->SetInputConnection(sphereSource->GetOutputPort());

			vtkSmartPointer<vtkActor> branch_actor = 
			vtkSmartPointer<vtkActor>::New();
			branch_actor->SetMapper(branch_mapper);
			branch_actors.push_back(branch_actor);

			this->Renderer->AddActor(branch_actor);
			}*/

			//draw root points
			for( int j = 0; j < GSnakeList.Snakes[i].RootPt.NP; j++ )
			{
				//Create a sphere
				vtkSmartPointer<vtkSphereSource> sphereSource = 
					vtkSmartPointer<vtkSphereSource>::New();
				sphereSource->SetCenter(GSnakeList.Snakes[i].RootPt.Pt[j].x * general_para->getSh() ,
					GSnakeList.Snakes[i].RootPt.Pt[j].y * general_para->getSh(),
					GSnakeList.Snakes[i].RootPt.Pt[j].z);
				sphereSource->SetRadius(3);

				//Create a mapper and actor
				vtkSmartPointer<vtkPolyDataMapper> root_mapper = 
					vtkSmartPointer<vtkPolyDataMapper>::New();
				root_mapper->SetInputConnection(sphereSource->GetOutputPort());

				vtkSmartPointer<vtkActor> root_actor = 
					vtkSmartPointer<vtkActor>::New();
				root_actor->SetMapper(root_mapper);

				root_actor->GetProperty()->SetColor(1,1,0);

				branch_actors.push_back(root_actor);


				this->Renderer->AddActor(root_actor);
			}
		}

		//draw text in the upper left corner
		/*QString im_size("Size:");
		QString temp;
		temp.setNum(IM->SM);
		im_size.append(temp);
		im_size.append(",");
		temp.setNum(IM->SN);
		im_size.append(temp);
		im_size.append(",");
		temp.setNum(IM->SZ);
		im_size.append(temp);

		QString text;
		text.setNum(N_Snakes);
		text.prepend("\n #Snakes:");

		im_size.append(text);

		cornerAnnotation->SetText( 2, im_size.toStdString().c_str());*/

		Vector3D v1;

		//draw boundary as spheres
		if( slider->getRadiusDisplay() == 2 || slider->getRadiusDisplay() == 3 )
		{
			int sample = 1;
			for( int i = 0; i < GSnakeList.NSnakes; i++ )
			{
				if( GSnakeList.valid_list[i] == 0)
					continue;
				for( int j = 0; j < GSnakeList.Snakes[i].Ru.size(); j += sample )
				{

					if( j == 0 )
					{
						v1.x = GSnakeList.Snakes[i].Cu.Pt[0].x - GSnakeList.Snakes[i].Cu.Pt[1].x;
						v1.y = GSnakeList.Snakes[i].Cu.Pt[0].y - GSnakeList.Snakes[i].Cu.Pt[1].y;
						v1.z = GSnakeList.Snakes[i].Cu.Pt[0].z - GSnakeList.Snakes[i].Cu.Pt[1].z;
						v1.ConvertUnit();
					}
					else
					{
						v1.x = GSnakeList.Snakes[i].Cu.Pt[j].x - GSnakeList.Snakes[i].Cu.Pt[j-1].x;
						v1.y = GSnakeList.Snakes[i].Cu.Pt[j].y - GSnakeList.Snakes[i].Cu.Pt[j-1].y;
						v1.z = GSnakeList.Snakes[i].Cu.Pt[j].z - GSnakeList.Snakes[i].Cu.Pt[j-1].z;
						v1.ConvertUnit();
					}

					// Create a circle
					vtkSmartPointer<vtkRegularPolygonSource> polygonSource =
						vtkSmartPointer<vtkRegularPolygonSource>::New();

					//polygonSource->GeneratePolygonOff();
					polygonSource->SetNumberOfSides(20);
					polygonSource->SetGeneratePolygon(0);
					//polygonSource->SetRadius(GSnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					polygonSource->SetRadius(GSnakeList.Snakes[i].Ru[j] * general_para->getSh());
					polygonSource->SetCenter(GSnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh() ,
						GSnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
						GSnakeList.Snakes[i].Cu.Pt[j].z);
					polygonSource->SetNormal(v1.x,v1.y,v1.z);
					polygonSource->Update();


					vtkSmartPointer<vtkPolyDataMapper> boundary_mapper = 
						vtkSmartPointer<vtkPolyDataMapper>::New();
					boundary_mapper->SetInputConnection(polygonSource->GetOutputPort());

					vtkSmartPointer<vtkActor> boundary_actor = 
						vtkSmartPointer<vtkActor>::New();
					boundary_actor->SetMapper(boundary_mapper);
					boundary_actor->GetProperty()->SetColor(0,0,1);
					boundary_actor->GetProperty()->SetLineWidth(2);
					boundary_actors.push_back(boundary_actor);

					this->Renderer->AddActor(boundary_actor);
				}
			}
		}
		if( slider->getRadiusDisplay() == 1 || slider->getRadiusDisplay() == 3  )
		{
			int sample = 1;
			for( int i = 0; i < GSnakeList.NSnakes; i++ )
			{
				if( GSnakeList.valid_list[i] == 0)
					continue;
				for( int j = 1; j < GSnakeList.Snakes[i].Ru.size()-1; j += sample )
				{
					//Create a sphere
					vtkSmartPointer<vtkSphereSource> sphereSource = 
						vtkSmartPointer<vtkSphereSource>::New();
					sphereSource->SetCenter(GSnakeList.Snakes[i].Cu.Pt[j].x * general_para->getSh() ,
						GSnakeList.Snakes[i].Cu.Pt[j].y * general_para->getSh(),
						GSnakeList.Snakes[i].Cu.Pt[j].z);
					//sphereSource->SetRadius(GSnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					sphereSource->SetRadius(GSnakeList.Snakes[i].Ru[j] * general_para->getSh());

					//Create a mapper and actor
					vtkSmartPointer<vtkPolyDataMapper> boundary_mapper = 
						vtkSmartPointer<vtkPolyDataMapper>::New();
					boundary_mapper->SetInputConnection(sphereSource->GetOutputPort());

					vtkSmartPointer<vtkActor> boundary_actor = 
						vtkSmartPointer<vtkActor>::New();
					boundary_actor->SetMapper(boundary_mapper);
					boundary_actor->GetProperty()->SetColor(0,1,0);
					boundary_actors.push_back(boundary_actor);

					this->Renderer->AddActor(boundary_actor);
				}
			}
		}
	}

	/*else
	{

	if( line_actors.size() != 0 )
	{
	for( int i = 0; i < line_actors.size(); i++ )
	{
	this->Renderer->RemoveActor(line_actors[i]);
	}
	line_actors.clear();

	this->Renderer->RemoveActor(this->point_actor);
	}
	}*/

	this->QVTK->GetRenderWindow()->Render();
}

void QtTracer::changeBrightness(int in)
{
	this->ImageActors->setBrightness((float)in);
	//this->Camera->Azimuth(in);
	this->QVTK->GetRenderWindow()->Render();
}
void QtTracer::changeOpacity(int in)
{
	this->ImageActors->setOpacityValue((float)in/100);
	//this->Camera->Azimuth(in);
	this->QVTK->GetRenderWindow()->Render();
}
void QtTracer::changeOpacityTh(int in)
{
	this->ImageActors->setOpacity((float)in);
	this->QVTK->GetRenderWindow()->Render();
}

void QtTracer::changeLineWidth(int in)
{
	LineWidth = in;
	//this->QVTK->GetRenderWindow()->Render();
	draw3DTraces();
}
void QtTracer::drawVolume(int x, int y, int z)
{

	//this->Renderer->RemoveAllViewProps();

	//3D Rendering
	this->ImageActors->loadImage(file.toStdString(), "Image", x, y ,z);
	this->Renderer->AddVolume(this->ImageActors->RayCastVolume(-1));
	this->ImageActors->setRenderStatus(-1, true);

	//draw outline
	double bounds[6];
	this->ImageActors->RayCastVolume(-1)->GetBounds(bounds);
	vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
	cubeSource->SetBounds(bounds);

	vtkSmartPointer<vtkPolyDataMapper> cubeMapper = 
		vtkSmartPointer<vtkPolyDataMapper>::New();
	cubeMapper->SetInputConnection(cubeSource->GetOutputPort());

	vtkSmartPointer<vtkActor> cubeActor = 
		vtkSmartPointer<vtkActor>::New();
	cubeActor->SetMapper(cubeMapper);
	cubeActor->GetProperty()->SetRepresentationToWireframe();
	cubeActor->GetProperty()->SetColor(1,0,0);
	this->Renderer->AddActor(cubeActor);

	this->QVTK->GetRenderWindow()->Render();

}



void QtTracer::drawVolume()
{
	if( !file_display.isEmpty())
	{
		this->Renderer->RemoveAllViewProps();

		//3D Rendering
		this->ImageActors->loadImage(file_display.toStdString(), "Image");
		this->Renderer->AddVolume(this->ImageActors->RayCastVolume(-1));

		this->ImageActors->setRenderStatus(-1, true);

		//draw outline
		double bounds[6];
		this->ImageActors->RayCastVolume(-1)->GetBounds(bounds);
		vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetBounds(bounds);

		vtkSmartPointer<vtkPolyDataMapper> cubeMapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
		cubeMapper->SetInputConnection(cubeSource->GetOutputPort());

		vtkSmartPointer<vtkActor> cubeActor = 
			vtkSmartPointer<vtkActor>::New();
		cubeActor->SetMapper(cubeMapper);
		cubeActor->GetProperty()->SetRepresentationToWireframe();
		cubeActor->GetProperty()->SetColor(1,0,0);
		this->Renderer->AddActor(cubeActor);

		this->QVTK->GetRenderWindow()->Render();
	}
	else if( !file.isEmpty() )
	{
		this->Renderer->RemoveAllViewProps();

		//3D Rendering
		this->ImageActors->loadImage(file.toStdString(), "Image");
		this->Renderer->AddVolume(this->ImageActors->RayCastVolume(-1));
		this->ImageActors->setRenderStatus(-1, true);

		//draw outline
		double bounds[6];
		this->ImageActors->RayCastVolume(-1)->GetBounds(bounds);
		vtkSmartPointer<vtkCubeSource> cubeSource = vtkSmartPointer<vtkCubeSource>::New();
		cubeSource->SetBounds(bounds);

		vtkSmartPointer<vtkPolyDataMapper> cubeMapper = 
			vtkSmartPointer<vtkPolyDataMapper>::New();
		cubeMapper->SetInputConnection(cubeSource->GetOutputPort());

		vtkSmartPointer<vtkActor> cubeActor = 
			vtkSmartPointer<vtkActor>::New();
		cubeActor->SetMapper(cubeMapper);
		cubeActor->GetProperty()->SetRepresentationToWireframe();
		cubeActor->GetProperty()->SetColor(1,0,0);
		this->Renderer->AddActor(cubeActor);

		this->QVTK->GetRenderWindow()->Render();

	}

	/* //add text
	cornerAnnotation = vtkSmartPointer<vtkCornerAnnotation>::New();
	cornerAnnotation->SetMaximumFontSize( 20 );
	cornerAnnotation->SetText( 2, "#Snakes:" );
	cornerAnnotation->GetTextProperty()->SetColor( 0,0,1);
	this->Renderer->AddViewProp(cornerAnnotation);*/


	/*vtkSmartPointer<vtkWorldPointPicker> worldPointPicker = 
	vtkSmartPointer<vtkWorldPointPicker>::New();

	vtkSmartPointer<vtkRenderWindowInteractor> renderWindowInteractor = 
	vtkSmartPointer<vtkRenderWindowInteractor>::New();
	renderWindowInteractor->SetPicker(worldPointPicker);
	renderWindowInteractor->SetRenderWindow(this->QVTK->GetRenderWindow());

	vtkSmartPointer<MouseInteractorStyle> style = 
	vtkSmartPointer<MouseInteractorStyle>::New();
	renderWindowInteractor->SetInteractorStyle( style );
	renderWindowInteractor->Start();*/

}


void QtTracer::draw3DTracing(SnakeClass s)
{

	/*Renderer->GetActiveCamera()->SetFocalPoint(s.Cu.GetMiddlePt().x,
	s.Cu.GetMiddlePt().y,
	s.Cu.GetMiddlePt().z);*/

	/*double vup[3];
	Vector3D v1;
	v1.x = s.Cu.Pt[s.Cu.NP-1].x - s.Cu.Pt[s.Cu.NP-2].x;
	v1.y = s.Cu.Pt[s.Cu.NP-1].y - s.Cu.Pt[s.Cu.NP-2].y;
	v1.z = s.Cu.Pt[s.Cu.NP-1].z - s.Cu.Pt[s.Cu.NP-2].z;
	v1.ConvertUnit();
	vup[0] = v1.x;
	vup[1] = v1.y;
	vup[2] = v1.z;
	Renderer->GetActiveCamera()->SetViewUp(vup);*/

	if( !slider->getDynamicDisplay() )
		return;

	//delete previous actors
	if( tube_actors.size() != 0 )
	{
		for( int i = 0; i < tube_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(tube_actors[i]);
		}
		tube_actors.clear();
	}

	//draw the tracing snake
	//remove previous tracing line actor
	if( tracing_line_actor )
		this->Renderer->RemoveActor(tracing_line_actor);

	vtkSmartPointer<vtkPoints> line_points;
	vtkSmartPointer<vtkCellArray> line_cells;
	vtkSmartPointer<vtkPolyData> PolyTraces;
	line_points = vtkSmartPointer<vtkPoints>::New();
	line_cells = vtkSmartPointer<vtkCellArray>::New();
	PolyTraces = vtkSmartPointer<vtkPolyData>::New();


	for( int j = 0; j <s.Cu.NP; j++ )
	{
		line_points->InsertPoint(j, s.Cu.Pt[j].x * general_para->getSh(), 
			s.Cu.Pt[j].y * general_para->getSh(),
			s.Cu.Pt[j].z);
	}

	line_cells->InsertNextCell(s.Cu.NP);
	for (int j = 0; j < s.Cu.NP; j++)
	{
		line_cells->InsertCellPoint(j);
	}

	PolyTraces->SetPoints(line_points);
	PolyTraces->SetLines(line_cells);

	//draw tubes with varying radius
	if( slider->getTubeDisplay() )
	{
				// Varying tube radius 
				vtkSmartPointer<vtkDoubleArray> tubeRadius =
					vtkSmartPointer<vtkDoubleArray>::New();
				tubeRadius->SetName("TubeRadius");
				tubeRadius->SetNumberOfTuples(s.Cu.NP);

				vtkSmartPointer<vtkUnsignedCharArray> colors = vtkSmartPointer<vtkUnsignedCharArray>::New();
				colors->SetName("Colors");
				colors->SetNumberOfComponents(3);
				colors->SetNumberOfTuples(s.Cu.NP);

				for (int j=0 ;j<s.Cu.NP ; j++)
				{
					//tubeRadius->SetTuple1(j,Tracer->SnakeList.Snakes[i].Ru[j] * sqrt((double)2));
					tubeRadius->SetTuple1(j,s.Ru[j] * general_para->getSh());
					//colors->InsertTuple3(j,int(1 * 255),int(1 * 255),int(0 * 255));
					//colors->InsertTuple3(j,int((Tracer->SnakeList.Snakes[i].Ru[j] * 30)),0,int((255 - Tracer->SnakeList.Snakes[i].Ru[j] * 30)));
				}

				PolyTraces->GetPointData()->AddArray(tubeRadius);
				//PolyTraces->GetPointData()->AddArray(colors);
				PolyTraces->GetPointData()->SetActiveScalars("TubeRadius");

				vtkSmartPointer<vtkTubeFilter> tubeFilter = 
					vtkSmartPointer<vtkTubeFilter>::New();
				tubeFilter->SetInput(PolyTraces);
				tubeFilter->SetNumberOfSides(10);
				tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();
				tubeFilter->Update();

				vtkSmartPointer<vtkPolyDataMapper> tubeMapper = 
					vtkSmartPointer<vtkPolyDataMapper>::New();
				tubeMapper->SetInputConnection(tubeFilter->GetOutputPort());
				tubeMapper->ScalarVisibilityOn();
				tubeMapper->SetScalarModeToUsePointFieldData();
				tubeMapper->SelectColorArray("Colors");

				vtkSmartPointer<vtkActor> tubeActor = 
					vtkSmartPointer<vtkActor>::New();
				tubeActor->SetMapper(tubeMapper);

				tubeActor->GetProperty()->SetDiffuseColor(double(72)/255,double(118)/255,1);
				tubeActor->GetProperty()->SetSpecularColor(1, 1, 1);
				tubeActor->GetProperty()->SetSpecular(0.4);
				tubeActor->GetProperty()->SetSpecularPower(50);
			    tubeActor->GetProperty()->SetRepresentationToWireframe();

				//tubeActor->GetProperty()->SetColor(0.6,0.9,0.1);
				this->Renderer->AddActor(tubeActor);

				tube_actors.push_back(tubeActor);
	}

	vtkSmartPointer<vtkPolyDataMapper> polymap = vtkSmartPointer<vtkPolyDataMapper>::New();
	polymap->SetInput(PolyTraces);

	tracing_line_actor = vtkSmartPointer<vtkActor>::New();
	tracing_line_actor->SetMapper(polymap);

	tracing_line_actor->GetProperty()->SetColor(1,0,0);
	tracing_line_actor->GetProperty()->SetPointSize(2);
	tracing_line_actor->GetProperty()->SetLineWidth(5);

	this->Renderer->AddActor(tracing_line_actor);

	this->QVTK->GetRenderWindow()->Render();


	nth_frame++;

    QString temp;
	temp.setNum(nth_frame);
	temp.append(".png");
	writeRendering(Renderer->GetRenderWindow(), temp.toStdString().c_str());

}
void QtTracer::surfaceRendering(bool rand_color, ImagePointer ID)
{
	//3D Surface Rendering
	if( mesh_actors.size() != 0 )
	{
		for( int i = 0; i < mesh_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(mesh_actors[i]);
		}
		mesh_actors.clear();
	}

	bool decimation = false;

	typedef itk::ConnectedComponentImageFilter< ImageType, itk::Image<short int, 3> > LabelFilterType;
	LabelFilterType::Pointer labelFilter = LabelFilterType::New();
	labelFilter->SetInput(ID);
	labelFilter->Update();

	std::vector<vtkSmartPointer<vtkPolyData> > aa = getVTKPolyDataPrecise(labelFilter->GetOutput());

	for( int i = 0; i <aa.size(); i++ )
	{

		if( decimation )
		{
			//Decimate filter (reduces the number of triangles in the mesh) for the poly data
			vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
			decimate->SetInput(aa[i]);
			decimate->SetTargetReduction(0.75);
			//printf("Decimating the contours...");
			decimate->Update();
			//printf("Done\n");

			//Smooth poly data filter for a final smoothing
			//printf("Smoothing the contours after decimation...");
			vtkSmartPointer<vtkSmoothPolyDataFilter> smoothfinal = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
			smoothfinal->SetRelaxationFactor(0.2);
			smoothfinal->SetInput(decimate->GetOutput());
			smoothfinal->SetNumberOfIterations(0);
			smoothfinal->Update();
			//printf("Done\n");

			//Return processed poly data
			aa[i] = smoothfinal->GetOutput();
		}


		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInput(aa[i]);
		mapper->ImmediateModeRenderingOff();
		mapper->Update();

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		if( rand_color )
			actor->GetProperty()->SetColor((double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10);
		else
			actor->GetProperty()->SetColor(double(72)/255,double(118)/255,1);
		actor->SetScale(general_para->getSh(),general_para->getSh(),1);
		/*if( i%6 == 0 )
		actor->GetProperty()->SetColor(1,0,0);
		else if( i%6 == 1 )
		actor->GetProperty()->SetColor(0,1,0);
		else if( i%6 == 2 )
		actor->GetProperty()->SetColor(0,0,1);
		else if( i%6 == 3 )
		actor->GetProperty()->SetColor(1,1,0);
		else if( i%6 == 4 )
		actor->GetProperty()->SetColor(1,0,1);
		else if( i%6 == 5 )
		actor->GetProperty()->SetColor(0,1,1);*/

		this->Renderer->AddActor(actor);

		mesh_actors.push_back(actor);
	}
	this->QVTK->GetRenderWindow()->Render();
}

void QtTracer::surfaceRendering(bool rand_color)
{
	//3D Surface Rendering
	if( mesh_actors.size() != 0 )
	{
		for( int i = 0; i < mesh_actors.size(); i++ )
		{
			this->Renderer->RemoveActor(mesh_actors[i]);
		}
		mesh_actors.clear();
	}

	bool decimation = false;

	typedef itk::CastImageFilter< ImageType,itk::Image<short int, 3> > CasterType;
	CasterType::Pointer caster = CasterType::New();
	caster->SetInput(IM->ISeg);
	caster->Update();

	std::vector<vtkSmartPointer<vtkPolyData> > aa = getVTKPolyDataPrecise(caster->GetOutput());

	for( int i = 0; i <aa.size(); i++ )
	{

		if( decimation )
		{
			//Decimate filter (reduces the number of triangles in the mesh) for the poly data
			vtkSmartPointer<vtkDecimatePro> decimate = vtkSmartPointer<vtkDecimatePro>::New();
			decimate->SetInput(aa[i]);
			decimate->SetTargetReduction(0.75);
			//printf("Decimating the contours...");
			decimate->Update();
			//printf("Done\n");

			//Smooth poly data filter for a final smoothing
			//printf("Smoothing the contours after decimation...");
			vtkSmartPointer<vtkSmoothPolyDataFilter> smoothfinal = vtkSmartPointer<vtkSmoothPolyDataFilter>::New();
			smoothfinal->SetRelaxationFactor(0.2);
			smoothfinal->SetInput(decimate->GetOutput());
			smoothfinal->SetNumberOfIterations(0);
			smoothfinal->Update();
			//printf("Done\n");

			//Return processed poly data
			aa[i] = smoothfinal->GetOutput();
		}


		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInput(aa[i]);
		mapper->ImmediateModeRenderingOff();
		mapper->Update();

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);
		if( rand_color )
			actor->GetProperty()->SetColor((double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10,(double)(rand()%10)/(double)10);
		else
			actor->GetProperty()->SetColor(double(72)/255,double(118)/255,1);
		actor->SetScale(general_para->getSh(),general_para->getSh(),1);
		/*if( i%6 == 0 )
		actor->GetProperty()->SetColor(1,0,0);
		else if( i%6 == 1 )
		actor->GetProperty()->SetColor(0,1,0);
		else if( i%6 == 2 )
		actor->GetProperty()->SetColor(0,0,1);
		else if( i%6 == 3 )
		actor->GetProperty()->SetColor(1,1,0);
		else if( i%6 == 4 )
		actor->GetProperty()->SetColor(1,0,1);
		else if( i%6 == 5 )
		actor->GetProperty()->SetColor(0,1,1);*/

		this->Renderer->AddActor(actor);

		mesh_actors.push_back(actor);
	}
	this->QVTK->GetRenderWindow()->Render();
}


void QtTracer::writeRendering()
{

	QString save_file = QFileDialog::getSaveFileName(this, "Save Rendering Window to Image", ".",
		tr("Image (*.tif )"));
	this->statusBar()->showMessage(save_file);

	if( !save_file.isEmpty() )
	{
		vtkWindowToImageFilter *filter = vtkWindowToImageFilter::New();
		filter->SetInput(this->Renderer->GetRenderWindow());
		vtkPNGWriter  *jw = vtkPNGWriter ::New();
		jw->SetInput(filter->GetOutput());
		jw->SetFileName(save_file.toStdString().c_str());
		jw->Write();
		jw->Delete();
		filter->Delete(); 
	}
}

void QtTracer::writeRendering(vtkRenderWindow *rw, const char *filename)
{
	vtkWindowToImageFilter *filter = vtkWindowToImageFilter::New();
	filter->SetInput(rw);
	vtkPNGWriter  *jw = vtkPNGWriter ::New();
	jw->SetInput(filter->GetOutput());
	jw->SetFileName(filename);
	jw->Write();
	jw->Delete();
	filter->Delete();
}

void QtTracer::writeRenderingAnimation()
{
	QString dir_path = QFileDialog::getExistingDirectory(this, tr("Open Image Directory"),
			"/home",
			QFileDialog::ShowDirsOnly
			| QFileDialog::DontResolveSymlinks);

	int j = 0;
	for( int i = 0; i < 72; i += 1 )
	{
	this->Camera->Azimuth(5);
	j++;
	QString temp;
	temp.setNum(j);
	temp.prepend(tr("/"));
	temp.prepend(dir_path);
	temp.append(".png");
	writeRendering(Renderer->GetRenderWindow(), temp.toStdString().c_str());
	}
}

void QtTracer::about()
{
	QMessageBox::about(this, tr("About Open-Snake Tracing System"),
		tr("<p>Rensselear Polytechnic Institute</p>"
		"<p>Open-Snake Tracing System "
		"is a part of the FARSIGHT open-source toolkit."
		"Website: <a href=\"http://www.farsight-toolkit.org\">http://www.farsight-toolkit.org</a></p>"
		"<p>Algorithms and Software Developer: <a href =\"mailto:wangy15@rpi.edu\"> Yu Wang</a>"
		"<p>Version: 0.2.1"));
}

void QtTracer::exit()
{
	QApplication::exit();
	QApplication::quit();
	::exit(0);
}


QImage convertITK2QT(ImagePointer2D I, bool mono)
{
	int X = I->GetLargestPossibleRegion().GetSize()[0]; 
	int Y = I->GetLargestPossibleRegion().GetSize()[1];
	QImage IQ;

	if(mono)
		IQ = QImage(X, Y, QImage::Format_MonoLSB);
	else
		IQ = QImage(X,Y, QImage::Format_ARGB32);
	typedef itk::ImageRegionIteratorWithIndex<ImageType2D> IteratorType;
	IteratorType It(I, I->GetRequestedRegion());
	for(It.GoToBegin(); !It.IsAtEnd(); ++It)
	{
		int temp_x = It.GetIndex()[0];
		int temp_y = It.GetIndex()[1];

		if(mono)
			IQ.setPixel(temp_x, temp_y, It.Get());
		else
		{
			QRgb value;
			value = qRgb(It.Get(), It.Get(), It.Get()); 
			IQ.setPixel(temp_x, temp_y, value);
		}
	}
	return IQ;
}

QImage convertITK2QT(ProbImagePointer2D I, bool mono)
{
	int X = I->GetLargestPossibleRegion().GetSize()[0]; 
	int Y = I->GetLargestPossibleRegion().GetSize()[1];
	QImage IQ;

	if(mono)
		IQ = QImage(X, Y, QImage::Format_MonoLSB);
	else
		IQ = QImage(X,Y, QImage::Format_ARGB32);
	typedef itk::ImageRegionIteratorWithIndex<ProbImageType2D> IteratorType;
	IteratorType It(I, I->GetRequestedRegion());
	for(It.GoToBegin(); !It.IsAtEnd(); ++It)
	{
		int temp_x = It.GetIndex()[0];
		int temp_y = It.GetIndex()[1];

		if(mono)
			IQ.setPixel(temp_x, temp_y, It.Get());
		else
		{
			QRgb value;
			value = qRgb(It.Get(), It.Get(), It.Get()); 
			IQ.setPixel(temp_x, temp_y, value);
		}
	}
	return IQ;
}

QImage convertITK2QT(LabelImagePointer2D I, bool mono)
{
	int X = I->GetLargestPossibleRegion().GetSize()[0]; 
	int Y = I->GetLargestPossibleRegion().GetSize()[1];
	QImage IQ;

	if(mono)
		IQ = QImage(X, Y, QImage::Format_MonoLSB);
	else
		IQ = QImage(X,Y, QImage::Format_ARGB32);
	typedef itk::ImageRegionIteratorWithIndex<LabelImageType2D> IteratorType;
	IteratorType It(I, I->GetRequestedRegion());
	for(It.GoToBegin(); !It.IsAtEnd(); ++It) 
	{
		int temp_x = It.GetIndex()[0];
		int temp_y = It.GetIndex()[1];

		if(mono)
			IQ.setPixel(temp_x, temp_y, It.Get());
		else
		{
			QRgb value;
			value = qRgb(It.Get(), It.Get(), It.Get()); 
			IQ.setPixel(temp_x, temp_y, value);
		}
	}
	return IQ;
}

QImage convertITK2QT(RGBImagePointer2D I)
{
	int X = I->GetLargestPossibleRegion().GetSize()[0]; 
	int Y = I->GetLargestPossibleRegion().GetSize()[1];
	QImage IQ;


	IQ = QImage(X,Y, QImage::Format_ARGB32);
	typedef itk::ImageRegionIteratorWithIndex<RGBImageType2D> IteratorType;
	IteratorType It(I, I->GetRequestedRegion());
	for(It.GoToBegin(); !It.IsAtEnd(); ++It) 
	{
		int temp_x = It.GetIndex()[0];
		int temp_y = It.GetIndex()[1];

		QRgb value;
		value = qRgb(It.Get()[0], It.Get()[1], It.Get()[2]); 
		IQ.setPixel(temp_x, temp_y, value);
	}
	return IQ;
}

