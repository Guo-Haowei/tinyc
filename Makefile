CFLAGS=-Wall -std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc: $(OBJS)
	cc -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): cc.h

format:
	clang-format -i *.h *.c unit/*

clean:
	rm cc *.o *.out

### TODO: clean up .o
unit:
	@echo "compiling unit test [list]"
	@gcc -Wno-pointer-to-int-cast list.c debug.c unit/list.test.c -o list.out
	@valgrind ./list.out
	@echo "compiling unit test [string_util]"
	@gcc preproc.c token.c list.c error.c filecache.c arena.c lexer.c string_util.c debug.c unit/string_util.test.c -o string_util.out
	@./string_util.out

.PHONY: clean format unit
