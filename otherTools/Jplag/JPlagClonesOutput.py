#!/usr/bin/python

from bs4 import BeautifulSoup
import sys
import os

def getRange(string):
   l = string.find("(")
   r = string.rfind(")")
   tokens = string[l:r + 1].strip("()").split("-")
   return (int(tokens[0]), int(tokens[1]), string[:l])

def JPlagStats():
   matchedLines = 0
   clones = 0
   os.chdir(sys.argv[1])
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
            secondRange = getRange(cells[2].get_text())
            print("{0},{1},{2}".format(firstRange[0], firstRange[1], firstRange[2]))
            print("{0},{1},{2}".format(secondRange[0], secondRange[1], secondRange[2]))

if len(sys.argv) < 1:
   sys.stderr.write("Needs directory containing -top.html files\n")
   sys.exit(-1)
JPlagStats()
