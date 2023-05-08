#!/bin/bash


cnt=0
while true
do
	convert slides.pdf[$cnt] -sharpen "0x1.0" -type truecolor -resize 400x300 slides-$cnt.bmp
	if [ $? == 1 ]
	then
		break
	fi
	let "cnt++"
done
mkdir -p $NAVY_HOME/fsimg/share/slides/
rm $NAVY_HOME/fsimg/share/slides/*
mv *.bmp $NAVY_HOME/fsimg/share/slides/
