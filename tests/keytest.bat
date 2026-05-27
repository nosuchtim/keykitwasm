@echo off

if "%KEYTEST_EXE%"=="" set KEYTEST_EXE=..\src\key.exe

echo Running stringstress test ...
"%KEYTEST_EXE%" stringstress.k > stringstress.out
diff -b stringstress.out stringstress.sav
