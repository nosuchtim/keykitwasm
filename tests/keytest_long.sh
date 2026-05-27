KEYTEST_EXE=${KEYTEST_EXE:-../src/key.exe}

echo Running stringsoak test ...
"$KEYTEST_EXE" stringsoak.k > stringsoak.out
diff stringsoak.out stringsoak.sav
