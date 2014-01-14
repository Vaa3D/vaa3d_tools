#!/bin/sh

for var in "$@"
do
    echo "$var"
    fn="$var";
    case "$var" in
    	*Py.swc) 
    		echo "Already pyramidal swc" 
    		;;
    	*py.swc)
    		echo "Already pyramidal swc"
    		;;
    	*)
    		filename="${var%.*}"
    		./edswc "$var" -pyramidal -o "$filename"Py.swc
    		fn="$filename"Py.swc
    		;;
    esac
    #echo "$fn"
    #echo "$var"
    #echo "   "
    stackfn="";
    imagefolder="/Users/feng/Documents/image/mCA3_CA1_raw/"
    case "$var" in
        0515_15*)
            stackfn="$imagefolder"0515_15.raw
            ;;
        020910_02*)
            stackfn="$imagefolder"020910_02LR_4to30.raw
            ;;
        020910_03a*)
            stackfn="$imagefolder"020910_03a_1to16.raw
            ;;
        020910_04_1to29*)
            stackfn="$imagefolder"020910_04_1to29.raw
            ;;
        020910_04_3to31*)
            stackfn="$imagefolder"020910_04_3to31.raw
            ;;
        020910_B09*)
            stackfn="$imagefolder"020910_B09.raw
            ;;
        020910c_02*)
            stackfn="$imagefolder"020910c_02_6to31.raw
            ;;
        020910C_07_1to41*)
            stackfn="$imagefolder"020910_07_1to41.raw
            ;;
        020910c_07_2*)
            stackfn="$imagefolder"020910c_07_2.raw
            ;;
        020910c_07a*)
            stackfn="$imagefolder"020910c_07a.raw
            ;;
        081210_A13*)
            stackfn="$imagefolder"081210_A13_1to47.raw
            ;;
        081210_A14_16to54*)
            stackfn="$imagefolder"081210_A14_16to54.raw
            ;;
        081210_A15_8to56*)
            stackfn="$imagefolder"081210_A15_8to56.raw
            ;;
        *)
            echo "something wrong"
            exit 1
            ;;
    esac
    ./edswc $fn -signal $stackfn -bluechannel 3 -o "$fn"_blue.swc
    if [ -e "$var"_puncta.apo_big.apo ]; then
        echo ./edswc "$fn" -apo "$var"_puncta.apo_big.apo -analysis -signal "$stackfn" -bluechannel 3 -stack "$stackfn" -neuronchannel 2
        echo "    "
    	./edswc $fn -apo "$var"_puncta.apo_big.apo -analysis -signal $stackfn -bluechannel 3 -stack $stackfn -neuronchannel 2
    fi
    if [ -e "$var"_puncta.apo_small.apo ]; then
        echo ./edswc "$fn" -apo "$var"_puncta.apo_small.apo -analysis -signal "$stackfn" -bluechannel 3 -stack "$stackfn" -neuronchannel 2
        echo "    "
    	./edswc $fn -apo "$var"_puncta.apo_small.apo -analysis -signal $stackfn -bluechannel 3 -stack $stackfn -neuronchannel 2
    fi
    if [ -e "$var"_puncta.apo ]; then
        echo ./edswc "$fn" -apo "$var"_puncta.apo -analysis -signal "$stackfn" -bluechannel 3 -stack "$stackfn" -neuronchannel 2
        echo "    "
    	./edswc $fn -apo "$var"_puncta.apo -analysis -signal $stackfn -bluechannel 3 -stack $stackfn -neuronchannel 2
    fi
done
