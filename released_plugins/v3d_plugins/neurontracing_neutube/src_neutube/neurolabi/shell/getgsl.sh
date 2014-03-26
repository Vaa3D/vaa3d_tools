#!/bin/sh
if test $# -eq 0
then
  local_dir=`pwd`
else
  local_dir=$1
fi

package=gsl-1.12.tar.gz
ftp_server=ftp.gnu.org
remote_dir=gnu/gsl

ftp -n -v $ftp_server <<EOT
user anonymous anonymous
epsv
get $remote_dir/$package $local_dir/$package
