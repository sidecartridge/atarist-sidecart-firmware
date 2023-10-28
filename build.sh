#!/bin/bash

# Ensure an argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <working_folder> all|release"
    exit 1
fi

if [ -z "$2" ]; then
    echo "Usage: $0 <working_folder> all|release"
    exit 1
fi

cp -f version.txt ./configurator/version.txt

working_folder=$1
build_type=$2

ST_WORKING_FOLDER=$working_folder/configurator stcmd make $build_type
ST_WORKING_FOLDER=$working_folder stcmd make $build_type

filename_tos="./dist/SIDECART.TOS"

# Copy the SIDECART.TOS file for testing purposes
ST_WORKING_FOLDER=$working_folder stcmd cp ./configurator/dist/SIDECART.TOS $filename_tos

filename="./dist/FIRMWARE.IMG"

# Copy the BOOT.BIN file to a ROM size file for testing
ST_WORKING_FOLDER=$working_folder stcmd cp ./dist/BOOT.BIN $filename

# Determine the file size accordingly
filesize=$(ST_WORKING_FOLDER=$working_folder stcmd stat -c %s "$filename")

# Size for 64Kbytes in bytes
targetsize=$((64 * 1024))

# Check if the file is larger than 64Kbytes
if [ "$filesize" -gt "$targetsize" ]; then
    echo "The file is already larger than 64Kbytes."
    exit 2
fi

# Resize the file to 64Kbytes
ST_WORKING_FOLDER=$working_folder stcmd truncate -s $targetsize $filename

if [ $? -ne 0 ]; then
    echo "Failed to resize the file."
    exit 3
fi

echo "File has been resized to 64Kbytes."