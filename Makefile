CFLAGS=-std=c++20 -g -fno-common

SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)


chibicc: $(OBJS)
		$(CXX) -o chibicc $(OBJS) $(LDFLAGS)	

$(OBJS): chibicc.h

test/%.exe: chibicc test/%.c
		$(CXX) -o- -E -P -C test/$*.c | ./chibicc -o test/$*.s -
		$(CXX) -o $@ test/$*.s -xc test/common

test: $(TESTS)
		for i in $^; do echo $$i; ./$$i || exit 1; echo; done	
		test/driver.sh
	

clean:
		rm -f chibicc tmp*  $(TESTS) test/*.s test/*.exe
		find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'

.PHONU: test clean
