#!/bin/bash

# This is a script help you run all the test cases.
# All the test cases are stored in test_array.
# If you want to reorder the test cases, just reorder the test_array
# in the order you like.  
# 
# There are two ways to skip test cases:
# 1. Specify the test cases you want to skip in skip_arr
# 2. Run the script with -s flag
#   ex. ./run_all_mcontest.sh -s 1 2 3 4 5
#  this will skip test-1 through test-5

# What test cases do you want?
test_array=(1 2 3 4 5 6 7 8 9 10 11 12 13)
# What do you want to skip?
skip_arr=()

# Functions 
usage () { 
  echo "Usage: ./run_all_mcontest.sh [-s <1-13>]" 1>&2; exit 1; 
}

array_contains () {
    local arr=$1[@]
    local seeking=$2

    for each_elem in "${!arr}"
    do
      if [ "$each_elem" == "${seeking}" ] ; then
          return 0
      fi
    done
    return 1
}

# First, parse the arguments
skip_flag=0

for var in "$@"
do
  if [ "$var" = "-s" ] || [ "$var" = "-skip" ]
  then
    skip_flag=1
  else
    if [ $skip_flag != 1 ]
    then
      continue
    else
      if [ $var -gt 13 ] || [ $var -lt 1 ]
      then
        usage
      else
        skip_arr+=($var)
      fi
    fi
  fi
done


# Start running tests
# exit if anything fails
set -e

make clean
make -f Makefile


# don't exit if any of the testers fail
set +e

#for i in `seq 1 13`;
for i in "${test_array[@]}"
do
  if array_contains skip_arr "$i"; then
    echo "Skip test-$i"
  else 
    echo "Test $i running"
   ./mcontest testers_exe/tester-$i
    echo "Test $i completed."
  fi
done

echo "All test completed."
