#!/bin/bash
var1=$1 # testcase
var2=$2 # options

if [ ! -d build ]; then
    mkdir build
fi

if [ ! -d tmp ]; then
    mkdir tmp
fi

cd build || exit 1
cmake .. || exit 1
cmake --build . || exit 1
cd ../tmp || exit 1

testcase() {
    file=$1

    name=$(basename $file)
    name="${name%.*}"

    echo "================================================================================"
    echo "Running test [$name]..."

    if [ "$var2" != "" ]
    then
        ../build/bin/Debug/toy_c "$var2" $file || exit 1
    else
        ../build/bin/Debug/toy_c $file || exit 1
    fi

    nasm -fwin32 $name.asm || exit 1
    gcc $name.obj -o $name.act || exit 1
    ./$name.act > $name.act.txt
    rm $name.act $name.obj

    gcc -Wno-parentheses -Wno-overflow $file -o $name.exp || exit 1
    ./$name.exp > $name.exp.txt
    rm $name.exp || exit 1

    diff $name.exp.txt $name.act.txt
    if [ $? != 0 ]
    then
        echo -e "\e[31mTest [$name] failed.\e[0m"
    else
        echo -e "\e[32mTest [$name] passed.\e[0m"
    fi
    echo "================================================================================"
}

if [ "$var1" != "" ]
then
    # working directory is tmp
    testcase ../$var1
    exit 0
fi

# echo "********************************************************************************"
# echo "UNIT TEST"
# echo "********************************************************************************"
# ./bin/Debug/dict-test.exe || exit 1
# ./bin/Debug/list-test.exe || exit 1

echo "********************************************************************************"
echo "FAILED TEST"
echo "********************************************************************************"
for file in ../test/invalid/*.c;
do
    echo $(basename $file)
    ../build/bin/Debug/toy_c $file && exit 1
done

echo "********************************************************************************"
echo "UNIT TEST"
echo "********************************************************************************"
for file in ../test/compiler/*.c;
do
    testcase $file
done

echo "********************************************************************************"
echo "LEETCODE"
echo "********************************************************************************"
for file in ../test/leetcode/*.c;
do
    testcase $file
done
