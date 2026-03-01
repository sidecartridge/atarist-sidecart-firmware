./build.sh "$PWD" all

dd if=/dev/zero bs=1 count=4 2>/dev/null | cat - ./dist/FIRMWARE.IMG > tmp && mv tmp ./FIRMWARE.STC
dd if=/dev/zero bs=1 count=65536 >> FIRMWARE.STC 2>/dev/null
