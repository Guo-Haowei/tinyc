CFLAGS=-Wall -Wextra -Wno-pointer-to-int-cast -std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc: $(OBJS)
	cc -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): cc.h token.inl

format:
	@clang-format -i *.h *.c unit/*.cpp

clean:
	@rm cc *.o

unit:
	sh run-unit-test.sh

.PHONY: clean format unit
