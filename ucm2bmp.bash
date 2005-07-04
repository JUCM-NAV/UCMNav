#!/usr/bin/env bash
for epsfile in *.eps 
do 
	echo Converting "$epsfile" to "${epsfile%%.*}".bmp
	./ps2bmp.bash -f bmp256 -s 2.0 "$epsfile" "${epsfile%%.*}".bmp
done

