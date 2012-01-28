#!/bin/bash

echo -e '@nodes\nlabel'
awk '{ print $1"\n"$2 }' $1 | sort -n | uniq
echo -e '@arcs\n\t\t-'
sort -n $1 | uniq

