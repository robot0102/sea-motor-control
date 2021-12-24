#!/bin/bash

PORT=33333

nc -ul ${PORT} | awk '{print $4, $5}' | feedgnuplot --dataid --autolegend --lines --stream 0 --xlen 1000 --exit
