#!/usr/bin/python

import sys
import subprocess
import decimal

class Clone:
   def __init__(self, sections):
      self.sections = frozenset(sections)

   def __str__(self):
      return "\n".join(map(sections, str))

   def __eq__(self, other):
      if isinstance(other, Clone):
         return self.sections == other.sections
      else:
         return False

   def __hash__(self):
      return hash((self.sections))

class Section:
   def __init__(self, filename, startLine, endLine, startToken, endToken, dist):
      self.filename = filename
      self.startLine = startLine
      self.endLine = endLine
      self.startToken = startToken
      self.endToken = endToken
      self.dist = dist

   def __str__(self):
      return "FILE: {4} LINES:{0}:{1} TOKENS:{2}:{3} DIST:{5}".format(self.startLine, self.endLine, self.startToken, self.endToken, self.filename, self.dist)

   def __eq__(self, other):
      if isinstance(other, Section):
         return self.filename == other.filename and self.startLine == other.startLine and self.endLine == other.endLine
      else:
         return False

   def __hash__(self):
      return hash((self.filename, self.startLine, self.endLine))

def lineToSection(line):
   tokens = line.split()
   filename = tokens[3]
   lines = tokens[4]
   startToken = tokens[8]
   endToken = tokens[9]
   lines = lines.split(":")
   startLine = int(lines[1])
   endLine = int(lines[2])
   startToken = int(startToken.split(":")[1])
   endToken = int(endToken.split(":")[1])
   dist = decimal.Decimal(tokens[1].split(":")[1])
   return Section(filename, startLine, endLine, startToken, endToken, dist)

def getLineSection(filename, startLine, endLine):
   filename = open(filename, "r")
   lines = filename.readlines()
   joined = "".join(lines[startLine - 1:endLine - 1])
   return joined

def getTokenSection(filename, startToken, endToken):
   proc = subprocess.Popen(args=["/home/ooee/otherTools/Deckard/src/main/myMain", str(filename), str(startToken), str(endToken)], stdout=subprocess.PIPE)
   proc.wait()
   return proc.stdout.read()

def main():
   if len(sys.argv) < 2:
      source = sys.stdin
   else:
      source = open(sys.argv[1], "r")
   clones = set()
   line = source.readline()
   while line != "":
      sections = set()
      while line != "" and line != "\n":
         section = lineToSection(line)
         sections.add(section)
         line = source.readline()
      clones.add(Clone(sections))
      line = source.readline()
   cloneNumber = 1
   for clone in clones:
      sectionNumber = 1
      print("========== {0} ==========".format(cloneNumber))
      for section in clone.sections:
         filename = section.filename
         joined = getLineSection(filename, section.startLine, section.endLine)
         print("({0}) {1}".format(sectionNumber, section))
         print(joined)
         tokens = getTokenSection(filename, section.startToken, section.endToken)
         print(tokens)
         sectionNumber += 1
      cloneNumber += 1
      
main()
