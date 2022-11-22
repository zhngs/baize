#!/bin/bash

OPENSSL=openssl-1.1.1p
OPENSSL_PATH=https://www.openssl.org/source/old/1.1.1/$OPENSSL.tar.gz
CUR_DIR=

download() {
    if [ -f "$1" ]; then
        echo "$1 existed."
    else
        echo "$1 not existed, begin to download..."
        wget $2
        if [ $? -eq 0 ]; then
            echo "download $1 successed";
        else
            echo "Error: download $1 failed";
            return 1;
        fi
    fi
    return 0
}

get_cur_dir() {
  # get the fully qualified path to the script
  case $0 in
    /*)
      SCRIPT="$0"
      ;;
    *)
      PWD_DIR=$(pwd);
      SCRIPT="${PWD_DIR}/$0"
      ;;
  esac
  # resolve the true real path without any sym links.
  CHANGED=true
  while [ "X$CHANGED" != "X" ]
  do
    # change spaces to ":" so the tokens can be parsed.
    SAFESCRIPT=`echo $SCRIPT | sed -e 's; ;:;g'`
    # get the real path to this script, resolving any symbolic links
    TOKENS=`echo $SAFESCRIPT | sed -e 's;/; ;g'`
    REALPATH=
    for C in $TOKENS
    do
      # change any ":" in the token back to a space.
      C=`echo $C | sed -e 's;:; ;g'`
      REALPATH="$REALPATH/$C"
      # if REALPATH is a sym link, resolve it.  Loop for nested links.
      # Reference: https://github.com/apache/tomcat/blob/main/bin/catalina.sh#L129
      while [ -h "$REALPATH" ]
      do
        LS="`ls -ld "$REALPATH"`"
        LINK="`expr "$LS" : '.*-> \(.*\)$'`"
        if expr "$LINK" : '/.*' > /dev/null
        then
          # LINK is absolute.
          REALPATH="$LINK"
        else
          # LINK is relative.
          REALPATH="`dirname "$REALPATH"`""/$LINK"
        fi
      done
    done

    if [ "$REALPATH" = "$SCRIPT" ]
    then
      CHANGED=""
    else
      SCRIPT="$REALPATH"
    fi
  done
  # change the current directory to the location of the script
  CUR_DIR=$(dirname "${REALPATH}")
}

build_openssl()
{
  rm -rf $OPENSSL.tar.gz
  download $OPENSSL.tar.gz $OPENSSL_PATH
  tar -xvf $OPENSSL.tar.gz
  cd $OPENSSL
  ./config \
  --prefix=$CUR_DIR/openssl \
  --openssldir=$CUR_DIR/openssl \
  no-shared \
  -DOPENSSL_TLS_SECURITY_LEVEL=2 \
  enable-ec_nistp_64_gcc_128
  make
  make install
}

get_cur_dir
build_openssl
