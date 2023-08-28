
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


working_folder=$1
build_type=$2

ST_WORKING_FOLDER=$working_folder/romloader stcmd make $build_type
ST_WORKING_FOLDER=$working_folder stcmd make $build_type

filename="./dist/FIRMWARE.IMG"

# Copy the BOOT.BIN file to a ROM size file for testing
cp ./dist/BOOT.BIN $filename

# Determine the operating system and get the file size accordingly
OS=$(uname)
if [ "$OS" == "Linux" ]; then
    filesize=$(stat -c %s "$filename")
elif [ "$OS" == "Darwin" ]; then  # macOS's UNIX name is Darwin
    filesize=$(stat -f%z "$filename")
else
    echo "Unsupported operating system."
    exit 4
fi

# Size for 64Kbytes in bytes
targetsize=$((64 * 1024))

# Check if the file is larger than 64Kbytes
if [ "$filesize" -gt "$targetsize" ]; then
    echo "The file is already larger than 64Kbytes."
    exit 2
fi

# Resize the file to 64Kbytes
truncate -s $targetsize $filename

if [ $? -ne 0 ]; then
    echo "Failed to resize the file."
    exit 3
fi

echo "File has been resized to 64Kbytes."