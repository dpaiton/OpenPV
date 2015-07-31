#!/bin/bash
echo "WARNING!!!!! This script does not check anything. DO NOT RUN BLIND!  Run? [y,n]:"
read -n 1 -s run
if [ "$run" != "y" ] && [ "$run" != "Y" ]; then
   echo "Script Aborted"
   exit
fi
set -x #echo on
#########################################################################################
# Moves Petavision output files from output folder to the Classification folder and
# creates symbolic links for the demo params
# Must be run from within CurrentModel directory 
#
# NOTE: This script does not attempt to check if anything exists before replacing
#       (Check all paths/filenames before running)
#
#   Demo params look for the following files in CurrentModel (Dependent on run type)
# Classifiers/BiasS2ToGroundTruthReconS2Error_W.pvp
# Classifiers/S2MaxPooledToGroundTruthReconS2Error_W.pvp
# Classifiers/S2SumPooledToGroundTruthReconS2Error_W.pvp
# ConfidenceTables/confidenceTable.mat
# InitialV/S1Landscape_V.pvp
# InitialV/S1Portrait_V.pvp
# InitialV/S1Square_V.pvp
# InitialV/S2Landscape_V.pvp
# InitialV/S2Portrait_V.pvp
# InitialV/S2Square_V.pvp
# S1S2Weights/S1ToImageReconS1Error_W.pvp
# S1S2Weights/S2ToS1ReconS2Error_W.pvp
#
# 07/15/2015 Will Shainin
#########################################################################################
DIR=$(pwd)
GROUP="compneuro"

# PV output directory names (to be moved) 
VCUR="landscape25"
VCP="Checkpoint6366400"
CCUR="landscape25a"
CCP="Checkpoint200000"

# PV output directories
CLASSDIR="/nh/compneuro/Data/PASCAL_VOC/PASCAL_S1_96_S2_1536_SumMaxPooled_SLP/VOC2007_$CCUR"
S1S2DIR="/nh/compneuro/Data/PASCAL_VOC/PASCAL_S1_96_S2_1536/VOC2007_$VCUR"

# Location of all models for demo
MODELSDIR="/nh/compneuro/Data/PASCAL_VOC/demo_PASCAL_Classification_Models"

# readpvpfile path
RPVP="~/workspace/PetaVision/pv-core/mlab/util"

### CLASSIFIER
CLASSCP="$CLASSDIR/Checkpoints/$CCP"
## Create new directory for copying
NEWCLDIR="$MODELSDIR/Classifiers/$CCUR-$CCP"
mkdir $NEWCLDIR
chgrp $GROUP $NEWCLDIR
chmod g+s $NEWCLDIR

## Classifier weight files 
BIAS="BiasS2ToGroundTruthReconS2Error_W.pvp"
MAXP="S2MaxPooledToGroundTruthReconS2Error_W.pvp"
SUMP="S2SumPooledToGroundTruthReconS2Error_W.pvp"

## Perform copy
cp $CLASSCP/$BIAS $NEWCLDIR/$BIAS 
cp $CLASSCP/$MAXP $NEWCLDIR/$MAXP 
cp $CLASSCP/$SUMP $NEWCLDIR/$SUMP 

## Add symbolic link
ln -s $NEWCLDIR/ Classifiers

## Create README that notes where the files came from and when
echo "The pvp files in this directory were copied from:" > $NEWCLDIR/README
echo "$CLASSCP" >> $NEWCLDIR/README
echo "on $(date +%m/%d/%y@%H:%M)" >> $NEWCLDIR/README

### Confidence Table
## Create new directory for copying
NEWCTDIR="$MODELSDIR/ConfidenceTables/"$CCUR"_"$(date +%m%d%y_%H%M)""
mkdir $NEWCTDIR
chgrp $GROUP $NEWCTDIR
chmod g+s $NEWCTDIR

## Ground truth files for confidence calculation
GT="a5_GroundTruth.pvp"
GTR="a11_GroundTruthReconS2.pvp"

## Perform copy (pvp files and octave script)
cp $CLASSDIR/$GT $NEWCTDIR/
cp $CLASSDIR/$GTR $NEWCTDIR/ 
cp ../createConfidenceTable.m $NEWCTDIR/

## Create confidence table
cd $NEWCTDIR/
octave --eval "confidenceTable = createConfidenceTable('a5_GroundTruth.pvp', 'a11_GroundTruthReconS2.pvp', '$RPVP', 1000); save -7 confidenceTable.mat confidenceTable;"
cd $DIR

## Create README that notes where the files came from and when
echo "The pvp files in this directory were copied from:" > $NEWCTDIR/README
echo "$CLASSDIR" >> $NEWCTDIR/README
echo "on $(date +%m/%d/%y@%H:%M), and passed as input to createConfidenceTable.m to create the file confidenceTable.mat" >> $NEWCTDIR/README
echo "The command was: " >> $NEWCTDIR/README
echo "octave --eval \"confidenceTable = createConfidenceTable('$GT', '$GTR', '$RPVP', 1000); save -7 confidenceTable.mat confidenceTable;\"" >> $NEWCTDIR/README

