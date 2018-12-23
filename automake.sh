#!/bin/sh

if [ ! -n "$1" ]
then
	LOADADDR="LOADADDR=0x40008000"
else
	LOADADDR=$1
fi

ARCH=arm
CROSS_COMPILE=arm-none-eabi-

#make ARCH=${ARCH} tiny4412_defconfig

echo	"\tARCH=${ARCH}\n"\
		"\tCROSS_COMPILE=${CROSS_COMPILE}\n"\
		"\t${LOADADDR}\n"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} uImage ${LOADADDR} -j4
if [ $? ]
then 
	echo "\ncompilation success, and will cpoy uImage to tftp directory.\n"
	cp arch/arm/boot/uImage ../tftp/
else
	echo "\ncompilation failed.\n"
fi

echo	"make dts"
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} dtbs
if [ $? ]
then 
	echo "\ncompilation success, and will cpoy device tree to tftp directory.\n"
	cp arch/arm/boot/dts/exynos4412-tiny4412.dtb ../tftp/
else
	echo "\ncompilation failed.\n"
fi

echo	"backup config"
cp .config tiny4412_defconfig

echo	"\n\tdone."

