#!/usr/bin/bash
gcc -Wall -Wextra -Wno-sign-compare -Wno-implicit-function-declaration c.c -o c || exit 1
# ./cc hello.c
# cmake -G "Visual Studio 16 2019" -A Win32 ..