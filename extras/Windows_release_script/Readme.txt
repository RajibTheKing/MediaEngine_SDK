Follow the instruction written below to make release build.

1. Build Windows Desktop using Visual Studio or script for both configurations(Debug and Release).
2. Build Windows Phone using Visual Studio or script for both configurations(Debug and Release).
3. Make sure output directory of Visual Studio is default.
4. Set MediaDirectory of WindowsReleaseScript.py by mediaEngine directory of your work station. 
5. Run WindowsReleaseScript.py and choose your option.
6. See "WindowsReleaseBuilds" folder for output in same directory of WindowsReleaseScript.py.
7. WindowsReleaseScript.py rename following three folders with date-time suffix (FolderName -> FolderName_YYYY-MM-DD_HH-MM-SS) if they exists in same 
	directory of itself.
	
	a. WindowsReleaseBuilds				
	b. desktop
	c. windowsPhone
	
	Example: After rename "desktop" folder will be "desktop_YYYY-MM-DD_HH-MM-SS"
	
8. Make sure none of the three directories (or subdirectories of any of them) given above are open in any window before running the script. 
   Since we are going to rename the directory.