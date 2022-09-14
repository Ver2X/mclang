#!/bin/bash
tmp=`mktemp -d /tmp/mclang-test-XXXXX`
trap 'rm -rf $tmp' INT TERM HUP EXIT
echo > $tmp/empty.c


check()
{
	if [ $? -eq 0 ]; then
		echo "testing $1 ... passed"
	else
		echo "testing $1 ... failed"
		exit 1
	fi
}


# -o
rm -rf $tmp/out
./build/mclang -o $tmp/out $tmp/empty.c
[ -f $tmp/out ]
check -o

# --help
./build/mclang --help 2>&1 | grep -q mclang
check --help

echo OK
