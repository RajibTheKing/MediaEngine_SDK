import os
import subprocess
import shutil
import sys
import time


callsdk = r'D:\Dev\callsdk_v2'
mediaEngine = r'D:\Dev\MediaEngine_dev';
ringidSDK = r'D:\Dev\RingIDSDK'
ringidDesktop = r'D:\Dev\ringid_clients\ringID_WPF'
libraryPath = r'E:\OnlyForDesktop\123\desktop'



ret = 0
clean_videoEngine = 0
debugOrRelease = 0 # 0 -> debug, 1-> release
clean_callsdk = 0
call_build = 0
ringid_build = 0
libraryRelease = 0
log_enabled = 0

def check():
	global ret
	if ret != 0:
		print("##### ERROR OCCURED #####\n" * 10)
		print(ret)
		exit()

def buildEngine():
	global ret
	os.chdir(mediaEngine + r'\builds\desktop')
	if log_enabled == 1:
		if debugOrRelease == 0: 
			ret = subprocess.call(["msbuild.exe", "MediaEngine_Windows.sln", "/t:Rebuild", "/p:DefineConstants=LOG_ENABLED", "/p:configuration=debug"], shell=True)
			check()
		else: 
			ret = subprocess.call(["msbuild.exe", "MediaEngine_Windows.sln", "/t:Rebuild", "/p:DefineConstants=LOG_ENABLED", "/p:configuration=release"], shell=True)
			check()
	else:
		if clean_videoEngine == 1: 
			if debugOrRelease == 0: 
				ret = subprocess.call(["msbuild.exe", "MediaEngine_Windows.sln", "/t:Clean", "/p:configuration=debug"], shell=True)
				check()
			else: 
				ret = subprocess.call(["msbuild.exe", "MediaEngine_Windows.sln", "/t:Clean", "/p:configuration=release"], shell=True)
				check()
		if debugOrRelease == 0: 
			ret = subprocess.call(["msbuild.exe", "MediaEngine_Windows.sln", "/p:configuration=debug"], shell=True)
			check()
		else: 
			ret = subprocess.call(["msbuild.exe", "MediaEngine_Windows.sln", "/p:configuration=release"], shell=True)
			check()
	if debugOrRelease == 0: 
		shutil.copy2( mediaEngine + r'\output\desktop\debug\MediaEngine_Windows.lib', callsdk + r'\libs\media\desktop\libs\Debug')
	else: 
		shutil.copy2( mediaEngine + r'\output\desktop\release\MediaEngine_Windows.lib', callsdk + r'\libs\media\desktop\libs\Release')
	shutil.copy2( mediaEngine + r'\sources\common\InterfaceOfAudioVideoEngine.h', callsdk + r'\libs\media\desktop\include')
	print('******************************Copy successfull!***************************')

def buildCallSDK():
	global ret
	if clean_callsdk == 1:
		os.chdir(callsdk + r'\wrapper\desktop\VoiceChatSDK')
		if debugOrRelease == 0: 
			ret = subprocess.call(["msbuild.exe", "VoiceChatSDK.vcxproj", "/t:Clean", "/p:configuration=debug"], shell=True)
			check()
		else: 
			ret = subprocess.call(["msbuild.exe", "VoiceChatSDK.vcxproj", "/t:Clean", "/p:configuration=release"], shell=True)
			check()
		
		os.chdir(callsdk + r'\wrapper\desktop\VoiceChatParser')
		if debugOrRelease == 0: 
			ret = subprocess.call(["msbuild.exe", "VoiceChatParser.vcxproj", "/t:Clean", "/p:configuration=debug"], shell=True)
			check()
		else:
			ret = subprocess.call(["msbuild.exe", "VoiceChatParser.vcxproj", "/t:Clean", "/p:configuration=release"], shell=True)
			check()
		
		os.chdir(callsdk + r'\wrapper\desktop\CallSDK')
		if debugOrRelease == 0: 
			ret = subprocess.call(["msbuild.exe", "CallSDK.vcxproj", "/t:Clean", "/p:configuration=debug"], shell=True)
			check()
		else: 
			ret = subprocess.call(["msbuild.exe", "CallSDK.vcxproj", "/t:Clean", "/p:configuration=release"], shell=True)
			check()
		
	os.chdir(callsdk + r'\wrapper\desktop\VoiceChatSDK')
	if debugOrRelease == 0: 
		ret = subprocess.call(["msbuild.exe", "VoiceChatSDK.vcxproj", "/p:configuration=debug"], shell=True)
		check()
		shutil.copy2( callsdk + r'\wrapper\desktop\VoiceChatSDK\Debug\VoiceChatSDK.lib', callsdk + r'\wrapper\desktop\Debug')
	else: 
		ret = subprocess.call(["msbuild.exe", "VoiceChatSDK.vcxproj", "/p:configuration=release"], shell=True)
		check()
		shutil.copy2( callsdk + r'\wrapper\desktop\VoiceChatSDK\Release\VoiceChatSDK.lib', callsdk + r'\wrapper\desktop\Release')
	
	os.chdir(callsdk + r'\wrapper\desktop\VoiceChatParser')
	if debugOrRelease == 0: 
		ret = subprocess.call(["msbuild.exe", "VoiceChatParser.vcxproj", "/p:configuration=debug"], shell=True)
		check()
		shutil.copy2( callsdk + r'\wrapper\desktop\VoiceChatParser\Debug\VoiceChatParser.lib', callsdk + r'\wrapper\desktop\Debug')
	else: 
		ret = subprocess.call(["msbuild.exe", "VoiceChatParser.vcxproj", "/p:configuration=release"], shell=True)
		check()
		shutil.copy2( callsdk + r'\wrapper\desktop\VoiceChatParser\Release\VoiceChatParser.lib', callsdk + r'\wrapper\desktop\Release')
	
	os.chdir(callsdk + r'\wrapper\desktop\CallSDK')
	if debugOrRelease == 0: 
		ret = subprocess.call(["msbuild.exe", "CallSDK.vcxproj", "/p:configuration=debug"], shell=True)
		check()
	else: 
		ret = subprocess.call(["msbuild.exe", "CallSDK.vcxproj", "/p:configuration=release"], shell=True)
		check()

	if debugOrRelease == 0: shutil.copy2( callsdk + r'\wrapper\desktop\CallSDK\Debug\CallSDK.dll', ringidDesktop + r'\dlls')
	else : shutil.copy2( callsdk + r'\wrapper\desktop\CallSDK\Release\CallSDK.dll', ringidDesktop + r'\dlls')
	
