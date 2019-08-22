#!/bin/zsh
dirs=(parser messenger notifier handshaker client)
prev_dir=$(pwd)

for d in $dirs; do
	echo $d
	cd $prev_dir/$d
	make clean
	make
done

cd $prev_dir
