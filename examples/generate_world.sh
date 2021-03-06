#!/bin/bash

ROOT=$1
LEVELS=$2
DIRECTORIES='L R'
PATTERNDIR="$(pwd)/sense_patterns"

echo "Generating $LEVELS-level structure world in $ROOT"

PATTERNARRAY=($(ls $PATTERNDIR))
NUMPATTERNS=${#PATTERNARRAY[@]}

cd $ROOT &> /dev/null
if [ $? -ne 0 ]; then
    echo "Failed to cd to $ROOT"
    exit 1
fi

ROOTPATH=$(pwd)

function add_levels
{
    if [ $1 -gt 0 ]; then
        for d in $DIRECTORIES
        do
            #echo $1
            local cwd=$(pwd)
            #echo -e "\t$cwd"
            mkdir $d && cd $d
            cp $PATTERNDIR/${PATTERNARRAY[$((RANDOM%$NUMPATTERNS))]} .
            if [ $1 -eq 1 ]; then
                for d2 in $DIRECTORIES
                do
                    ln -s $ROOTPATH/$d2 $d2
                done
            fi
            #echo copying $PATTERNDIR/${PATTERNARRAY[$((RANDOM%$NUMPATTERNS))]} to $d
            add_levels $(expr $1 - 1)
            cd $cwd
        done
    fi
}

cp $PATTERNDIR/${PATTERNARRAY[$((RANDOM%$NUMPATTERNS))]} .
add_levels $LEVELS
echo "Done!"
