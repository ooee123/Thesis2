#!/usr/bin/python

import re
import sys
import subprocess
import os

clonePairScript = "/home/ooee/Thesis2/otherTools/Simian/grepClonePairs.sh"
extractLinesInFile = "/home/ooee/Thesis2/otherTools/extractLinesFromFile.py"
grepSed = "grep 'Between lines [[:digit:]]* and [[:digit:]]* in ' | sed 's/ Between lines \\([[:digit:]]*\\) and \\([[:digit:]]*\\) in \\(.*\\)/\\1,\\2,\\3/'"

class Code:
   def __init__(self, startLine, endLine, fileName, code):
      self.startLine = int(startLine)
      self.endLine = int(endLine)
      self.fileName = str(fileName)
      self.code = str(code)

class Clone:
   def __init__(self, codes):
      self.codes = codes;

def main():
   #execfile(extractLinesInFile)
   if len(sys.argv) < 1:
      sys.stderr.write("Needs file name\n")
      sys.exit(-1)
   report = open(sys.argv[1], "r").read()
   p = re.compile(r'Found [0-9]+ duplicate lines in the following files:')
   clones = filter(None, p.split(report))[1:]
   allClones = []
   for clone in clones:
      codeClones = []
      proc = subprocess.Popen(grepSed, shell=True, stdout=subprocess.PIPE, stdin=subprocess.PIPE)
      proc.stdin.write(clone)
      proc.stdin.close()
      lines = proc.stdout.readlines()
      for line in lines:
         tokens = line.strip().split(",")
         startLine = int(tokens[0])
         endLine = int(tokens[1])
         fileName = str(tokens[2])
         codeStr = getLineSection(fileName, startLine, endLine)
         code = Code(startLine, endLine, fileName, codeStr)
         codeClones.append(code)
      allClones.append(Clone(codeClones))
   printClones(allClones)

def printClones(clones):
   cloneNumber = 1
   for clone in clones:
      print(" == Clone {0} == ".format(cloneNumber))
      codeNumber = 1
      for code in clone.codes:
         print("===== {0} at {1}:{2} =====".format(code.fileName, code.startLine, code.endLine))
         print(code.code)
         codeNumber += 1
      cloneNumber += 1
"""
   #proc = subprocess.Popen([clonePairScript, sys.argv[1]], stdout=subprocess.PIPE)
   proc.wait()
   output = proc.stdout.readlines()

   sys.stderr.write("".join(output))
   lineNum = 0
   while lineNum < len(output):
      token1 = output[lineNum].strip().split(",")
      token2 = output[lineNum + 1].strip().split(",")
      clone1 = getLineSection(token1[2], int(token1[0]), int(token1[1]))
      clone2 = getLineSection(token2[2], int(token2[0]), int(token2[1]))
      print(" == Clone {0} == ".format((lineNum / 2) + 1))
      print("===== {0} at {1}:{2} =====".format(token1[2], token1[0], token1[1]))
      print(clone1)
      print("===== {0} at {1}:{2} =====".format(token2[2], token2[0], token2[1]))
      print(clone2)
      lineNum += 2
"""
def getLineSection(filename, startLine, endLine):
   filename = open(filename, "r")
   lines = filename.readlines()
   joined = "".join(lines[startLine - 1:endLine])
   return joined

main()
