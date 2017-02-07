#!/bin/bash
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
   /home/ooee/Thesis2/run.sh -cisrm mine_moss $C > mine/$C
   indent -linux --braces-on-func-def-line mine/$C
   rm mine/$C~
   cd mine_moss
   for D in *.c; do
      indent -linux --braces-on-func-def-line $D
      rm $D~
   done
   cd ..
   /home/ooee/Thesis2/run.sh -ci $C > mine_noSort/$C
   indent -linux --braces-on-func-def-line mine_noSort/$C
   rm mine_noSort/$C~
   /home/ooee/Thesis2/run.sh -csrm mine_noID_moss $C > mine_noID/$C
   indent -linux --braces-on-func-def-line mine_noID/$C
   rm mine_noID/$C~
   cd mine_noID_moss
   for D in *.c; do
      indent -linux --braces-on-func-def-line $D
      rm $D~
   done
   cd ..
   /home/ooee/Thesis2/run.sh -cm mine_nothing_moss $C > mine_nothing/$C
   indent -linux --braces-on-func-def-line mine_nothing/$C
   rm mine_nothing/$C~
   cd mine_nothing_moss
   for D in *.c; do
      indent -linux --braces-on-func-def-line $D
      rm $D~
   done
   cd ..
   cp $C original/$C
done
