#!/bin/bash
cd /data/mat/zhi/BigNeuron/first2000/images/
for filename in `ls -d *`
do
	mkdir "/data/mat/zhi/BigNeuron/first2000/reconstructions2/"$filename
	mkdir "/data/mat/zhi/BigNeuron/first2000/images2/"$filename
	mv $filename"/"$filename "/data/mat/zhi/BigNeuron/first2000/images2/"$filename"/"
	  		
done

