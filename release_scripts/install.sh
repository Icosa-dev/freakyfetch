#!/bin/bash
set -xe
printf "Installing binary...\n"
mkdir -pv /usr/bin /usr/lib/freakyfetch /usr/share/man/man1 /etc/freakyfetch
cp freakyfetch /usr/bin
cp libfetch.so /usr/lib
cp fetch.h /usr/include
cp -r res/* /usr/lib/freakyfetch
cp default.config /etc/freakyfetch/config
cp ./freakyfetch.1.gz /usr/share/man/man1
printf "Done!\n"

