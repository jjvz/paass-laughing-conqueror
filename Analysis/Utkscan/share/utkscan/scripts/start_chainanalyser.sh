#!/bin/bash
# usage: ./start_chainanalyser.sh

echo
echo 'Enter the main runnumber, e.g. 091: '
read RUN

echo 'Enter the last run part number, e.g. 23 (as in run_091-23): '
read runpart

#MDIR="/media/jjvz/PR231/PR270"
MDIR="/media/jjvz/DATA2/PR270"
echo $MDIR
RDIR="analysis"
echo $RDIR
PREFIX="runPR270_"
echo $PREFIX
#EXT="ldf"
EXT="pld"
echo $EXT
#END=$runpart
#echo $RUN

#select your runs, e.g. "for COUNT in {256,257}"
#for COUNT in {0..$runpart}
#for COUNT in {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30}
#for COUNT in {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44}
#for COUNT in {0..44}
for COUNT in $(seq 0 $runpart)
#for COUNT in {6048,6049,6050,6052,6054,6055,6056,6070,6071}
do
  echo $COUNT
  if [ $COUNT -lt 1 ]
  then
      FILE=$MDIR/$PREFIX$RUN.$EXT
  elif [ $COUNT -gt 0 ]
  then
      FILE=$MDIR/$PREFIX$RUN"-"$COUNT.$EXT
  else
      echo "This might not work"
  fi
  echo $FILE
#  if [ $COUNT -lt 1 ]
#  then
#      FILE2=$RDIR/$PREFIX$RUN
#  elif [ $COUNT -gt 0 ]
#  then
      FILE2=$RDIR/$PREFIX$RUN"-"$COUNT
#  else
#      echo "This might not work"
#  fi
  echo $FILE2
  if [ ! -e $FILE ]
      then
      echo "!!!!!! ERROR - Input file not found !!!!!!!"
  fi
  if [ -e $FILE ]
  then
     if [ -e $FILE2 ]
	  then
	  rm $FILE2
#	  rm -i $FILE2	#use this line to ask before overwriting!!
      fi
      if [ ! -e $FILE2 ]
          then
	  echo $FILE
	  jj-paass-pr270/install/bin/utkscan -b -i $FILE -c jj-paass-pr270/install/bin/Config_PR270.xml -o $FILE2
          wait
      fi
  fi
done
