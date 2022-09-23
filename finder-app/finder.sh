#!/bin/bash

if [ $# -ne 2 ] # number of arguments != 2
then
    echo "Please provide both arguments <filesdir> and <searchstr> to the script."
    echo -e "\nUsage:\n./finder.sh <filesdir> <searchstr>"
    echo -e "\t<filesdir> = path to directory on filesystem"
    echo -e "\t<searchstr> = text string which will be searched within these files"
    exit 1
fi

filesdir=$1

if [ ! -d $filesdir ] # not a valid directory
then
    echo "Please provide a valid directory for <filesdir>."
    echo "Provided '$filesdir' is not a valid directory"
    exit 1
fi

searchstr=$2

X=$(find $filesdir -type f | wc -l)

# target grep does not support --binary-files=without-match option
# Y=$(grep -r --binary-files=without-match $searchstr $filesdir | wc -l)
Y=$(grep -r $searchstr $filesdir | wc -l)
echo "The number of files are $X and the number of matching lines are $Y"
