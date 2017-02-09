#!/bin/bash

for D in *; do
   if [ -d "${D}" ] && [ "${D}" != "graphs" ]; then
      cd $D
      $@
      cd ..
   fi
done
