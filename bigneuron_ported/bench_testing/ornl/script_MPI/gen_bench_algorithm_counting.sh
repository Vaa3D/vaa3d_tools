#!/bin/bash
#
#This is a shell program to batch count swc files based on different tracing algorithms
#
			
tarfile=$1
METHOD=$2

#APP1

  if [ $METHOD == "1" ]; then
	tar ztvf $tarfile| grep _app1.swc|wc -l 
  fi;

#APP2

  if [ $METHOD == "2" ]; then
	tar ztvf $tarfile| grep _app2.swc|wc -l
  fi;

#MOST

  if [ $METHOD == "3" ]; then
	tar ztvf $tarfile| grep _MOST.swc|wc -l
  fi;

#NEUTUBE

  if [ $METHOD == "4" ]; then
	tar ztvf $tarfile| grep _neutube.swc|wc -l
  fi;

#FARSIGHT Snake

  if [ $METHOD == "5" ]; then
	tar ztvf $tarfile| grep _snake.swc|wc -l
  fi;

#3 from SimpleTracing

  if [ $METHOD == "6" ]; then
		tar ztvf $tarfile| grep _simple.swc|wc -l
  fi;

  if [ $METHOD == "7" ]; then

    tar ztvf $tarfile| grep _Rayshooting.swc|wc -l
  fi;

  if [ $METHOD == "8" ]; then
    tar ztvf $tarfile| grep _Rollerball.swc|wc -l
  fi;

#TreMap

  if [ $METHOD == "9" ]; then
    tar ztvf $tarfile| grep _TreMap.swc|wc -l
  fi;

#MST

  if [ $METHOD == "10" ]; then
    tar ztvf $tarfile| grep _MST_Tracing.swc|wc -l
  fi;

#NeuroGPSTree

  if [ $METHOD == "11" ]; then
    tar ztvf $tarfile| grep _NeuroGPSTree.swc|wc -l
  fi;

#fastmarching_spanningtree

  if [ $METHOD == "12" ]; then
    tar ztvf $tarfile| grep _fastmarching_spanningtree.swc|wc -l
  fi;

#meanshift

  if [ $METHOD == "13" ]; then
    tar ztvf $tarfile| grep _meanshift.swc|wc -l
 $outputScript;
  fi;

#CWlab_method1_version1

  if [ $METHOD == "14" ]; then
    tar ztvf $tarfile| grep _Cwlab_ver1.swc|wc -l
  fi;

#LCM_boost

  if [ $METHOD == "15" ]; then
    tar ztvf $tarfile| grep _LCMboost.swc|wc -l
  fi;

#NeuroStalker

  if [ $METHOD == "16" ]; then
    tar ztvf $tarfile| grep _NeuroStalker.swc|wc -l
  fi;

#nctuTW

  if [ $METHOD == "17" ]; then
    tar ztvf $tarfile| grep _nctuTW.swc|wc -l
  fi;

#tips_GD

  if [ $METHOD == "18" ]; then
    tar ztvf $tarfile| grep _nctuTW_GD.swc|wc -l
  fi;

#SimpleAxisAnalyzer

  if [ $METHOD == "19" ]; then
    tar ztvf $tarfile| grep _axis_analyzer.swc|wc -l
  fi;

#NeuronChaser

  if [ $METHOD == "20" ]; then
    tar ztvf $tarfile| grep _NeuronChaser.swc|wc -l
  fi;

#smartTracing

  if [ $METHOD == "21" ]; then
    tar ztvf $tarfile| grep _smartTracing.swc|wc -l
  fi;

#neutu_autotrace

  if [ $METHOD == "22" ]; then
    tar ztvf $tarfile| grep _neutu_autotrace.swc|wc -l
  fi;

#Advantra

  if [ $METHOD == "23" ]; then
    tar ztvf $tarfile| grep _Advantra.swc|wc -l
  fi;

#RegMST

  if [ $METHOD == "24" ]; then
    tar ztvf $tarfile| grep _tubularity_model_S.v3draw_MST_Tracing_Ws_21_th_170.swc|wc -l
  fi;

#EnsembleNeuronTracer

  if [ $METHOD == "25" ]; then
    tar ztvf $tarfile| grep _EnsembleNeuronTracerBasic.swc|wc -l
  fi;

#EnsembleNeuronTracerV2n

  if [ $METHOD == "26" ]; then
    tar ztvf $tarfile| grep _EnsembleNeuronTracerV2n.swc|wc -l
  fi;

#EnsembleNeuronTracerV2s

  if [ $METHOD == "27" ]; then
    tar ztvf $tarfile| grep _EnsembleNeuronTracerV2s.swc|wc -l
  fi;

#3DTraceSWC

  if [ $METHOD == "28" ]; then
    tar ztvf $tarfile| grep _pyzh.swc|wc -l
  fi;

#Rivulet

  if [ $METHOD == "29" ]; then
    tar ztvf $tarfile| grep _Rivulet.swc|wc -l
  fi;

#ENT

  if [ $METHOD == "30" ]; then
    tar ztvf $tarfile| grep _ENT.swc|wc -l
  fi;


#APP2_new1

  if [ $METHOD == "31" ]; then
    tar ztvf $tarfile| grep _app2new1.swc|wc -l
  fi;

#APP2_new2

  if [ $METHOD == "32" ]; then
    tar ztvf $tarfile| grep _app2new2.swc|wc -l
  fi;

#APP2_new3

  if [ $METHOD == "33" ]; then
    tar ztvf $tarfile| grep _app2new3.swc|wc -l
  fi;

#LCM_boost_2

  if [ $METHOD == "34" ]; then
    tar ztvf $tarfile| grep _LCMboost_2.swc|wc -l
  fi;

