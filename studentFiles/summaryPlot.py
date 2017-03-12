#!/usr/bin/python

import os
import collections
import sys
import json
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy
import shutil


aggregate = collections.OrderedDict()
aggregate["original"] =  {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}
aggregate["mine_nothing"] = {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "Moss": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}
aggregate["mine"] = {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "Moss": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}
aggregate["mine_noID"] = {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}, "Moss": {"clones": 0, "lines": 0}}
aggregate["mine_noSort"] =  {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}
aggregate["mine_moss"] = {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}
aggregate["mine_noID_moss"] = {"Simian": {"clones": 0, "lines": 0}, "JPlag": {"clones": 0, "lines": 0}, "CloneDR": {"clones": 0, "lines": 0}}

allStudents = collections.OrderedDict()
allStudents["original"] =  {"Simian": [], "JPlag": [], "CloneDR": []}
allStudents["mine_nothing"] = {"Simian": [], "JPlag": [], "Moss": [], "CloneDR": []}
allStudents["mine"] = {"Simian": [], "JPlag": [], "Moss": [], "CloneDR": []}
allStudents["mine_noID"] = {"Simian": [], "JPlag": [], "CloneDR": [], "Moss": []}
allStudents["mine_noSort"] =  {"Simian": [], "JPlag": [], "CloneDR": []}
allStudents["mine_moss"] = {"Simian": [], "JPlag": [], "CloneDR": []}
allStudents["mine_noID_moss"] = {"Simian": [], "JPlag": [], "CloneDR": []}

renaming = {"original": "original", "mine": "normalized", "mine_noID": "noID", "mine_noSort": "noPDG", "mine_nothing": "noID_PDG", "mine_moss": "split", "mine_noID_moss": "noID_split"}

renamingMoss = {'mine_nothing': 'noID_PDG_split', 'mine': 'split', 'mine_noID': 'noID_split'}

students = {}
difference = collections.OrderedDict()

showNumberClones = False

matplotlib.rcParams.update({'font.size': 11})
matplotlib.rcParams.update({'figure.figsize': (11, 6)})

