#!/usr/bin/bash

println() {
    echo "================================================================================"
}

if [ ! -d tmp ]; then
    mkdir tmp || exit 1
fi

cd tmp || exit 1

gcc -Wno-sign-compare -Wno-implicit-function-declaration -D_TEST ../cc.c -o cc > out || exit 1
clear

testcase() {
    file=$1

    name=$(basename $file)
    name="${name%.*}"

    echo "Running test [$name]..."

    gcc $file || exit 1
    ./a > $name.expect || exit 1
    ./cc $file > $name.actual || exit 1

    diff $name.expect $name.actual
    if [ $? != 0 ]
    then
        echo -e "\e[31mTest [$name] failed.\e[0m"
    else
        echo -e "\e[32mTest [$name] passed.\e[0m"
    fi
}

for file in ../test/*.c;
do
    testcase $file
done

exit 0