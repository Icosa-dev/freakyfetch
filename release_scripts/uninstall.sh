#!/bin/bash
set -xe
printf "Uninstalling...\n"
rm -f /usr/bin/freakyfetch
rm -rf /usr/lib/freakyfetch
rm -f /usr/lib/libfetch.so
rm -f /usr/include/fetch.h
rm -rf /etc/freakyfetch
rm -f /usr/share/man/man1/freakyfetch.1.gz
printf "Done!\n"

