CFLAGS=-std=c17 -g -fno-common
STD=c++2a
CXX=g++
CPPFLAGS=-std=$(STD) -g -fno-common

GCC_CXXFLAGS = -DMESSAGE='"Compiled with G++"'
CLANG_CXXFLAGS = -DMESSAGE='"Compiled with Clang++"'
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

mclang: $(OBJS)
	$(CXX) -o mclang $(CPPFLAGS) $(OBJS) $(LDFLAGS)	

$(OBJS): mclang.h


ir: $(IRTEST_SRCS)
	for i in $^; do ./mclang $$i; cat ./ir/tmp.ll ; done

ir/%.exe: mclang ir/%.c


test/%.exe: mclang test/%.c
		$(CC) -o- -E -P -C test/$*.c | ./mclang -o test/$*.s -
		$(CC) -o $@ test/$*.s -xc test/common

test: $(TESTS)
		for i in $^; do echo $$i; ./$$i || exit 1; echo; done	
		test/driver.sh
	

clean:
		rm -f mclang tmp*  $(TESTS) test/*.s test/*.exe
		find * -type f '(' -name '*~' -o -name '*.o' ')' -exec rm {} ';'

.PHONU: test clean
