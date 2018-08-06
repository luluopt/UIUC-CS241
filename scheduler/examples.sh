#!/bin/bash

# Scheduler Lab
# CS 241 - Spring 2017

for file in `ls examples/`; do
    if [[ $file =~ ^proc([[:digit:]]+)-([[:alnum:]]+).out$ ]]; then
        # Only compare the last six lines for correctness. The last six lines
        # contain the the final timing diagram as well as the average waiting,
        # turnaround, and response time.
        ./simulator -s ${BASH_REMATCH[2]} examples/proc${BASH_REMATCH[1]}.in \
            | tail -n 6 > output1;
        tail -n 6 examples/$file > output2;
        
        if diff -q output1 output2 > /dev/null; then
            : # do nothing
        else
            echo "Failed examples/$file"
        fi
    fi
done

# cleanup
rm output1 output2;

