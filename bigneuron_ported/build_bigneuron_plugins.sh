#!/bin/bash
# batch build all bigneuron related plugins
# by Zhi Zhou

svn up

for mydir in $(ls -d */); do
  echo 
  echo $mydir
  echo ===============================  
  cd $mydir
  if [ -f *.pro ]; then
  for mypro in $( ls *.pro ); do
  	qmake $mypro 
  	make 
  done;
  else
  for mysubdir in $(ls -d */); do
	echo
	echo $mysubdir
	cd $mysubdir
  	if [ -f *.pro ]; then
  	for mysubpro in $( ls *.pro ); do
  		qmake $mysubpro 
  		make 
	done;
	fi
	cd ..
  done;
  fi
  cd ..
done  

cd ..

