#!/bin/bash

INDENT="indent -linux -l9999 --braces-on-func-def-line"
pwd=$(pwd)

if [ -d "mine" ]; then
   rm -rf mine
fi
if [ -d "mine_noSort" ]; then
   rm -rf mine_noSort
fi
if [ -d "mine_noID" ]; then
   rm -rf mine_noID
fi
if [ -d "mine_nothing" ]; then
   rm -rf mine_nothing
fi
if [ -d "original" ]; then
   rm -rf original
fi
if [ -d "mine_moss" ]; then
   rm -rf mine_moss 
fi
if [ -d "mine_nothing_moss" ]; then
   rm -rf mine_nothing_moss 
fi
if [ -d "mine_noID_moss" ]; then
   rm -rf mine_noID_moss 
fi
mkdir mine
mkdir mine_moss
mkdir mine_noSort
mkdir mine_noID
mkdir mine_nothing
mkdir mine_nothing_moss
mkdir mine_noID_moss
mkdir original
for C in *.c; do
   /home/ooee/Thesis2/run.sh -isrm mine_moss $C > mine/$C
   if [ ! -s mine/$C ]; then
      echo "$PWD mine $C" >> /home/ooee/Thesis2/studentFiles/errors
   fi
   $INDENT mine/$C
   rm mine/$C~
   cd mine_moss
   for D in *.c; do
      $INDENT $D
      rm $D~
   done
   cd ..
   /home/ooee/Thesis2/run.sh -i $C > mine_noSort/$C
   if [ ! -s mine_noSort/$C ]; then
      echo "$PWD mine_noSort $C" >> /home/ooee/Thesis2/studentFiles/errors
   fi
   $INDENT mine_noSort/$C
   rm mine_noSort/$C~
   /home/ooee/Thesis2/run.sh -srm mine_noID_moss $C > mine_noID/$C
   if [ ! -s mine_noID/$C ]; then
      echo "$PWD mine_noID $C" >> /home/ooee/Thesis2/studentFiles/errors
   fi
   $INDENT mine_noID/$C
   rm mine_noID/$C~
   cd mine_noID_moss
   for D in *.c; do
      $INDENT $D
      rm $D~
   done
   cd ..
   /home/ooee/Thesis2/run.sh -m mine_nothing_moss $C > mine_nothing/$C
   if [ ! -s mine_nothing/$C ]; then
      echo "$PWD mine_nothing $C" >> /home/ooee/Thesis2/studentFiles/errors
   fi
   $INDENT mine_nothing/$C
   rm mine_nothing/$C~
   cd mine_nothing_moss
   for D in *.c; do
      $INDENT $D
      rm $D~
   done
   cd ..
   gcc -fpreprocessed -dD -E $C | $INDENT | sed '/^$/d' | sed '/^#/d' > original/$C
   #cp $C original/$C
done
