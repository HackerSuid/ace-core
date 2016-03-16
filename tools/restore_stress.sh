#!/bin/sh

echo "Restoring sequence..."

if [ ! -f "sampledata/bitmaps/high-order/STRESS/12.bmp" ]
then
    echo "The sequence shouldn't be restored."
    exit
fi

mv sampledata/bitmaps/high-order/STRESS/9.bmp sampledata/bitmaps/high-order/STRESS/8.bmp
mv sampledata/bitmaps/high-order/STRESS/10.bmp sampledata/bitmaps/high-order/STRESS/9.bmp
mv sampledata/bitmaps/high-order/STRESS/11.bmp sampledata/bitmaps/high-order/STRESS/10.bmp
mv sampledata/bitmaps/high-order/STRESS/12.bmp sampledata/bitmaps/high-order/STRESS/11.bmp

echo "Done!"

