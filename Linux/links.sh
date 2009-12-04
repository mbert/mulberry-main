#!/bin/sh

# paths to search

all_paths="../Sources/Application \
	    ../Sources/Formatting \
	    ../Sources/Plugins \
	    ../Sources/PP_Sources \
	    ../Sources/Support \
	    ../../Sources_Common/Application \
	    ../../Sources_Common/Automation \
	    ../../Sources_Common/Calendar_Store \
	    ../../Sources_Common/Formatting \
	    ../../Sources_Common/HTTP \
	    ../../Sources_Common/i18n \
	    ../../Sources_Common/Mail \
	    ../../Sources_Common/Offline \
	    ../../Sources_Common/Plugins \
	    ../../Sources_Common/Preferences_Store \
	    ../../Sources_Common/Resources \
	    ../../Sources_Common/Support \
	    ../../Sources_Common/Tasks \
	    ../../Sources_Common/VCard_Store \
	    ../../Libraries/CICalendar/Source \
	    ../../Libraries/vCard/Source \
	    ../../Libraries/XMLLib/Source"

# clear out old link directory
\rm -rf Includes/*
cd Includes

# get list of header files

for item in $all_paths
do
    all_files="$all_files `find $item -name \".AppleDouble\" -prune -o -name \"*.h\" -print | tr \" \" \"_\"`"
done

# do links
for item in $all_files
do
ln -s "$item" .
#echo "$item"
done
