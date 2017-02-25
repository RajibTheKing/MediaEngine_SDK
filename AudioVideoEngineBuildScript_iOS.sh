#!/bin/bash

#Author: Rajib Chandra Das
#
SOURCE_DIRECTORY="/Users/RajibTheKing/Desktop/WorkProcedure/Hahaha/IPV-MediaEngine";
BUILD_DIRECTORY="/Users/RajibTheKing/Desktop/WorkProcedure/Hahaha/newLibs";
RELEASE_NAME="";
#This is a function
LogOutput()
{
	if [ "$2" == "RED" ]; then
  		echo "\033[1;31m$1";
  	elif [ "$2" == "BLUE" ]; then
  		echo "\033[1;34m$1";
  	elif [ "$2" == "CYAN" ]; then
  		echo "\033[1;36m$1";
  	else
  		echo "\033[1;32m$1";
	fi
	
	#echo "\033[1;31m$1"; #Red
	#echo "\033[1;32m$1"; #Green
	#echo "\033[1;33m$1"; #Yellow
	#echo "\033[1;34m$1"; #Blue
	#echo "\033[1;35m$1"; #Magenta
	#echo "\033[1;36m$1"; #Cyan
	#echo "\033[1;37m$1"; #Grey
	#echo "\033[1;38m$1"; #Light Grey
	#echo "\033[1;39m$1"; #Black
}



CreateDirectoryIfNotExist()
{
	if [ ! -d "$1" ]; then
		LogOutput "TheKing--> Creating $1 Directory" "CYAN";
		mkdir "$1" || exit 1;
	fi
}

read -p "Please Enter a RELEASE_NAME: " -r RELEASE_NAME;

if [ "$RELEASE_NAME" == "" ]; then
	LogOutput "TheKing--> You must give a RELEASE_NAME" "RED";
	exit 1;
fi

LogOutput "\n\nTheKing--> Just Started AudioVideoEngine Build";

LogOutput "TheKing--> Source Path Root = $SOURCE_DIRECTORY" "RED";
LogOutput "TheKing--> Build Path Root = $BUILD_DIRECTORY" "RED";
LogOutput "TheKing--> Release Name = $RELEASE_NAME" "RED";


CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/";
CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${RELEASE_NAME}/";
CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${RELEASE_NAME}/Debug-iphoneos/";

LogOutput "\n\n\nTheKing--> Creating Library for Debug-iphoneos\n\n\n";

xcodebuild 	-project "${SOURCE_DIRECTORY}/AudioVideoEngine.xcodeproj" \
			-scheme AudioVideoEngine \
			-configuration "Debug" \
			-sdk iphoneos10.2  \
			ARCHS="armv7 arm64" \
			CONFIGURATION_BUILD_DIR="$BUILD_DIRECTORY/${RELEASE_NAME}/Debug-iphoneos/" \
			ONLY_ACTIVE_ARCH=NO || exit 1;

CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${RELEASE_NAME}/Debug-iphonesimulator/";

LogOutput "\n\n\nTheKing--> Creating Library for Debug-iphonesimulator\n\n\n";

xcodebuild 	-project "$SOURCE_DIRECTORY/AudioVideoEngine.xcodeproj" \
			-scheme AudioVideoEngine \
			-configuration "Debug" \
			-sdk iphonesimulator10.2  \
			ARCHS="i386 x86_64 " \
			CONFIGURATION_BUILD_DIR="$BUILD_DIRECTORY/${RELEASE_NAME}/Debug-iphonesimulator/" \
			ONLY_ACTIVE_ARCH=NO || exit 1;

CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${RELEASE_NAME}/Debug-universal/";

LogOutput "\n\n\nTheKing--> Merging Library for Debug-Universal\n\n\n";

lipo -create 	"$BUILD_DIRECTORY/${RELEASE_NAME}/Debug-iphoneos/libAudioVideoEngine.a" \
				"$BUILD_DIRECTORY/${RELEASE_NAME}/Debug-iphonesimulator/libAudioVideoEngine.a" \
			 	-output "$BUILD_DIRECTORY/${RELEASE_NAME}/Debug-universal/libAudioVideoEngine.a";


CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${RELEASE_NAME}/include/";

LogOutput "\n\n\nTheKing--> Copy Header files related to libAudioVideoEngine..\n\n\n";

cp "$SOURCE_DIRECTORY/VideoEngineController/InterfaceOfAudioVideoEngine.h" "$BUILD_DIRECTORY/${RELEASE_NAME}/include/InterfaceOfAudioVideoEngine.h";

LogOutput "TheKing--> AudioVideoEngine BuildScript Finished Successfully...";


