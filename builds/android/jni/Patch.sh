#!/bin/bash

#Author: Rajib Chandra Das
#

current_dir=$(pwd);

cp -f ./Android.mk ./Android.mk.backup;
rm -f ./Android.mk;

destFile="Android.mk";
srcFile="Android.mk.backup";
echo "Patching $current_dir/$destFile";
touch "$destFile";
bLineShouldWrite=0;

while IFS= read -r line || [[ -n "$line" ]]
do
    
    if [[ $line == *"#START_BUILDING_MEDIAENGINE"* ]]; then
		bLineShouldWrite=1;
    fi

    if [ $bLineShouldWrite == 1 ]; then
    	echo "$line" >> $destFile;
    fi

	if [[ $line == *"#END_BUILDING_MEDIAENGINE"* ]]; then
		bLineShouldWrite=0;
    fi

done < $srcFile

echo "Patching Completed....";