#!/bin/bash
mkdir mine
mkdir mine_noSort
mkdir mine_noID
mkdir mine_nothing
mkdir original
for C in *.c; do
   /home/ooee/Thesis2/run.sh $C > mine/$C
   /home/ooee/Thesis2/run.sh -ci $C > mine_noSort/$C
   /home/ooee/Thesis2/run.sh -csr $C > mine_noID/$C
   /home/ooee/Thesis2/run.sh -c $C > mine_nothing/$C
   cp $C original/$C
done
