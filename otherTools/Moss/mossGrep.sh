#!/bin/bash

URL=$(grep "http://moss.stanford.edu/results/" Results)
wget $URL
