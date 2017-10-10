import os
import subprocess
import shutil
import sys
import time
import datetime
import zipfile



#################################################
#												#
#		JUST CHANGE MEDIA-ENGINE DIRECTORY		#
#												#
#################################################



MediaDirectory = r'D:\Current\call-in-live\app\src\main\jni\videoengine'





OutputDirectory = 'WindowsReleaseBuilds'
PlatformList = ['desktop', 'windowsPhone']


def copyFile(fromFile, toFile):
	shutil.copy2( fromFile, toFile)
	print 'Copying...   ', fromFile, '  ->  ', toFile

def getGitInfo(mediaDir):
	GitDirectory = MediaDirectory + '\.git'

	branchFile = open(GitDirectory+'\\HEAD','r')
	shaFile = open(GitDirectory+'\\logs\\HEAD','r')
	
	branch  = branchFile.readline()
	branch = branch.replace('/',' ')
	branch = branch.split()
	branch = branch[len(branch) - 1]
	
	print '\t\tBranch Name: ', branch
	
	lastLine = ''
	for line in shaFile.readlines():
		lastLine = line

	#print lastLine
	sha = lastLine.split()[1][0:8]
	print '\t\tSHA: ', sha

		
	branchFile.close() 
	shaFile.close()
	
	return branch+'.'+sha

def getTimeWithSecond():
	currentTime = datetime.datetime.now()	
	strTime = currentTime.strftime("%Y-%m-%d_%H-%M-%S")	
	return strTime

	
def renameWithTime(folderName):	
	message = 'The file "' + folderName + '" does not exist.'
	
	if os.path.exists(os.getcwd() +'\\'+ folderName):
		newName = folderName+'_'+getTimeWithSecond()	
		message = 'Renamed file "' + folderName + '". New name: "' + newName+ '".'				
		os.rename(folderName, newName)	
	
	print message
	
	
#@mediaDir : Media directory for getting branch name and SHA of git.	
def getZipFileName(mediaDir):
	gitInfo = getGitInfo(mediaDir)	
	#print gitInfo

	releaseType = 'release'
	teamPrefix = 'media'

	currentTime = datetime.datetime.now()
	buildTime = currentTime.strftime("%Y.%m.%d.%H.%M")
	#buildTime = currentTime.strftime("%Y.%m.%d.%H.%M.%S")
	
	#print buildTime

	zipFileName = teamPrefix + '.' + buildTime + '.' + gitInfo + '.' +releaseType +'.zip'
	print '\t\tZipFileName: ', zipFileName, '\n\n'

	return zipFileName

	
def copyLibs(thardPartyDirectory, dest, archi, platform):
	lib_list = os.listdir(thardPartyDirectory);
	
	for thLibName in lib_list:
		#curFrom : Platform Path
		curFrom = thardPartyDirectory + '\\' + thLibName + r'\libs\\'+ platform 
		#print curFrom
		print 'Thid party lib name: ', thLibName
		
		if os.path.exists(curFrom):
			curArchiList = os.listdir(curFrom)
			
			for curArchName in curArchiList:
				if archi !='all' and curArchName != archi:
					continue 
					
				fromArchPath = curFrom + '\\' + curArchName
				destArchPath = dest + '\\' + curArchName
				
				#print '\t\t-> ', fromArchPath, ' -> ', destArchPath
				
				
				if os.path.exists(fromArchPath) and os.path.exists(destArchPath):
				
					for curFile in os.listdir(fromArchPath):
						copyFile(fromArchPath+'\\'+curFile, destArchPath+'\\'+curFile)
						
		else :
			print '\t\t  Not Exists!!!!!!'	

def makeZipFile(zipFileName, sourceDirectory):			
	zf = zipfile.ZipFile(zipFileName, "w")
	for dirname, subdirs, files in os.walk(sourceDirectory):
		zf.write(dirname)
		for filename in files:
			zf.write(os.path.join(dirname, filename))
	zf.close()
			
	
