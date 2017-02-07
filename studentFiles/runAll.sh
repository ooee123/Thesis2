#!/bin/bash

for D in *; do
   if [ -d "${D}" ]; then
      cd $D
      $@
      cd ..
   fi
done
