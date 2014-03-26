#!/bin/sh
# script for execution of deployed applications
#
# Sets up the MCR environment for the current $ARCH and executes 
# the specified command.
#
exe_name=$0
exe_dir=`dirname "$0"`
echo "------------------------------------------"
if [ "x$1" = "x" ]; then
  echo Usage:
  echo  $0  '<result_file> <param_file> <jsonoutputfile>'
else
  declare -a arg_spec=(--resultfile --param_file --jsonoutputfile)
  args=
  while [ $# -gt 0 ]; do
      token=`echo "$1" | sed 's/ /\\\\ /g'`   # Add blackslash before each blank
      args="${args} ${arg_spec[0]} ${token}" 
      unset -v 'arg_spec[0]'
      arg_spec=(${arg_spec[@]})
      shift
  done
  echo "${exe_dir}/Tbar_apply_sa $args"
fi
exit

