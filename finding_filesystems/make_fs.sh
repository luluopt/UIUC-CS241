#!/bin/bash
trap "exit" INT

# Invalid number of arguments
if [ "$#" -ne 2 ]; then
  echo "Usage ./make_fs.sh [source dir] [output filename]"
  exit
fi
# Checking for valid directory
if [ ! -d "$1" ]; then
  echo "$1 is not a directory!"
  exit
fi
# Cleaning up old fs if it exists
if [ -a $2 ]; then
  rm -f $2
fi

# Initialize filesystem
./minixfs-creator mkfs $2

for r in $(find "$1" | grep '/' | cut -d "/" -f 2-)
do
  if [ -d $1/$r ]; then
    # Copy over directories
    echo Making directory /$r
    ./minixfs-creator $2 mkdir /$r
  fi
  if [ -f $1/$r ]; then
    # Copying over files
    echo Copying file $r
    ./minixfs-creator $2 cp $1/$r /$r
  fi
done
