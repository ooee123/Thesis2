OTHERTOOLS_DIR=/home/ooee/Thesis2/otherTools
SIMIAN_SH=$OTHERTOOLS_DIR/Simian/simian.sh
SIMIAN_EXTRACT_CLONES=$OTHERTOOLS_DIR/Simian/simianClonesOutput.py
SIMIAN_IGNORE_ID_SH=$OTHERTOOLS_DIR/Simian/simian_ignoreIdentifiers.sh
JPLAG_SH=$OTHERTOOLS_DIR/Jplag/Jplag.sh
DECKARD_SH=$OTHERTOOLS_DIR/Deckard/scripts/clonedetect/deckard.sh
DECKARD_PARSE=$OTHERTOOLS_DIR/Deckard/parseLine.py
CLONEDR_SETUP=$OTHERTOOLS_DIR/Clonedr/cloneDrSetup.py

# Given an absolute path to the directory of the source files...
analysis=analysis
SOURCE_DIR=$(pwd)
$CLONEDR_SETUP *.c > cloneDR
mkdir $analysis
cd $analysis

#Simian
echo "Running Simian"
mkdir Simian
$SIMIAN_SH $SOURCE_DIR/*.c > Simian/Report
$SIMIAN_EXTRACT_CLONES Simian/Report > Simian/Clones

#Simian Ignoring Identifiers
echo "Running Simian_Ignore_ID"
mkdir Simian_Ignore_ID
$SIMIAN_IGNORE_ID_SH $SOURCE_DIR/*.c > Simian_Ignore_ID/Report
$SIMIAN_EXTRACT_CLONES Simian_Ignore_ID/Report > Simian_Ignore_ID/Clones

#Jplag
echo "Running JPlag"
mkdir Jplag
cd Jplag
$JPLAG_SH $SOURCE_DIR > Report
cd ..

#Deckard
#echo "Running Deckard"
#mkdir Deckard
#cd Deckard
#cp $OTHERTOOLS_DIR/config .
#$DECKARD_SH > Report
#$DECKARD_PARSE clusters/post* > clones
#cd ..

#CloneDR Setup
echo "Setting up CloneDR"
$CLONEDR_SETUP *.c > cloneDR
