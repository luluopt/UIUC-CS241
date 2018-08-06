#!/bin/bash

# this shell script runs both testers, but only if make reports that nothing
# changed since the last passing run of the tester

# config vars
tfile=.testsran

# first, run make and check for changes
make 2>&1 | tee /tmp/makeout
if [ $? -ne 0 ] ; then
    echo "your code doesn't compile!"
    exit 1
fi

unchanged=$(cat /tmp/makeout | grep "Nothing to be done for" | wc -l)

# if nothing changed and the tests have completed successfully and nothing has
# changed since then, don't rerun the tests
if [ 1 -eq $unchanged ] && [ -f $tfile ]; then
    echo "no new changes! not rerunning the tests"
    exit 0
fi

# we got here, so we are going to do a new test run
rm -f $tfile

# run part1, saving it's output in a temporary file
./part1 > /tmp/part1out

# compare the two
diff /tmp/part1out part1-expected-output

# store diff's exit status
p1ret=$?

if [ $p1ret -ne 0 ] ; then
    echo "your part1 output and part1-expected-output were different!"
else
    echo "part1 passes!"
fi

# run part2, saving it's output in a temporary file
./part2 > /tmp/part2out

# compare the two
diff /tmp/part2out part2-expected-output

# store diff's exit status
p2ret=$?

if [ $p2ret -ne 0 ] ; then
    echo "your part2 output and part2-expected-output were different!"
else
    echo "part2 passes!"
fi

# run part3, saving it's output in a temporary file
./part3 part3-test-file> /tmp/part3out

# compare the two
diff /tmp/part3out part3-expected-output

# store diff's exit status
p3ret=$?

if [ $p3ret -ne 0 ] ; then
    echo "your part3 output and part3-expected-output were different!"
else
    echo "part3 passes!"
fi

if [ 0 -eq $p1ret ] && [ 0 -eq $p2ret ] && [ 0 -eq $p3ret ]; then
    # everything completed successfully
    touch $tfile
    exit 0
else
    # just in case
    rm -f $tfile
    exit 1
fi
