import os
import subprocess
import shutil
import sys
import time


videoengine = r'F:\git_Codes\AndroidTestClientVE_FTest\AndroidTestClientVE_FTest\app\src\main\jni\videoengine'
testCameraWindowsPhone = r'F:\git_Codes\AndroidTestClientVE_FTest\AndroidTestClientVE_FTest\app\src\main\jni\TestCamera_WindowsPhone'
projectSocial = r'F:\git_Codes\ProjectSocial';


ret = 0
clean_videoEngine = 0
log_enabled = 0

def check():
	global ret
	if ret != 0:
		print("##### ERROR OCCURED #####\n" * 10)
		print(ret)
		exit()

def buildEngine():
	global ret
	os.chdir(videoengine + r'\output\winphone')
	if clean_videoEngine == 1: 
		ret = subprocess.call(["msbuild","MediaEngine_WindowsPhone.sln", "/t:Clean", "/p:configuration=debug"], shell=True)
		check()
		ret = subprocess.call(["msbuild","MediaEngine_WindowsPhone.sln", "/t:Clean", "/p:configuration=release"], shell=True)
		check()

	if log_enabled == 0: ret = subprocess.call(["msbuild","MediaEngine_WindowsPhone.sln", "/p:configuration=debug"], shell=True)
	else: ret = subprocess.call(["msbuild","MediaEngine_WindowsPhone.sln", "/p:configuration=debug", "/p:DefineConstants=LOG_ENABLED"], shell=True)
	check()
	if log_enabled == 0: ret = subprocess.call(["msbuild","MediaEngine_WindowsPhone.sln", "/p:configuration=release"], shell=True)
	else: ret = subprocess.call(["msbuild","MediaEngine_WindowsPhone.sln", "/p:configuration=release", "/p:DefineConstants=LOG_ENABLED"], shell=True)
	check()
	
	shutil.copy2( testCameraWindowsPhone + r'\Windows_Phone_Libs\MediaEngine_WindowsPhone.lib', projectSocial + r'\BackEnd\CallSdkWindowsPhone\libs\_Debug')
	shutil.copy2( testCameraWindowsPhone + r'\Windows_Phone_Libs\Release\MediaEngine_WindowsPhone.lib', projectSocial + r'\BackEnd\CallSdkWindowsPhone\libs\_Release')
	print('******************************Copy successfull!***************************')

ln = 60

while True:
	str = raw_input("Want to clean build videoEngine? (Y/n)   ")
	if str[0] == 'y' or str[0] == 'Y':
		clean_videoEngine = 1
		break
	elif str[0] == 'n' or str[0] == 'N':
		clean_videoEngine = 0
		break


while True:
	str = raw_input("Want to enable Log? (Y/n)   ")
	if str[0] == 'y' or str[0] == 'Y':
		log_enabled = 1
		break
	elif str[0] == 'n' or str[0] == 'N':
		log_enabled = 0
		break

buildEngine()

exit()