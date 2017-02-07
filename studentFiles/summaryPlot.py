#!/usr/bin/python

import os
import json
import matplotlib.pyplot as plt
import numpy

aggregate = {"original": {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}, "mine_noSort": {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}, "mine_noID": {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}, "mine": {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "Moss": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}, "mine_nothing": {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "Moss": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}, "mine_noID_moss": {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}, "mine_moss": {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}}

students = {}
difference = {}

def main():
   global students
   global difference
   files = os.listdir(".")
   for f in files:
      if os.path.isdir(f):
         summaryFile = f + "/summary"
         if os.path.isfile(summaryFile):
            summary = open(summaryFile, "r")
            results = json.load(summary)
            students[f] = results
            print(f)
            beforeAfterBars(results, "JPlag")
            beforeAfterBars(results, "Simian")
            for key in results:
               for tool in results[key]:
                  for data in results[key][tool]:
                     aggregate[key][tool][data] += results[key][tool][data]
      else:
         print("Summary file not found for " + f)
   aggregateDifferences = processDifferences(aggregate)
   print(json.dumps(aggregate, indent = 4))
   beforeAfterBars(aggregate, "JPlag")
   for student in students:
      difference[student] = processDifferences(students[student])

def processDifferences(result):
   original = result["original"]
   mine_nothing = result["mine_nothing"]
   mine = result["mine"]
   mine_noID = result["mine_noID"]
   difference = processToolDifferent(original, mine)
   return difference

def processToolDifferent(before, after):
   difference = {}
   for tool in after:
      if tool in before:
         difference[tool] = {}
         for data in after[tool]:
            difference[tool][data] = after[tool][data] - before[tool][data]
   return difference

def beforeAfterBars(result, toolName):
   ind = range(len(result.keys()))
   plt.title(toolName)
   width = 0.35
   barNumber = 0
   for version in result:
      height = result[version][toolName]["lines"]
      original = plt.bar(barNumber, height, width = width, align='center')
      plt.text(barNumber, height, str(height), ha='center', va='bottom')
      barNumber += 1
   plt.xticks(ind, result.keys())
   plt.show()

main()