def main(chosenDir=None):
   global students
   global difference
   graphsDirectory = 'graphs'
   if os.path.isdir(graphsDirectory):
      shutil.rmtree(graphsDirectory)
   os.mkdir(graphsDirectory)
   if chosenDir == None:
      files = os.listdir(".")
   else:
      files = [chosenDir]
   for f in files:
      if os.path.isdir(f):
         summaryFile = f + "/summary"
         if os.path.isfile(summaryFile):
            print(f)
            summary = open(summaryFile, "r")
            results = json.load(summary, object_pairs_hook=collections.OrderedDict)
            students[f] = results
            beforeAfterBars(results, "JPlag", f, "Tokens", graphsDirectory)
            beforeAfterBars(results, "Simian", f, "Lines", graphsDirectory)
            beforeAfterBars(results, "CloneDR", f, "Lines", graphsDirectory)
            beforeAfterBars(results, "Moss", f, "Lines", graphsDirectory)
            for key in results:
               for tool in results[key]:
                  allStudents[key][tool].append(results[key][tool]["lines"])
                  for data in results[key][tool]:
                     aggregate[key][tool][data] += results[key][tool][data]
      else:
         print("Summary file not found for " + f)
   #aggregateDifferences = processDifferences(aggregate)
   histogramDifferences(processDifferencesTool(students, "original", "mine_nothing", "Simian"), "Simian Differences original vs noID_PDG", "diff_Simian_original_nothing", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "original", "mine", "Simian"), "Simian Differences original vs normalized", "diff_Simian_original_mine", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "original", "mine_noID", "Simian"), "Simian Differences original vs noID_PDG", "diff_Simian_original_noID", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "mine_noID", "mine", "Simian"), "Simian Differences noID vs normalized", "diff_Simian_noID_mine", graphsDirectory)

   histogramDifferences(processDifferencesTool(students, "original", "mine_nothing", "JPlag"), "JPlag Differences original vs noID_PDG", "diff_Jplag_original_nothing", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "original", "mine", "JPlag"), "JPlag Differences original vs normalized", "diff_Jplag_original_mine", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "original", "mine_noID", "JPlag"), "JPlag Differences original vs noID_PDG", "diff_Jplag_original_noID", graphsDirectory)

   histogramDifferences(processDifferencesTool(students, "original", "mine_nothing", "CloneDR"), "CloneDR Differences original vs noID_PDG", "diff_Clonedr_original_nothing", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "original", "mine", "CloneDR"), "CloneDR Differences original vs normalized", "diff_Clonedr_original_mine", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "original", "mine_noID", "CloneDR"), "CloneDR Differences original vs noID_PDG", "diff_Clonedr_original_noID", graphsDirectory)
   histogramDifferences(processDifferencesTool(students, "mine_noID", "mine", "CloneDR"), "CloneDR Differences noID vs normalized", "diff_Clonedr_noID_mine", graphsDirectory)

   histogramDifferences(processDifferencesTool(students, "mine_nothing", "mine", "Moss"), "Moss Differences noID_PDG_split vs split", "diff_Moss_nothing_split_mine_split", graphsDirectory)
   beforeAfterBars(aggregate, "JPlag", "summary", "Tokens", graphsDirectory)
   beforeAfterBars(aggregate, "Simian", "summary", "Lines", graphsDirectory)
   beforeAfterBars(aggregate, "CloneDR", "summary", "Lines", graphsDirectory)
   beforeAfterBars(aggregate, "Moss", "summary", "Lines", graphsDirectory)
   #summaryBars(allStudents, "JPlag", "statistics", "Tokens", graphsDirectory)
   #summaryBars(allStudents, "Simian", "statistics", "Lines", graphsDirectory)
   #summaryBars(allStudents, "CloneDR", "statistics", "Lines", graphsDirectory)
   #summaryBars(allStudents, "Moss", "statistics", "Lines", graphsDirectory)
   #boxPlot(allStudents, "Simian", "statisticsBox", "Lines", graphsDirectory)
   #boxPlot(allStudents, "JPlag", "statisticsBox", "Tokens", graphsDirectory)
   #boxPlot(allStudents, "CloneDR", "statisticsBox", "Lines", graphsDirectory)
   #boxPlot(allStudents, "Moss", "statisticsBox", "Lines", graphsDirectory)

   for student in students:
      #difference[student] = processDifferences(students[student])
      pass

def processDifferencesTool(allStudents, fromVersion, toVersion, toolName):
   differences = []
   for student in allStudents:
      fromV = allStudents[student][fromVersion]
      toV = allStudents[student][toVersion]
      fromLines = fromV[toolName]['lines']
      toLines = toV[toolName]['lines']
      differenceLines = toLines - fromLines;
      differences.append(differenceLines)
   print("_".join([fromVersion, toVersion, toolName, str(differences)]))
   return differences

def histogramDifferences(data, title, filePrefix, filePath):
   barWidth = 5
   bins = 30
   if max(data) - min(data) > barWidth * bins:
      bins = int((max(data) - min(data) + 0.5) / barWidth)
   plt.title(title)
   plt.hist(data, bins)
   plt.savefig(os.path.join(filePath, filePrefix), bbox_inches='tight')
   plt.clf()

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

def boxPlot(data, toolName, studentName, yAxis, filePath="."):
   width = 0.75
   plt.title(toolName)
   arrayOfData = []
   versions = []
   for version in data:
      if toolName in data[version]:
         versions.append(version)
         arrayOfData.append(data[version][toolName])
   plt.boxplot(arrayOfData, widths=width, whiskerprops={'linestyle':'-'})
   if toolName == "Moss":
      labels = [renamingMoss[x] for x in versions]
   else:
      labels = [renaming[x] for x in versions]
   plt.xticks(range(1, len(arrayOfData) + 1), labels)
   plt.xlabel("Normalization Method")
   plt.ylabel(yAxis)
   plt.savefig(os.path.join(filePath, studentName + "_" + toolName), bbox_inches='tight')
   plt.clf()

