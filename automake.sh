#!/bin/sh

if [ ! -n "$1" ]
then
	LOADADDR="LOADADDR=0x40008000"
else
	LOADADDR=$1
fi

ARCH=arm
CROSS_COMPILE=arm-none-eabi-

echo	"\tARCH=${ARCH}\n"\
		"\tCROSS_COMPILE=${CROSS_COMPILE}\n"\
		"\t${LOADADDR}\n"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} uImage ${LOADADDR} -j4

if [ $? ]
then 
	echo "\ncompilation success, and will cpoy uImage to tftp directory.\n"
	cp arch/arm/boot/uImage /home/fyql/YJX/knowledge/tiny4412/tftp/
else
	echo "\ncompilation failed.\n"
fi

echo	"make dts"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs

echo	"backup config"
cp .config tiny4412_defconfig

echo	"\n\tdone."

