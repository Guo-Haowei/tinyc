CFLAGS=-Wall -Wextra -std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc: $(OBJS)
	cc -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): cc.h

format:
	@clang-format -i *.h *.c unit/*.cpp

clean:
	@rm cc *.o

unit:
	sh run-unit-test.sh

.PHONY: clean format unit
