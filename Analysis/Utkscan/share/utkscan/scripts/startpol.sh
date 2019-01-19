#!/bin/bash

echo Enter last three digits of runfile, eg 016

read number

filename=acq/runPR270_$number.pld

jj-paass-pr270/install/bin/utkscan -b -i $filename -c jj-paass-pr270/install/bin/Config_PR270.xml -o analysis/kpolout

wait
root -l  read_pol.C\(\"analysis/kpolout.pol\"\)
