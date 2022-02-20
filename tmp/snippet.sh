#!/bin/sh

DEFAULTVAL=${1:-"empty"}

## Set the source to retrieve header files from
if [ -z ${VAL+DEFAULTVAL} ]; then 
  VAL=$DEFAULTVAL; 
  echo "using default source $VAL"; 
else 
  echo "using user defined source $VAL"; 
fi
