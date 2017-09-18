import os
import subprocess
import shutil
import sys
import time


videoengine = r'D:\Dev\MediaEngine_dev'
callsdk = r'D:\Dev\callsdk_v2'
testapp = r'D:\Dev\ringid_clients\ringIDAndroid\ringID'
ownClient = r'D:\Dev\AndroidTestClient';




application_mk = videoengine + r'\builds\android\jni\Application.mk'

ret = 0

armeabi = 0
armeabi_v7a = 0
arm64_v8a = 0
x86 = 0

old = ''
flag = False
clean_videoEngine = 0

def check():
	global ret
	if ret != 0:
		print("##### ERROR OCCURED #####\n" * 10)
		print(ret)
		if flag == True:
			with open(application_mk, "r+") as output_file:
			    	output_file.truncate()
			   	output_file.write(old)
			   	output_file.close()
		exit()

def buildEngine():
	global ret
	os.chdir(videoengine + r'\builds\android\jni')
	if clean_videoEngine == 1: 
		ret = subprocess.call(["ndk-build","clean"], shell=True)
		check()
		ret = subprocess.call(["ndk-build"], shell=True)
	else: ret = subprocess.call(["ndk-build"], shell=True)
	check()
	if armeabi == 1: 
		shutil.copy2( videoengine + r'\output\android\armeabi\libvideoEngineController.a', callsdk + r'\libs\media\Android\libs\armeabi')
		shutil.copy2( videoengine + r'\output\android\armeabi\libRingIDSDK.so', ownClient + r'\app\src\main\jniLibs\armeabi')
	if armeabi_v7a == 1: 
		shutil.copy2( videoengine + r'\output\android\armeabi-v7a\libvideoEngineController.a', callsdk + r'\libs\media\Android\libs\armeabi-v7a')
		shutil.copy2( videoengine + r'\output\android\armeabi-v7a\libRingIDSDK.so', ownClient + r'\app\src\main\jniLibs\armeabi-v7a')
	if x86 == 1: 
		shutil.copy2( videoengine + r'\output\android\x86\libvideoEngineController.a', callsdk + r'\libs\media\Android\libs\x86')
		shutil.copy2( videoengine + r'\output\android\x86\libRingIDSDK.so', ownClient + r'\app\src\main\jniLibs\x86')
	if arm64_v8a == 1: 
		shutil.copy2( videoengine + r'\output\android\arm64-v8a\libvideoEngineController.a', callsdk + r'\libs\media\Android\libs\x86')
		shutil.copy2( videoengine + r'\output\android\arm64-v8a\libRingIDSDK.so', ownClient + r'\app\src\main\jniLibs\x86')
		
	shutil.copy2( videoengine + r'\\sources\common\InterfaceOfAudioVideoEngine.h', callsdk + r'\libs\media\Android\include')
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

clean_callsdk = 0
call_build = 0

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
	str = raw_input("Specify space separated architecture(/s) you want to build?  ")
	nums = str.split()
	cnt = 0
	for x in nums:
		if x == '1': 
			cnt = cnt + 1
			armeabi = 1
		elif x == '2':
			cnt = cnt + 1
			armeabi_v7a = 1
		elif x == '3':
			cnt = cnt + 1
			x86 = 1
		elif x == '4':
			cnt = cnt + 1
			arm64_v8a = 1
	if cnt == 0:
		print('You must choose at least armeabi architecture')
	else:
		break

architectures = 'APP_ABI ='
if armeabi == 1: architectures = architectures + ' armeabi'
if armeabi_v7a == 1: architectures = architectures + ' armeabi-v7a'
if x86 == 1: architectures = architectures + ' x86'
if arm64_v8a == 1: architectures = architectures + ' arm64-v8a'

with open(application_mk, "r+") as input_file:
    new = ''
    old = ''
    cnt = 0
    flag = True
    for line in input_file:
    	old = old + line
    	if line.find('APP_ABI') == -1:
    		new = new + line

    new = new + architectures + '\n'

    with open(application_mk, "r+") as output_file:
    	output_file.truncate()
    	output_file.write(new)
    	output_file.close()
    input_file.close()


buildEngine()

os.chdir(callsdk + r'\wrapper\android\jni')
os.utime(callsdk + r'\libs\media\Android\libs\armeabi-v7a\libvideoEngineController.a',(time.time(),time.time()))

if call_build == 1:
	if clean_callsdk == 1: 
		ret = subprocess.call(["ndk-build", "clean", architectures], shell=True)
		check()
		ret = subprocess.call(["ndk-build", architectures], shell=True)
	else: ret = subprocess.call(["ndk-build", architectures], shell=True)
	check()
	if clean_callsdk == 1: print('******************************CallSDK Clean Build successfull!***************************')
	else: print('******************************CallSDK Build successfull!***************************')

	if armeabi_v7a == 1: shutil.copy2( callsdk + r'\wrapper\android\libs\armeabi-v7a\libvoicechatsdk.so', testapp + r'\src\main\jniLibs\armeabi-v7a\libvoicechatsdk.so')
	if armeabi == 1: shutil.copy2( callsdk + r'\wrapper\android\libs\armeabi\libvoicechatsdk.so', testapp + r'\src\main\jniLibs\armeabi\libvoicechatsdk.so')
	if x86 == 1: shutil.copy2( callsdk + r'\wrapper\android\libs\x86\libvoicechatsdk.so', testapp + r'\src\main\jniLibs\x86\libvoicechatsdk.so')
	if arm64_v8a == 1: shutil.copy2( callsdk + r'\wrapper\android\libs\arm64-v8a\libvoicechatsdk.so', testapp + r'\src\main\jniLibs\arm64-v8a\libvoicechatsdk.so')

if flag == True:
	with open(application_mk, "r+") as output_file:
		output_file.truncate()
		output_file.write(old)
		output_file.close()

exit()