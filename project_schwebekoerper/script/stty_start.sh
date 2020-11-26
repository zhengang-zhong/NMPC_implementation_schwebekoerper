#!/bin/sh
#This is a script for stty
echo "starting stty..."
while :
do 
	echo "You are working on WSL or oracle virtualmachine? (w/v)"
	read MY_OS
	case $MY_OS in
		w)
			echo "STM32 identified on WSL as ttyS4"
			DEVICE="/dev/ttyS3"
			break
			;;
		v)
			echo "STM32 identified on virtualmachin as ttyACM0"
			DEVICE="/dev/ttyACM0"
			break
			;;		
		*)
			echo "unindentified input"
			;;
	esac
done
echo "$DEVICE"
sudo chmod 666 $DEVICE
stty -F $DEVICE -a
stty -F $DEVICE sane 115200
cu -l $DEVICE -s 115200
#stty -F /dev/ttyS5 raw 9600 -echo -echoe -echok -echoctl -echoke -iexten -onlcr cs8 crtscts
