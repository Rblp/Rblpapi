#!/bin/bash
# Emacs please make this -*- mode: shell-mode; -*-

uname=$(type -P uname)
if [ "${uname}" = "" ]; then
    echo "You do not have uname so this is unlikely to be a Unix system. Exiting."
    exit -1
fi

arch=$(uname -m)

file="http://static.bloomberglabs.com/api/cpp/blpapi_cpp_3.8.8.1-linux.tar.gz"
basefile=$(basename ${file})
release=$(basename ${basefile} -linux.tar.gz)
    
tempdir=$(mktemp --directory --suffix blp)
cd ${tempdir}
wget ${file}
tar xfz ${basefile}
    
if [ "${arch}" = "x86_64" ]; then
    cp -vax ${release}/Linux/libblpapi3_64.so /usr/local/lib 
elif [ "${arch}" = "i686" ]; then
    cp -vax ${release}/Linux/libblpapi3_32.so /usr/local/lib 
else 
    echo "Unknown architecture: ${arch}. Exiting."
    exit -1
fi
    
## update ldconfig cache
ldconfig
cd .. && rm -rf ${tempdir}
