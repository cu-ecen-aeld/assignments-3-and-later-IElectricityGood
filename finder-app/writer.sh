#!/bin/bash

if [ $# -ne 2 ] # number of arguments != 2
then
    echo "Please provide both arguments <writefile> and <writestr> to the script."
    echo -e "\nUsage:\n./writer.sh <writefile> <writestr>"
    echo -e "\t<writefile> = full path to a file on filesystem"
    echo -e "\t<writestr> = text string which will be written within this file"
    exit 1
fi

writefile=$1
writestr=$2

echo $writestr > $writefile 2>&1
if [[ $? != 0 ]] # creating this empty file failed
then
    echo "Could not create file $writefile!"
    exit 1
fi
