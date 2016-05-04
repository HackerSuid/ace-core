#!/bin/bash

ROOT=$1
LEVELS=$2
DIRECTIONS='north east south west'
PATTERNDIR="$(pwd)/sense_patterns"

echo "Generating $LEVELS-level structure world in $ROOT"

PATTERNARRAY=($(ls $PATTERNDIR))
NUMPATTERNS=${#PATTERNARRAY[@]}

cd $ROOT &> /dev/null
if [ $? -ne 0 ]; then
    echo "Failed to cd to $ROOT"
    exit 1
fi

function add_levels
{
    if [ $1 -gt 0 ]; then
        for d in $DIRECTIONS
        do
            #echo $1
            local cwd=$(pwd)
            #echo -e "\t$cwd"
            mkdir $d && cd $d
            cp $PATTERNDIR/${PATTERNARRAY[$((RANDOM%$NUMPATTERNS))]} .
            #echo copying $PATTERNDIR/${PATTERNARRAY[$((RANDOM%$NUMPATTERNS))]} to $d
            add_levels $(expr $1 - 1)
            cd $cwd
        done
    fi
}

cp $PATTERNDIR/${PATTERNARRAY[$((RANDOM%$NUMPATTERNS))]} .
add_levels $LEVELS
echo "Done!"
