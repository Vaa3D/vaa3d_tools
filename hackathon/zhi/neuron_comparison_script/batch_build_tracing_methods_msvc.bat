::!/bin/bash
::THis is a shell progrem to batch build tracing methods plugins using a VC Studio complier's nnmake -f Makefile.Release function, including: 
::1.	All-path-pruning 1 (bottom-up pruning) @
::2.	All-path-pruning 2 (top-down pruning)  @ 
::3.	MOST  @
::4.	NeuTube
::5.	FarSight-Snake   @
::6.	SimpleTracing-dt  @
::7.	Simple-RayCasting @ 
::8.	Simple-RollingBall @
::9.	TreMap   @
::10.	MST-Tracing  @
::11.	NeuronGPSTree(HUST)   @
::12.	fastmarching_spanningtree(BJUT)  @
::13.	MeanshiftSpanningtree(BJUT)  @
::14.	CWlab_method1_version1(NTUST)  @
::15.	LCM_boost (A-STAR)   @ 
::16.	NeuroStalker (Sydney)  @
::17.	nctuTW(NCTUTW)  @
::18.	GD_nctuTW (ALLEN + NCTUTW)  @
::19.	SimpleAxisAnalyzer (NIU)  @
::20.	NeuronChaser(Erasmus)   @
::21.	smartTrace (ALLEN) @
::22.	neutu_autotrace (Janelia + PSU)  @
::23.	Advantra (Erasmus) @
::24.	RegMST (EPFL) @
::25.	EnsembleNeuronTracerBasic(NTUST)
::26.	EnsembleNeuronTracerV2n(NTUST)
::27.	EnsembleNeuronTracerV2s(NTUST)

::APP1 and APP2 
cd ../../../released_plugins/v3d_plugins/neurontracing_vn2/
nmake clean
svn up
qmake
nmake -f Makefile.Release

::MOST 
cd ../neurontracing_MOST/
nmake clean
svn up
qmake
nmake -f Makefile.Release
	

::SimpleTracing-dt,  Simple-RollingBall, and Simple-RayCasting
cd ../neurontracing_simpletracing/
nmake clean
svn up
qmake
nmake -f Makefile.Release

::smartTrace
::TreMap 
::MST 
::snake
::NeuroGPSTree

::fastmarching_spanningtree 
cd ../../../bigneuron_ported/zhijiang_zn_bjut/fastmarching_spanningtree_vaa3d/
nmake clean
svn up
qmake
nmake -f Makefile.Release

::MeanshiftSpanningtree 
cd ../../../bigneuron_ported/zhijiang_zn_bjut/MeanshiftSpanningtree/
nmake clean
svn up
qmake
nmake -f Makefile.Release

::CWlab_method1_version1 
::LCM_boost 
::NeuroStalker 
::nctuTW 
::GD_nctuTW 
::SimpleAxisAnalyzer 
::NeuronChaser
::Advantra
::neutu_autotrace
::RegMST
::EnsembleNeuronTracerBasic
::EnsembleNeuronTracerV2n
::EnsembleNeuronTracerV2s
::neuTube 





