#!/bin/bash

/bin/tar xf networkx.tgz &> /dev/null
/usr/bin/python sigcomm2012.py $@
