mkdir "..\obj\local-copy"

mkdir "..\obj\local-copy\armeabi-v7a"
mkdir "..\obj\local-copy\armeabi"
mkdir "..\obj\local-copy\x86"

copy "..\obj\local\armeabi-v7a\libvideoEngineController.a" "..\obj\local-copy\armeabi-v7a\libvideoEngineController.a"
copy "..\obj\local\armeabi\libvideoEngineController.a" "..\obj\local-copy\armeabi\libvideoEngineController.a"
copy "..\obj\local\x86\libvideoEngineController.a" "..\obj\local-copy\x86\libvideoEngineController.a"