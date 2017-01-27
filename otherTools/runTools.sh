OTHERTOOLS_DIR=/home/ooee/otherTools
SIMIAN_SH=$OTHERTOOLS_DIR/Simian/simian.sh
SIMIAN_IGNORE_ID_SH=$OTHERTOOLS_DIR/Simian/simian_ignoreIdentifiers.sh
JPLAG_SH=$OTHERTOOLS_DIR/Jplag/Jplag.sh
DECKARD_SH=$OTHERTOOLS_DIR/Deckard/scripts/clonedetect/deckard.sh
DECKARD_PARSE=$OTHERTOOLS_DIR/Deckard/parseLine.py

# Given an absolute path to the directory of the source files...
#analysis=$(date "+%Y-%m-%d_%H:%M:%S")
analysis=analysis
mkdir $analysis
cd $analysis

#Simian
mkdir Simian
$SIMIAN_SH $@/*.c > Simian/Report

#Simian Ignoring Identifiers
mkdir Simian_Ignore_ID
$SIMIAN_IGNORE_ID_SH $@/*.c > Simian_Ignore_ID/Report

#Jplag
mkdir Jplag
cd Jplag
$JPLAG_SH $@ > Report
cd ..

#Deckard
mkdir Deckard
cd Deckard
cp $OTHERTOOLS_DIR/config .
$DECKARD_SH > Report
$DECKARD_PARSE clusters/post* > clones
