mkdir build
del build\*.* /Q
cl /Wall /c src/*.c /Fobuild\
cl test/*.cpp build/*.obj /Fobuild\ /Fetest
test.exe
del test.exe test.db build\*.* /Q
