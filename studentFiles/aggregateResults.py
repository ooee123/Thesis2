#!/usr/bin/python

import os
import subprocess
import json

TOTAL_SCRIPT = "/home/ooee/Thesis2/otherTools/Simian/grepTotal.sh"
CLONES_SCRIPT = "/home/ooee/Thesis2/otherTools/Simian/grepClones.sh"
SIMIAN_SUBDIRECTORIES = ["mine", "mine_noID", "mine_noSort", "mine_nothing", "original", "mine_noID_moss", "mine_moss"]

def runStats():
   proc = subprocess.Popen(args=[TOTAL_SCRIPT, "Report"], stdout=subprocess.PIPE)
   output = proc.stdout.read()
   numbers = output.split(",")
   totalLines = int(numbers[0])
   totalClones = int(numbers[1])
   return (totalLines, totalClones)

def aggregate():
   directories = os.listdir(".")
   for directory in directories:
      os.chdir(directory)
      for subdir in SIMIAN_SUBDIRECTORIES:
         os.chdir(subdir + "/analysis/Simian")
         results = runStats()
         