def buildRingID():
	global ret
	os.chdir(ringidDesktop)
	ret = subprocess.call(["msbuild.exe", "ringID.sln", "/t:Clean", "/p:configuration=debug"], shell=True)
	check()
		
	ret = subprocess.call(["msbuild.exe", "ringID.sln", "/p:configuration=debug"], shell=True)
	check()
	
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
	str = raw_input("Want to release mediaengine? (Y/n)   ")
	if str[0] == 'y' or str[0] == 'Y':
		libraryRelease = 1
		break
	elif str[0] == 'n' or str[0] == 'N':
		libraryRelease = 0
		break
		
while True:
	str = raw_input("Want to enable log? (Y/n)   ")
	if str[0] == 'y' or str[0] == 'Y':
		log_enabled = 1
		break
	elif str[0] == 'n' or str[0] == 'N':
		log_enabled = 0
		while True:
			str = raw_input("Enter 'd' for debug build or 'r' for release build... (d/r)  ")
			if len(str) == 1:
				if str[0] == 'd' or str[0] == 'D':
					debugOrRelease = 0
					break
				elif str[0] == 'r' or str[0] == 'R':
					debugOrRelease = 1
					break
		break

while True:
	ins = raw_input("Want to give callsdk build? (Y/n)  ")
	if len(ins) == 1:
		if ins[0] == 'Y' or ins[0] == 'y':
			call_build = 1
			while True:
				str = raw_input("Want to clean build callsdk? (Y/n)  ")
				if len(str) == 1:
					if str[0] == 'y' or str[0] == 'Y':
						clean_callsdk = 1
						break
					elif str[0] == 'n' or str[0] == 'N':
						clean_callsdk = 0
						break
			break

		elif ins[0] == 'N' or ins[0] == 'n':
			call_build = 0
			break

while True:
	ins = raw_input("Want to give ringid build? (Y/n)  ")
	if len(ins) == 1:
		if ins[0] == 'Y' or ins[0] == 'y':
			ringid_build = 1
			break

		elif ins[0] == 'N' or ins[0] == 'n':
			ringid_build = 0
			break

dr = debugOrRelease

if libraryRelease == 1:
	debugOrRelease = 0
	buildEngine()
	debugOrRelease = 1
	buildEngine()
	shutil.copy2( mediaEngine + r'\output\desktop\debug\MediaEngine_Windows.lib', libraryPath + r'\libs\Debug')
	shutil.copy2( mediaEngine + r'\output\desktop\Release\MediaEngine_Windows.lib', libraryPath + r'\libs\Release')
	shutil.copy2( mediaEngine + r'\sources\common\InterfaceOfAudioVideoEngine.h', libraryPath + r'\include')

else:
	buildEngine()
	
debugOrRelease = dr
	
if call_build == 1: buildCallSDK()

if ringid_build == 1: buildRingID()

exit()