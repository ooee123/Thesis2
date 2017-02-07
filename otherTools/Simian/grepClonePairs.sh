#!/bin/bash

cat $@ | grep "Between lines [[:digit:]]* and [[:digit:]]* in " | sed 's/ Between lines \([[:digit:]]*\) and \([[:digit:]]*\) in \(.*\)/\1,\2,\3/'