def summaryBars(data, toolName, studentName, yAxis, filePath="."):
   plt.title(toolName)
   width = 0.75
   barNumber = 0
   bars = 0
   height = []
   versions = []
   colors = []
   numberOfClones = []
   stdDevs = []
   first = True
   for version in data:
      if toolName in data[version]:
         if first:
            colors.append('r')
            first = False;
         else:
            colors.append('b')
         stdDev = numpy.std(data[version][toolName], ddof=1)
         stdDevs.append(stdDev)
         avg = numpy.average(data[version][toolName])
         versions.append(version)
         bars += 1
         barHeight = avg
         height.append(barHeight)
         #plt.boxplot(data[version][toolName])
         plt.text(barNumber, barHeight, barHeight, ha='center', va='bottom')

         numberOfClonesBarHeight = len(data[version][toolName])
         numberOfClones.append(numberOfClonesBarHeight)
         if showNumberClones:
            plt.text(barNumber, numberOfClonesBarHeight, numberOfClonesBarHeight, ha='center', va='bottom')
         barNumber += 1
      else:
         pass
   barNumber = numpy.arange(0, barNumber, 1)
   barlist = plt.bar(barNumber, height, width = width, align='center', color=colors, yerr=stdDevs)
   if showNumberClones:
      numberOfClonesBarList = plt.bar(barNumber + width / 4, numberOfClones, width=width / 2, align='center', color=colors)
   if toolName == "Moss":
      labels = [renamingMoss[x] for x in versions]
   else:
      labels = [renaming[x] for x in versions]
   plt.xticks(barNumber, labels)
   plt.xlabel("Normalization Method")
   plt.ylabel(yAxis)
   plt.savefig(os.path.join(filePath, studentName + "_" + toolName), bbox_inches='tight')
   plt.clf()

def beforeAfterBars(result, toolName, studentName, yAxis, filePath="."):
   plt.title(toolName)
   width = 0.75
   barNumber = 0
   bars = 0
   height = []
   versions = []
   colors = []
   numberOfClones = []
   first = True
   for version in result:
      if toolName in result[version]:
         if first:
            colors.append('r')
            first = False;
         else:
            colors.append('b')
         versions.append(version)
         bars += 1
         barHeight = result[version][toolName]["lines"]
         height.append(barHeight)
         plt.text(barNumber, barHeight, barHeight, ha='center', va='bottom')

         numberOfClonesBarHeight = result[version][toolName]["clones"]
         numberOfClones.append(numberOfClonesBarHeight)
         if showNumberClones:
            plt.text(barNumber, numberOfClonesBarHeight, numberOfClonesBarHeight, ha='center', va='bottom')
         barNumber += 1
      else:
         pass
   barNumber = numpy.arange(0, barNumber, 1)
   barlist = plt.bar(barNumber, height, width = width, align='center', color=colors)
   if showNumberClones:
      numberOfClonesBarList = plt.bar(barNumber + width / 4, numberOfClones, width=width / 2, align='center', color=colors)
   if toolName == "Moss":
      labels = [renamingMoss[x] for x in versions]
   else:
      labels = [renaming[x] for x in versions]
   plt.xticks(barNumber, labels)
   plt.xlabel("Normalization Method")
   plt.ylabel(yAxis)
   plt.ylim(0, max(height) * 1.1)
   plt.savefig(os.path.join(filePath, studentName + "_" + toolName), bbox_inches='tight')
   plt.clf()

if len(sys.argv) > 1:
   main(sys.argv[1])
else:
   main()
