#!/bin/sh
if test $# -eq 0
then
    local_dir=`pwd`
else
    local_dir=$1
fi

package=fftw-3.2.1.tar.gz
ftp_server=ftp.fftw.org
remote_dir=pub/fftw

ftp -n -v $ftp_server <<EOT
user anonymous anonymous
epsv
get $remote_dir/$package $local_dir/$package