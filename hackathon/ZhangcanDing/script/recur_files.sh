printfile()
{
	srcdir=$1
	destdir=$2
	for file in "$srcdir"/*
	do
		if [ -d $file ]
		then
			printfile $file $destdir
		else
			cp $file $destdir
		fi
	done
}

printfile $1 $2


