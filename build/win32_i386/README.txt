wzMRTD - An electronic passport reader library
Copyright (c) 2007, Johann Dantant - www.wzpass.net

Please read LICENSE.txt for complete informations about the license.


This is the project to build wzMRTD.dll and wzMRTD.exe for Win32/i386

The IDE used is Visual C++ 6

The workspace holds 2 projects :

- wzmrtd_dll produces the wzmrtd.dll itself
- wzmrtd_exe produces wzmrtd.exe, a self-standing executable

Both wzmrtd.exe and wzmrtd.dll use the "delay load import" feature
for a late dynamic linking to winscard.dll (PC/SC support library)
and springprox.dll (Pro-Active readers support library).
