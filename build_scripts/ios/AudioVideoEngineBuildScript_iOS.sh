#!/bin/bash

#Author: Rajib Chandra Das
#
PROJECT_DIRECTORY="../../builds/ios";
BUILD_DIRECTORY="../../output";
BUILD_CONFIGURATION=$1;
BUILD_NAME=$2;
XCODE_SDK_VERSION=$3;

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


#read -p "Please Enter a BUILD_NAME: " -r BUILD_NAME;
if [ "$BUILD_NAME" == "" ]; then
	LogOutput "TheKing--> You must give a BUILD_NAME" "RED";
	exit 1;
fi

LogOutput "\n\nTheKing--> Just Started AudioVideoEngine Build";

LogOutput "TheKing--> Source Path Root = $PROJECT_DIRECTORY" "RED";
LogOutput "TheKing--> Build Path Root = $BUILD_DIRECTORY" "RED";
LogOutput "TheKing--> Release Name = $BUILD_NAME" "RED";


CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/";
CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${BUILD_NAME}/";
CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${BUILD_NAME}/${BUILD_CONFIGURATION}-iphoneos/";

LogOutput "\n\n\nTheKing--> Creating Library for ${BUILD_CONFIGURATION}-iphoneos\n\n\n";

xcodebuild 	-project "${PROJECT_DIRECTORY}/AudioVideoEngine.xcodeproj" \
			-scheme AudioVideoEngine \
			-configuration "${BUILD_CONFIGURATION}" \
			-sdk iphoneos${XCODE_SDK_VERSION}  \
			ARCHS="armv7 arm64" \
			CONFIGURATION_BUILD_DIR="$BUILD_DIRECTORY/${BUILD_NAME}/${BUILD_CONFIGURATION}-iphoneos/" \
			GCC_PREPROCESSOR_DEFINITIONS="" \
			ONLY_ACTIVE_ARCH=NO || exit 1;

CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${BUILD_NAME}/${BUILD_CONFIGURATION}-iphonesimulator/";

LogOutput "\n\n\nTheKing--> Creating Library for ${BUILD_CONFIGURATION}-iphonesimulator\n\n\n";

xcodebuild 	-project "$PROJECT_DIRECTORY/AudioVideoEngine.xcodeproj" \
			-scheme AudioVideoEngine \
			-configuration "${BUILD_CONFIGURATION}" \
			-sdk iphonesimulator${XCODE_SDK_VERSION}  \
			ARCHS="i386 x86_64 " \
			CONFIGURATION_BUILD_DIR="$BUILD_DIRECTORY/${BUILD_NAME}/${BUILD_CONFIGURATION}-iphonesimulator/" \
			GCC_PREPROCESSOR_DEFINITIONS="" \
			ONLY_ACTIVE_ARCH=NO || exit 1;

CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${BUILD_NAME}/${BUILD_CONFIGURATION}-universal/";

LogOutput "\n\n\nTheKing--> Merging Library for ${BUILD_CONFIGURATION}-Universal\n\n\n";

lipo -create 	"$BUILD_DIRECTORY/${BUILD_NAME}/${BUILD_CONFIGURATION}-iphoneos/libAudioVideoEngine.a" \
				"$BUILD_DIRECTORY/${BUILD_NAME}/${BUILD_CONFIGURATION}-iphonesimulator/libAudioVideoEngine.a" \
			 	-output "$BUILD_DIRECTORY/${BUILD_NAME}/${BUILD_CONFIGURATION}-universal/libAudioVideoEngine.a";


CreateDirectoryIfNotExist "${BUILD_DIRECTORY}/${BUILD_NAME}/include/";

LogOutput "\n\n\nTheKing--> Copy Header files related to libAudioVideoEngine..\n\n\n";

cp "$PROJECT_DIRECTORY/../../sources/common/InterfaceOfAudioVideoEngine.h" "$BUILD_DIRECTORY/${BUILD_NAME}/include/InterfaceOfAudioVideoEngine.h";

LogOutput "TheKing--> AudioVideoEngine BuildScript Finished Successfully...";



