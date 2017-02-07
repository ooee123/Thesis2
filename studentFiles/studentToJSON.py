#!/usr/bin/python

import os
import sys
import subprocess
import json
from bs4 import BeautifulSoup
from urllib2 import urlopen

TOTAL_SCRIPT = "/home/ooee/Thesis2/otherTools/Simian/grepTotal.sh"
CLONES_SCRIPT = "/home/ooee/Thesis2/otherTools/Simian/grepClones.sh"
SOURCE_SUBDIRECTORIES = ["mine", "mine_noID", "mine_noSort", "mine_nothing", "original", "mine_noID_moss", "mine_moss"]

def runStats():
   proc = subprocess.Popen(args=[TOTAL_SCRIPT, "Report"], stdout=subprocess.PIPE)
   output = proc.stdout.read()
   numbers = output.split(",")
   totalLines = int(numbers[0])
   totalClones = int(numbers[1])
   return (totalLines, totalClones)

def simianStats(resultsSubDirDict):
   os.chdir("analysis/Simian")
   res = runStats()
   os.chdir("../..")
   simian = {}
   simian['lines'] = res[0]
   simian['clones'] = res[1]
   resultsSubDirDict['Simian'] = simian

def getRange(string):
   l = string.find("(")
   r = string.rfind(")")
   tokens = string[l:r + 1].strip("()").split("-")
   return (int(tokens[0]), int(tokens[1]), string[:l])

def JPlagStats(resultsSubDirDict):
   matchedLines = 0
   clones = 0
   os.chdir("analysis/Jplag/result")
   files = os.listdir(".")
   for f in files:
      if f.endswith("-top.html"):
         html = open(f, "r")
         soup = BeautifulSoup(html, "lxml")
         tableRows = soup.body.center.table.find_all('tr')
         tableRows.pop(0)
         for row in tableRows:
            clones += 1
            cells = row.find_all('td')
            firstRange = getRange(cells[1].get_text())
            first = firstRange[1] - firstRange[0] + 1
            secondRange = getRange(cells[2].get_text())
            second = secondRange[1] - secondRange[0] + 1
            matchedLines += max(first, second)
   resultsSubDirDict['JPlag'] = {'lines': matchedLines, 'clones': clones}
   os.chdir("../../..")

def deckardStats(resultsSubDirDict):
   pass

def mossStats(resultsSubDirDict):
   proc = subprocess.Popen(args=["grep", "http://moss.stanford.edu/results/", "Report"], stdout=subprocess.PIPE)
   url = proc.stdout.read()
   html = urlopen(url).read()
   soup = BeautifulSoup(html, "lxml")
   rows = soup.body.table.find_all('tr')
   rows.pop(0)
   totalLines = 0
   totalClones = 0
   for row in rows:
      cells = row.find_all('td')
      lines = int(cells[2].get_text())
      totalLines += lines
   totalClones = len(rows)
   resultsSubDirDict['Moss'] = {'lines': totalLines, 'clones': totalClones}

def cloneDRStats(resultsSubDirDict):
   if os.path.isdir("cloneDR.results"):
      os.chdir("cloneDR.results")
      html = open("index.html", "r")
      soup = BeautifulSoup(html, "lxml")
      rows = soup.body.find_all("table")[1].tbody.find_all("tr")
      clones = int(rows[4].find_all('td')[1].get_text())
      lines = int(rows[7].find_all('td')[1].get_text())
      resultsSubDirDict['CloneDR'] = {'lines': lines, 'clones': clones}
      os.chdir("..")
   else:
      sys.stderr.write("Clone DR not present. Omitting\n")

def getToolsStats():
   results = {}
   for subdir in SOURCE_SUBDIRECTORIES:
      resultsSubDir = {}
      os.chdir(subdir)
      simianStats(resultsSubDir)
      JPlagStats(resultsSubDir)
      deckardStats(resultsSubDir)
      if subdir == "mine" or subdir == "mine_nothing":
         os.chdir("../" + subdir + "_moss")
         mossStats(resultsSubDir)
         os.chdir("../" + subdir)
      cloneDRStats(resultsSubDir)
      results[subdir] = resultsSubDir
      os.chdir("..")
   f = open("summary", "w")
   f.write(json.dumps(results, indent = 4))
   f.close()
   
getToolsStats()
