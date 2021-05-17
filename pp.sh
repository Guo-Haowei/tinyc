#!/usr/bin/bash
# because wcc does not support preprocessor, use gcc to preprocess it first

gcc -E -DNOT_DEVELOPMENT cc.c > cc-min.c || exit 1

sed '/^$/d' cc-min.c | sed '/^#/d' | cat > cc-min.c

