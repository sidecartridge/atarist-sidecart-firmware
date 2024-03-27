#ifndef FLOPPYSELECTOR_H_
#define FLOPPYSELECTOR_H_

#include <sys/types.h>
#include <stdio.h>

#include "commands.h"
#include "screen.h"
#include "helper.h"
#include "storage.h"

// Define the structure to hold floppy image parameters
typedef struct
{
    __uint16_t template;
    __uint16_t num_tracks;
    __uint16_t num_sectors;
    __uint16_t num_sides;
    __uint16_t overwrite;
    char volume_name[14];
    char floppy_name[256];
} FloppyImageHeader;

__uint16_t floppy_menu();

#endif /* FLOPPYSELECTOR_H_ */