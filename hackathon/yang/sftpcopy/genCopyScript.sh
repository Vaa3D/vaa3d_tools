# 8/30/2018 YY
# generate a script transfer data with headless sftp
# sh genCopyScript.sh filenametodownload scriptname username password pathtofoldercontainsfile

FILE=$1
OUTSCRIPT=$2
USRNAME=$3
IPADDRESS=$4
PASSWORD=$5
DIR=$6

echo "#!/usr/bin/expect" >> $OUTSCRIPT
echo "\n" >> $OUTSCRIPT
echo "set timeout -1" >> $OUTSCRIPT
echo "spawn sftp -oPort=22 ${USRNAME}@${IPADDRESS}" >> $OUTSCRIPT
echo "expect \"password:\" { send \"${PASSWORD}\\\n\" } " >> $OUTSCRIPT
echo "expect \"sftp>\" { send \"cd ${DIR}\\\n\" } " >> $OUTSCRIPT
echo "expect \"sftp>\" { send \"reget ${FILE}\\\n\" } " >> $OUTSCRIPT
echo "expect \"sftp>\" { send \"bye\\\n\" } " >> $OUTSCRIPT

