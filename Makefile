C_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

libeverdb.a: $(OBJ_FILES)
	ar rcs libeverdb.a libeverdb.a $^

obj/%.o: src/%.c
	gcc -c -fms-extensions -g -Wall -o $@ $<

clean:
	rm libeverdb.a
	rm obj/*
	rm test_exe
	rm test.db

test_exe: test/test_edb.cpp
	g++ -fms-extensions -fpermissive -g -Wall $+ libeverdb.a -o test_exe


.PHONY: test
test: test_exe
	./test_exe