## Add symbolic link
ln -s $NEWCTDIR/ ConfidenceTables

#### Initial S1 S2 run files
INITCP="$S1S2DIR/Checkpoints/$VCP"

### Initialization values
## Create new directory for copying
NEWIVDIR="$MODELSDIR/InitialV/$VCUR"
mkdir $NEWIVDIR
chgrp $GROUP $NEWIVDIR
chmod g+s $NEWIVDIR

## Initialization activity files 
S1V="S1_V.pvp"
S2V="S2_V.pvp"

## Destiniation names
S1L="S1Landscape_V.pvp"
S2L="S2Landscape_V.pvp"
S1S="S1Square_V.pvp"
S2S="S2Square_V.pvp"
S1P="S1Portrait_V.pvp"
S2P="S2Portrait_V.pvp"

## Perform copy
cp $INITCP/$S1V $NEWIVDIR/$S1L
cp $INITCP/$S2V $NEWIVDIR/$S2L
cp ../convertLandscapeToSquareAndPortrait.m $NEWIVDIR/

## Create portrait and square pvp activity files from landscape files
cd $NEWIVDIR/
octave --eval "convertLandscapeToSquareAndPortrait('$S1L', '$S1S', '$S1P', '$RPVP');"
octave --eval "convertLandscapeToSquareAndPortrait('$S2L', '$S2S', '$S2P', '$RPVP');"
cd $DIR

## Create README that notes where the files came from and when
echo "Landscape membrane potential files in this directory were copied from:" > $NEWIVDIR/README
echo "$INITCP/$S1V" >> $NEWIVDIR/README
echo "$INITCP/$S2V" >> $NEWIVDIR/README
echo "on $(date +%m/%d/%y@%H:%M)" >> $NEWIVDIR/README
echo "" >> $NEWIVDIR/README
echo "Square and portrait files were generated using the command:" >> $NEWIVDIR/README
echo "octave --eval \"convertLandscapeToSquareAndPortrait('$S1L', '$S1S', '$S1P, '$RPVP');\"" >> $NEWIVDIR/README
echo "octave --eval \"convertLandscapeToSquareAndPortrait('$S2L', '$S2S', '$S2P, '$RPVP');\"" >> $NEWIVDIR/README
echo "" >> $NEWIVDIR/README
echo "The resulting square files have a square aspect ratio, with the landscape data centered in the middel of the square." >> $NEWIVDIR/README
echo "Square membrane potential files were taken from the landscape files, with padding added to make a square aspect ratio." >> $NEWIVDIR/README
echo "That is, S1Square_V.pvp is 128x128x96, with S1Square{1}.values(:,17:112,:)==S1Landscape{1}.values;" >> $NEWIVDIR/README
echo "and S2Square_V.pvp is 64x64x1536, with S2Square{1}.values(:,9:56,:)==S2Landscape{1}.values;" >> $NEWIVDIR/README
echo "" >> $NEWIVDIR/README
echo "Portrait membrane potential values were taken from the landscape files, with a permute(V, [2 1 3]) command applied." >> $NEWIVDIR/README
echo "Thus, S1Portrait_V is 96x128 and S2Portrait_V is 48x64; while S1Landscape_V is 128x96 and S1Landscape_V is 64x48." >> $NEWIVDIR/README

## Add symbolic link
ln -s $NEWIVDIR/ InitialV

### S1 S2 Weights
## Create new directory for copying
NEWS1DIR="$MODELSDIR/S1ToImageWeights/$VCUR"
NEWS2DIR="$MODELSDIR/S2ToS1Weights/$VCUR"
mkdir $NEWS1DIR
mkdir $NEWS2DIR
chgrp $GROUP $NEWS1DIR
chgrp $GROUP $NEWS2DIR
chmod g+s $NEWS1DIR
chmod g+s $NEWS2DIR

## Weights files
S1W="S1ToImageReconS1Error_W.pvp"
S2W="S2ToS1ReconS2Error_W.pvp"

## Perform copy
cp $INITCP/$S1W $NEWS1DIR/$S1W
cp $INITCP/$S2W $NEWS2DIR/$S2W

## Create README that notes where the files came from and when
echo "The pvp file in this directory was copied from:" > $NEWS1DIR/README
echo "$INITCP" >> $NEWS1DIR/README
echo "on $(date +%m/%d/%y@%H:%M)" >> $NEWS1DIR/README
echo "The pvp file in this directory was copied from:" > $NEWS2DIR/README
echo "$INITCP" >> $NEWS2DIR/README
echo "on $(date +%m/%d/%y@%H:%M)" >> $NEWS2DIR/README

## Add symbolic links
ln -s $NEWS1DIR/ S1ToImageWeights
ln -s $NEWS2DIR/ S2ToS1Weights
