CFLAGS=-std=c17 -g -fno-common
CPPFLAGS=-std=c++17 -g -fno-common

GCC_CXXFLAGS = -DMESSAGE='"Compiled with GCC"'
CLANG_CXXFLAGS = -DMESSAGE='"Compiled with Clang"'
UNKNOWN_CXXFLAGS = -DMESSAGE='"Compiled with an unknown compiler"'

ifeq ($(CXX),g++)
	CXXFLAGS += $(GCC_CXXFLAGS)
else ifeq ($(CXX),clang++)
	CXXFLAGS += $(CLANG_CXXFLAGS)
else
	CXXFLAGS += $(UNKNOWN_CXXFLAGS)
endif


SRCS=$(wildcard *.cpp)
OBJS=$(SRCS:.cpp=.o)

TEST_SRCS=$(wildcard test/*.c)
TESTS=$(TEST_SRCS:.c=.exe)

IRTEST_SRCS=$(wildcard ir/*.c)
IRTESTS=$(IRTEST_SRCS:.c=.exe)

chibicc: $(OBJS)
	$(CXX) -o chibicc $(CPPFLAGS) $(OBJS) $(LDFLAGS)	

$(OBJS): chibicc.h


ir: $(IRTEST_SRCS)
	for i in $^; do ./chibicc $$i; cat ./ir/tmp.ll ; done

ir/%.exe: chibicc ir/%.c


test/%.exe: chibicc test/%.c
		$(CC) -o- -E -P -C test/$*.c | ./chibicc -o test/$*.s -
		$(CC) -o $@ test/$*.s -xc test/common

test: $(TESTS)
		for i in $^; do echo $$i; ./$$i || exit 1; echo; done	
		test/driver.sh
	

clean:
		rm -f chibicc tmp*  $(TESTS) test/*.s test/*.exe
		find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'

.PHONU: test clean
