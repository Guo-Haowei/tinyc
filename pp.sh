#!/usr/bin/bash
# because wcc does not support preprocessor, use gcc to preprocess it first

gcc -E -DNOT_DEVELOPMENT c.c > c-min.c || exit 1

sed '/^$/d' c-min.c | sed '/^#/d' | cat > c-min.c

