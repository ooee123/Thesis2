#!/usr/bin/python
import sys

def main():
   filename = sys.argv[1]
   startLine = int(sys.argv[2])
   endLine = int(sys.argv[3])

   if startLine <= 0:
      startLine = 1
   filename = open(filename, "r")
   lines = filename.readlines()
   for x in range(startLine, endLine):
      print(lines[x - 1])

main()
