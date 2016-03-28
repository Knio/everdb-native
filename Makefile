
edb: src/*.c
	gcc $+

test_exe: test/test_edb.cpp
	g++ $+ -o test_exe


.PHONY: test
test: test_exe
	./test_exe

