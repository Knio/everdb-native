C_FILES := $(wildcard src/*.c)
OBJ_FILES := $(addprefix obj/,$(notdir $(C_FILES:.c=.o)))

CC=gcc
CXX=g++
AR=ar
CFLAGS=-fms-extensions
CXXFLAGS=-fpermissive $(CFLAGS)

DEBUG ?= 0
ifeq ($(DEBUG), 1)
	CFLAGS += -g -Wall
endif

libeverdb.a: $(OBJ_FILES)
	$(AR) rcs $@ $^

obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

all: libeverdb.a

.PHONY: coverage
coverage: CFLAGS += --coverage
coverage: clean test
	lcov --capture --directory . --output-file coverage.info
	lcov --remove coverage.info 'test_*' '/usr/*' 'lib/*' --output-file coverage.info
	genhtml coverage.info --output-directory covout
	@echo "======================================"
	@echo "Open covout/index.html to view results"

test_exe: test/test_edb.cpp libeverdb.a
	$(CXX) $(CXXFLAGS) $+ -L. -leverdb -o test_exe

.PHONY: test
test: test_exe
	./test_exe

.PHONY: clean
clean:
	rm -rf *.a obj/* test_exe test.db *.gcda *.gcno covout coverage.info
