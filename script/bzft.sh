#!/bin/bash

BZFTCMD=/root/baize/build/bin/baize_file_transfer

if [[ "$1" = "put" ]]; then
    echo "calculate $2 md5sum, please wait for a while"
    BZMD5=`md5sum $2 | awk '{print $1}'`
    $BZFTCMD $1 $2 $BZMD5
fi

if [[ "$1" = "get" ]]; then
    $BZFTCMD $1 $2
    BZFILE=`cat .bzft.pathfile`
    BZMD5=`cat .bzft.md5`
    BZCAL_MD5=`md5sum $BZFILE | awk '{print $1}'`
    echo "send side md5: $BZMD5"
    echo "recv side md5: $BZCAL_MD5"
    if [[ "$1" = "get" ]]; then
        echo "md5 compare succeed!!"
    else
        echo "md5 compare failed!!"
    fi
    rm .bzft.*
fi
