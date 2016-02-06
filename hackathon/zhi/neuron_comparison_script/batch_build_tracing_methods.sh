#!/bin/bash
#THis is a shell progrem to batch build tracing methods plugins, including: 
#1.	All-path-pruning 1 (bottom-up pruning) @
#2.	All-path-pruning 2 (top-down pruning)  @ 
#3.	MOST  @
#4.	NeuTube
#5.	FarSight-Snake   @
#6.	SimpleTracing-dt  @
#7.	Simple-RayCasting @ 
#8.	Simple-RollingBall @
#9.	TreMap   @
#10.	MST-Tracing  @
#11.	NeuronGPSTree(HUST)   @
#12.	fastmarching_spanningtree(BJUT)  @
#13.	MeanshiftSpanningtree(BJUT)  @
#14.	CWlab_method1_version1(NTUST)  @
#15.	LCM_boost (A-STAR)   @ 
#16.	NeuroStalker (Sydney)  @
#17.	nctuTW(NCTUTW)  @
#18.	GD_nctuTW (ALLEN + NCTUTW)  @
#19.	SimpleAxisAnalyzer (NIU)  @
#20.	NeuronChaser(Erasmus)   @
#21.	smartTrace (ALLEN) @
#22.	neutu_autotrace (Janelia + PSU)  @
#23.	Advantra (Erasmus) @
#24.	RegMST (EPFL) @
#25.	EnsembleNeuronTracerBasic(NTUST)
#26.	EnsembleNeuronTracerV2n(NTUST)
#27.	EnsembleNeuronTracerV2s(NTUST)
#28.	3DTraceSWC(Brandeis)
#29.	Rivulet(Sydney)
#30.	ENT(NTUST)
#31.	PSF (Erhan)

#APP1 and APP2
cd ../../../released_plugins/v3d_plugins/neurontracing_vn2/
qmake
make

#MOST 
cd ../neurontracing_MOST/
qmake
make	

#SimpleTracing-dt,  Simple-RollingBall, and Simple-RayCasting
cd ../neurontracing_simpletracing/
qmake
make

#smartTrace
cd ../neurontracing_smarttracing/
qmake
make

#TreMap 
cd  ../../../hackathon/zhi/neurontracing_mip/
qmake
make

#MST 
cd  ../../../hackathon/zhi/neurontracing_mst/
qmake
make

#snake
cd ../../../hackathon/zhi/snake_tracing/
if [ ! -d ITK_include ]; then
	unzip ITK_include.zip		
fi	
qmake
make

#NeuroGPSTree
cd ../../../bigneuron_ported/zhouhang_hust/NeuroGPSTree/
if [ ! -d eigen3.2.2 ]; then
	unzip eigen3.2.2	
fi	
qmake
make

#fastmarching_spanningtree 
cd ../../../bigneuron_ported/zhijiang_zn_bjut/fastmarching_spanningtree_vaa3d/
qmake
make

#MeanshiftSpanningtree 
cd ../../../bigneuron_ported/zhijiang_zn_bjut/MeanshiftSpanningtree/
qmake
make

#CWlab_method1_version1 
cd ../../../bigneuron_ported/chingwei/CWlab_method1_version1/
qmake
make

#LCM_boost 
cd ../../../bigneuron_ported/LinGU/LCM_boost/
qmake
make

#NeuroStalker 
cd ../../../bigneuron_ported/siqi/stalker_v3d/
qmake
make

#nctuTW 
cd ../../../bigneuron_ported/ytc/nctuTW/
qmake
make

#GD_nctuTW 
cd ../../../bigneuron_ported/ytc/nctuTW_tips/
qmake
make

#SimpleAxisAnalyzer 
cd ../../../bigneuron_ported/jiezhou/recon_3d_thin_v1/
qmake
make

#NeuronChaser
cd ../../../bigneuron_ported/miroslav_r/NeuronChaser/
qmake
make

#Advantra
cd ../../../bigneuron_ported/miroslav_r/Advantra/
qmake
make

#neutu_autotrace
cd ../../../bigneuron_ported/ting_dezhe/neutu_autotrace/
qmake
make

#RegMST
cd ../../../bigneuron_ported/AmosSironi_PrzemyslawGlowacki/RegMST_tracing_plugin/
qmake
make

cd ../../../bigneuron_ported/AmosSironi_PrzemyslawGlowacki/SQBTree_plugin/
qmake
make

#EnsembleNeuronTracerBasic
cd ../../../bigneuron_ported/chingwei/EnsembleNeuronTracerBasic/
qmake
make

#EnsembleNeuronTracerV2n
cd ../../../bigneuron_ported/chingwei/EnsembleNeuronTracerV2n/
qmake
make

#EnsembleNeuronTracerV2s
cd ../../../bigneuron_ported/chingwei/EnsembleNeuronTracerV2s/
qmake
make

#ENT
cd ../../../bigneuron_ported/chingwei/ENT/
qmake
make

#Rivulet
cd ../../../bigneuron_ported/siqi/rivuletv3d/
qmake
make

#3DTraceSWC
cd ../../../bigneuron_ported/pengyu_zhihao/3DTraceSWC/
qmake
make

#PSF
cd ../../../hackathon/zhi/PSF_tracing/
qmake
make

#neuTube 
cd ../../../hackathon/ting_neutube/neutube_v2/
qmake
make




mv ../../../../v3d_external/bin/plugins/NeuroStalker ../../../../v3d_external/bin/plugins/neuron_tracing/
mv ../../../../v3d_external/bin/plugins/LCM_boost ../../../../v3d_external/bin/plugins/neuron_tracing/
mv ../../../../v3d_external/bin/plugins/bigneuron_ported/* ../../../../v3d_external/bin/plugins/neuron_tracing/
rm -rf ../../../../v3d_external/bin/plugins/bigneuron_ported/
mv ../../../../v3d_external/bin/plugins/bigneuronhackathon/* ../../../../v3d_external/bin/plugins/neuron_tracing/
rm -rf ../../../../v3d_external/bin/plugins/bigneuronhackathon/




