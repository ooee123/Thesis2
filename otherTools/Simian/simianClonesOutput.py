#!/usr/bin/python

import sys
import subprocess

clonePairScript = "/home/ooee/Thesis2/otherTools/Simian/grepClonePairs.sh"
extractLinesInFile = "/home/ooee/Thesis2/otherTools/extractLinesFromFile.py"

def main():
   execfile(extractLinesInFile)
   if len(sys.argv) < 1:
      sys.stderr.write("Needs file name\n")
      sys.exit(-1)
   proc = subprocess.Popen([clonePairScript, sys.argv[1]], stdout=subprocess.PIPE)
   proc.wait()
   output = proc.stdout.readlines()

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

def getLineSection(filename, startLine, endLine):
   filename = open(filename, "r")
   lines = filename.readlines()
   joined = "".join(lines[startLine - 1:endLine - 1])
   return joined

main()
