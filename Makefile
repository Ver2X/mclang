CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

chibicc: $(OBJS)
		$(CC) -o chibicc $(OBJS) $(LDFLAGS)	

$(OBJS): chibicc.h

test: chibicc
		./test.sh 
		./test-driver.sh
	

clean:
		rm -f chibicc *.o *~ tmp*

.PHONU: test clean
