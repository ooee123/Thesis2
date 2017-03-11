#!/usr/bin/python

import sys

def getLineSection(filename, startLine, endLine):
   filename = open(filename, "r")
   lines = filename.readlines()
   joined = "".join(lines[startLine - 1:endLine])
   return joined

def main():
   if len(sys.argv) < 3:
      sys.stderr.write("<start line> <end_line> <file>\n")
      sys.exit(-1)
   else:
      print(getLineSection(sys.argv[2], int(sys.argv[0]), int(sys.argv[1])))
   
if __name__ == "__main__":
   #main()
   pass
