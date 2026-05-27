KEYTEST_EXE=${KEYTEST_EXE:-../src/key.exe}

echo Running stringstress test ...
"$KEYTEST_EXE" stringstress.k > stringstress.out
diff stringstress.out stringstress.sav
