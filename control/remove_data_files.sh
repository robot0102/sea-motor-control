#!/bin/bash

PASSKEY=12345678

cd /home/pc104/Documents/_data_files/

read -p "Enter passkey (1..8) to remove files: " PASSKEY_TEST 

if [ $PASSKEY_TEST -eq $PASSKEY ]
then
    sudo rm -r *
    
    echo " "
    echo "Removed files..."
    echo " "
else
    echo " "
    echo "Invalid passkey..."
    echo " "
fi

