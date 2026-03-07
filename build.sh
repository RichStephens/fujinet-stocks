#!/bin/bash

set -e

if [ -z "$WATCOM" ]; then
    export WATCOM=/opt/watcom
    export PATH=$PATH:$WATCOM/binl:$WATCOM/binl64
    export INCLUDE=$WATCOM/h
    export EDPATH=$WATCOM/eddat
    export WIPFC=$WATCOM/wipfc
fi

make clean
make atari apple2 coco msdos
cp -rv r2r/* /tnfs

