#!/bin/sh
dirs=(parser messenger notifier handshaker client server)
prev_dir=$(pwd)

for d in $dirs; do
	echo $d
	cd $prev_dir/$d
	make clean
	make
done

cd $prev_dir
