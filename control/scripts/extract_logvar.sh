#!/bin/bash
# Check for correct number of runtime args
if [ "$#" -ne 2 ]; then
    echo "Must specify which variable to extract and log file name."
    echo "Example: ./extract_logvar.sh <logfile> <varname>"
    exit 1
fi

cat $1 | awk "/$2/"'{print substr($4, 1, length($4)-1)}' > logs/$2.csv
cat logs/$2.csv | feedgnuplot --lines --points --title "Loop Utilization" --xlabel "Sample" --ylabel "Time (ms)" --hardcopy "logs/$2.png"