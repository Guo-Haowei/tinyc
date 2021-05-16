#!/usr/bin/bash
gcc -Wall -Wextra -Wno-sign-compare -Wno-implicit-function-declaration cc.c -o cc || exit 1
# ./cc hello.c
# cmake -G "Visual Studio 16 2019 Win32" ..
