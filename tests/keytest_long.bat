@echo off

if "%KEYTEST_EXE%"=="" set KEYTEST_EXE=..\src\key.exe

echo Running stringsoak test ...
"%KEYTEST_EXE%" stringsoak.k > stringsoak.out
diff -b stringsoak.out stringsoak.sav
