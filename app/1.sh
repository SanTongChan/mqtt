#!/usr/bin/env bash

# Fatal One !
make clean

make COMPILE=gcc BOOT=new APP=1 SPI_SPEED=40 SPI_MODE=QIO SPI_SIZE_MAP=2

if [ $? -eq 0 ] ; then
	ls -lh ../bin/upgrade | grep .bin
	ls -l ../bin/upgrade | grep .bin
	echo "Successfully!"
	exit 0
else
	echo "Something wrong!"
fi

