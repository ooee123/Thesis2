#!/usr/bin/python

import os
import sys
import subprocess
import json
import collections
from bs4 import BeautifulSoup
from urllib2 import urlopen

TOTAL_SCRIPT = "/home/ooee/Thesis2/otherTools/Simian/grepTotal.sh"
CLONES_SCRIPT = "/home/ooee/Thesis2/otherTools/Simian/grepClones.sh"
SOURCE_SUBDIRECTORIES = ["original", "mine_nothing", "mine", "mine_noID", "mine_noSort", "mine_moss", "mine_noID_moss"]
GREP_SED = "grep 'Between lines [[:digit:]]* and [[:digit:]]* in ' | sed 's/ Between lines \\([[:digit:]]*\\) and \\([[:digit:]]*\\) in \\(.*\\)/\\1,\\2,\\3/'"
CLONE_COUNT = "grep 'Found [[:digit:]]* duplicate lines in the following files:' | wc -l"

def runStats():
   proc = subprocess.Popen(GREP_SED, shell=True, stdin=open("Report", "r"), stdout=subprocess.PIPE)
   proc.wait()
   cloneLines = proc.stdout.readlines()
   totalLines = 0
   for cloneLine in cloneLines:
      tokens = cloneLine.split(",")
      cloneSize = int(tokens[1]) - int(tokens[0]) + 1
      totalLines += cloneSize
   
   cloneCountProc = subprocess.Popen(CLONE_COUNT, shell=True, stdin=open("Report", "r"), stdout=subprocess.PIPE)
   cloneCountProc.wait()
   totalClones = int(cloneCountProc.stdout.read())
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
   tokens = 0
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
            tokens += int(cells[3].get_text())
   resultsSubDirDict['JPlag'] = {'lines': tokens, 'clones': clones}
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

def getToolsStats(simian=True, jplag=True, deckard=True, moss=True, cloneDR=True):
   results = collections.OrderedDict()
   for subdir in SOURCE_SUBDIRECTORIES:
      resultsSubDir = {}
      os.chdir(subdir)
      print(os.getcwd())
      if simian:
         simianStats(resultsSubDir)
      if jplag:
         JPlagStats(resultsSubDir)
      if deckard:
         deckardStats(resultsSubDir)
      if moss:
         if subdir == "mine" or subdir == "mine_nothing" or subdir == "mine_noID":
            os.chdir("../" + subdir + "_moss")
            mossStats(resultsSubDir)
            os.chdir("../" + subdir)
      if cloneDR:
         cloneDRStats(resultsSubDir)
      results[subdir] = resultsSubDir
      os.chdir("..")
   f = open("summary", "w")
   f.write(json.dumps(results, indent = 4))
   f.close()
   
if __name__ == "__main__":
   if len(sys.argv) > 1:
      s = 's' in sys.argv[1]      
      j = 'j' in sys.argv[1]      
      d = 'd' in sys.argv[1]      
      m = 'm' in sys.argv[1]
      c = 'c' in sys.argv[1]      
      getToolsStats(s, j, d, m, c)
   else:
      getToolsStats()
