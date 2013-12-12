#!/bin/sh
if test $# -eq 0
then
    local_dir=`pwd`
else
    local_dir=$1
fi

package=zlib-1.2.3.tar.gz
http_server=www.zlib.net

current_dir=`pwd`

if test `which wget`
then
    cd $local_dir
    wget $http_server/$package
    cd $current_dir
else
    if test `which curl`
    then
	cd $local_dir
	curl -c - -O $http_server/$package
	cd $current_dir
    else
	echo 'Failed to get zlib: No download tool found.'
	echo "Please use a browser to download $http_server/$package."
    fi
fi