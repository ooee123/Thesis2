#!/usr/bin/python

from bs4 import BeautifulSoup
from urllib2 import urlopen

import sys

def processMatch(url):
   baseUrl = url.rstrip("html.")
   topUrl = baseUrl + "-top.html"
   html = urlopen(topUrl).read()
   soup = BeautifulSoup(html, "lxml")
   tableRows = soup.body.center.table.find_all('tr')
   for i in xrange(1, len(tableRows)):
      tableRow = tableRows[i]
      cells = tableRow.find_all('td')
      print(cells[0].get_text().strip())
      print(cells[2].get_text().strip())
      
def processSummaryPage(url):
   html = urlopen(url).read()
   soup = BeautifulSoup(html, "lxml")
   tableRows = soup.body.table.find_all('tr')
   for i in xrange(1, len(tablerows)):
      tableRow = tableRows[i]
      cells = tableRow.find_all('td')
      linesMatched = cells[2].get_text()

baseUrl = sys.argv[1]
html = urlopen(baseUrl).read()
soup = BeautifulSoup(html, "lxml")
links = soup.find_all('a')
matches = []
for link in links:
   url = link.get('href')
   if baseUrl in url and url not in matches:
      matches.append(url)
print(matches)
processMatch(matches[0])

