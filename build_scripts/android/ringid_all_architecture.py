import os
import subprocess
import shutil
import sys
import time


videoengine = r'F:\git_Codes\AndroidTestClientVE_FTest\AndroidTestClientVE_FTest\app\src\main\jni\videoengine'
callsdk = r'F:\git_Codes\callsdk_v2'
testapp = r'F:\git_Codes\ringIDAndroid\ringID'
ownClient = r'F:\git_Codes\AndroidTestClientVE_FTest\AndroidTestClientVE_FTest';




application_mk = videoengine + r'\OthersLib\JNI\Application.mk'

ret = 0
three = 0
two = 0
one = 0
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
	os.chdir(videoengine + r'\OthersLib\JNI')
	if clean_videoEngine == 1: 
		ret = subprocess.call(["ndk-build","clean"], shell=True)
		check()
		ret = subprocess.call(["ndk-build"], shell=True)
	else: ret = subprocess.call(["ndk-build"], shell=True)
	check()
	if one == 1: 
		shutil.copy2( videoengine + r'\OthersLib\obj\local\armeabi\libvideoEngineController.a', callsdk + r'\libs\media\Android\libs\armeabi')
		shutil.copy2( videoengine + r'\OthersLib\libs\armeabi\libRingIDSDK.so', ownClient + r'\app\src\main\jniLibs\armeabi')
	if two == 1: 
		shutil.copy2( videoengine + r'\OthersLib\obj\local\armeabi-v7a\libvideoEngineController.a', callsdk + r'\libs\media\Android\libs\armeabi-v7a')
		shutil.copy2( videoengine + r'\OthersLib\libs\armeabi-v7a\libRingIDSDK.so', ownClient + r'\app\src\main\jniLibs\armeabi-v7a')
	if three == 1: 
		shutil.copy2( videoengine + r'\OthersLib\obj\local\x86\libvideoEngineController.a', callsdk + r'\libs\media\Android\libs\x86')
		shutil.copy2( videoengine + r'\OthersLib\libs\x86\libRingIDSDK.so', ownClient + r'\app\src\main\jniLibs\x86')
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
			one = 1
		elif x == '2':
			cnt = cnt + 1
			two = 1
		elif x == '3':
			cnt = cnt + 1
			three = 1
	if cnt == 0:
		print('You must choose at least one architecture')
	else:
		break

architectures = 'APP_ABI ='
if one == 1: architectures = architectures + ' armeabi'
if two == 1: architectures = architectures + ' armeabi-v7a'
if three == 1: architectures = architectures + ' x86'

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

	if two == 1: shutil.copy2( callsdk + r'\wrapper\android\libs\armeabi-v7a\libvoicechatsdk.so', testapp + r'\src\main\jniLibs\armeabi-v7a\libvoicechatsdk.so')
	if one == 1: shutil.copy2( callsdk + r'\wrapper\android\libs\armeabi\libvoicechatsdk.so', testapp + r'\src\main\jniLibs\armeabi\libvoicechatsdk.so')
	if three == 1: shutil.copy2( callsdk + r'\wrapper\android\libs\x86\libvoicechatsdk.so', testapp + r'\src\main\jniLibs\x86\libvoicechatsdk.so')

if flag == True:
	with open(application_mk, "r+") as output_file:
		output_file.truncate()
		output_file.write(old)
		output_file.close()

exit()