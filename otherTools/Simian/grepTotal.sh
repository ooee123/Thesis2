#!/bin/bash
cat $@ | grep "Found [[:digit:]]* duplicate lines in [[:digit:]]* blocks in [[:digit:]]* files" | sed 's/Found \([[:digit:]]*\) duplicate lines in \([[:digit:]]*\) blocks in [[:digit:]]* files/\1,\2/'
