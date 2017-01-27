#!/usr/bin/python

import decimal
import sys

similarityThreshold = decimal.Decimal("0.95")
maxParameters = 6
minNodes = 6
stringLengthNode = 16
startingDepth = 2

string = """C~GCC4 CloneDR 1.0
{0}
{1}
{2}
{3}
{4}
<
{5}"""

def main():
   files = sys.argv[1:]
   files = "\n".join(files)
   print(string.format(similarityThreshold, maxParameters, minNodes, stringLengthNode, startingDepth, files))

main()
