CFLAGS=-Wall -std=c11 -g
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

cc: $(OBJS)
	cc -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): cc.h

# test: 9cc test/test.c
# 	./9cc -test

# 	@./9cc test/test.c > tmp-test1.s
# 	@gcc -c -o tmp-test2.o test/gcc.c
# 	@gcc -static -o tmp-test1 tmp-test1.s tmp-test2.o
# 	@./tmp-test1

# 	@./9cc test/token.c > tmp-test2.s
# 	@gcc -static -o tmp-test2 tmp-test2.s
# 	@./tmp-test2

format:
	clang-format -i *.c

clean:
	rm cc *.o *.out

unit:
	gcc -Wno-pointer-to-int-cast list.c debug.c unit/list.test.c -o list.out
	valgrind ./list.out

.PHONY: clean format unit
