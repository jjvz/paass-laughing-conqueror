#!/bin/bash

echo
echo Enter last 3 digits of runfile, e.g. 016
read number

runname=runPR270_$number
filename=acq/$runname.pld

echo Analyzing runfile: $filename
echo

jj-paass-pr270/install/bin/utkscan -b -i $filename -c jj-paass-pr270/install/bin/Config_PR270.xml -o analysis/$runname
