mkdir build
del build\*.* /Q
cl /Wall /c src/*.c /Fobuild\
cl test/test_edb.cpp build/*.obj /Fobuild\
test_edb.exe
del test_edb.exe test.db build\*.* /Q
