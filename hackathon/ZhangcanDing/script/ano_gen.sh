#! /bin/bash

infolder=$1;

cd $infolder

for i in `ls *.v3draw`
do
	ID=${i%.v3draw}
	touch ${ID}.ano
	echo "APOFILE=${ID}.apo">>${ID}.ano
	echo "RAWIMG=$i">>${ID}.ano
	echo "SWCFILE=${ID}_auto.swc">>${ID}.ano
	echo "SWCFILE=${ID}_manual.swc">>${ID}.ano
done
