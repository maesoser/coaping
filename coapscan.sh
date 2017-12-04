#!/bin/sh
while :
do
while
  set $(dd if=/dev/urandom bs=4 count=1 2>/dev/null | od -An -tu1)
  [ $1 -lt 224 ] &&
  [ $1 -ne 10 ] &&
  { [ $1 -ne 192 ] || [ $2 -ne 168 ]; } &&
  { [ $1 -ne 172 ] || [ $2 -lt 16 ] || [ $2 -gt 31 ]; }
do :; done
ip_address=$1.$2.$3.$4
./coaping -n 1 $ip_address
done
