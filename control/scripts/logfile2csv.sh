#!/bin/bash

# Check for correct number of runtime args
if [ "$#" -le 1 ]; then
    echo "Must specify log file name and list of variables to extract."
    echo "Example: ./extract_logvar.sh <logfile> var1 var2 ... varN"
    exit 1
fi

LOGFILE="$1"
IFS='\n'

echo "Processing logfile $LOGFILE"

shift
for var in "$@"
do
    echo "Processing $var"
    datas=$(cat "$LOGFILE" | grep -o "{.*}" | grep "$var" | awk '{print substr($2, 1, length($2)-1)}')
    read -r -a "$var" <<< "$datas"
done

# for var in "$@"
# do
#     varname="datas_array_$var"
#     echo "${!varname[0]}"
# done

echo $varnums