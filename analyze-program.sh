#!/bin/bash

if [ $# -ne 1 ] || [ ! -d $1 ]; then
  echo "Usage: analyze-program.sh PATH-TO-DIR"
  exit 1
fi

for statf in $1/*.txt; do
    echo $statf
done