def makeDesktopBuild(zipFileName, tmpFileName):	
	print '\n\n\t----- Preparing WindowsDesktop Build -----\n\n'
	
	renameWithTime(tmpFileName)
	
	os.makedirs(tmpFileName)	
	
	os.makedirs(tmpFileName+'\\'+'include')	
	os.makedirs(tmpFileName+'\\'+'libs')	
	os.makedirs(tmpFileName+'\\'+'libs'+'\Release')	
	os.makedirs(tmpFileName+'\\'+'libs'+'\Debug')	
	
	mediaLibName = 'MediaEngine_Windows.lib'
	
	copyFile(MediaDirectory + r'\sources\common\InterfaceOfAudioVideoEngine.h', tmpFileName+r'\include\InterfaceOfAudioVideoEngine.h')
	copyFile(MediaDirectory + r'\output\desktop\Release\\' + mediaLibName, tmpFileName+'\libs\Release\\' + mediaLibName)
	copyFile(MediaDirectory + r'\output\desktop\Debug\\' + mediaLibName, tmpFileName+'\libs\Debug\\' + mediaLibName)
	
		
	copyLibs(MediaDirectory+ '\\third_party', tmpFileName+'\\'+'libs', 'all', 'desktop')
		
	makeZipFile(zipFileName, tmpFileName)
	
	os.makedirs(OutputDirectory+'\\'+'desktop')	
	shutil.move(zipFileName, OutputDirectory+'\\'+'desktop')
	
	shutil.rmtree(tmpFileName)


def makeWinPhoneBuild(zipFileName, tmpFileName):	
	print '\n\n\t----- Preparing WindowsPhone Build -----\n\n'
	renameWithTime(tmpFileName)

	os.makedirs(tmpFileName)	
	
	os.makedirs(tmpFileName+'\\'+'include')	
	os.makedirs(tmpFileName+'\\'+'libs')	
	os.makedirs(tmpFileName+'\\'+'libs'+'\Release')	
	os.makedirs(tmpFileName+'\\'+'libs'+'\Debug')	
	
	
	mediaLibName = 'MediaEngine_WindowsPhone.lib'
	
	copyFile(MediaDirectory + r'\sources\common\InterfaceOfAudioVideoEngine.h', tmpFileName+r'\include\InterfaceOfAudioVideoEngine.h')
	copyFile(MediaDirectory + r'\output\winphone\Release\\' + mediaLibName, tmpFileName+'\libs\Release\\' + mediaLibName)
	copyFile(MediaDirectory + r'\output\winphone\Debug\\' + mediaLibName, tmpFileName+'\libs\Debug\\' + mediaLibName)
	
		
	copyLibs(MediaDirectory+ '\\third_party', tmpFileName+'\\'+'libs', 'all', 'windowsPhone')
	
	makeZipFile(zipFileName, tmpFileName)
	
	os.makedirs(OutputDirectory+'\\'+'windowsPhone')	
	shutil.move(zipFileName, OutputDirectory+'\\'+'windowsPhone')
	
	shutil.rmtree(tmpFileName)	

	
print 'Choose an option given below.'
print '1. Desktop & WindowsPhone both.'
print '2. Desktop only.'
print '3. WindowsPhone only'

input = raw_input()
	
	
	
print '\n\n\n\n+++++++++++++++++++++++  Building... ++++++++++++++++++++++++++++\n'

ZipFileName = getZipFileName(MediaDirectory)	

renameWithTime(OutputDirectory)
os.makedirs(OutputDirectory)	

if input == '1' or input == '2' :
	makeDesktopBuild(ZipFileName, PlatformList[0])
	
if input == '1' or input == '3' :
	makeWinPhoneBuild(ZipFileName, PlatformList[1])

print "\n\t\tBUILD TIME : ",str(datetime.datetime.now()), " \n"
print '+++++++++++++++++++++++  Building Successful ++++++++++++++++++++++++++++\n\n'




