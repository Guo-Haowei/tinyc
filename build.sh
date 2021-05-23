#!/usr/bin/bash
gcc -E -DNOT_DEVELOPMENT c-source.c > c.c || exit 1
sed '/^$/d' c.c | sed '/^#/d' | cat > c.c

gcc -m32 -Wno-int-conversion -Wno-format -Wno-sign-compare -Wno-builtin-declaration-mismatch -Wno-implicit-function-declaration c.c -o c || exit 1