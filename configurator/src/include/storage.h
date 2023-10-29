#ifndef STORAGE_H
#define STORAGE_H

#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "screen.h"
#include "commands.h"

typedef enum
{
    ROMS,
    FLOPPIES,
    FLOPPIES_DB,
    HARDDISKS
} StorageType;

typedef enum
{
    SD_CARD_MOUNTED = 0,       // SD card is OK
    SD_CARD_NOT_MOUNTED,       // SD not mounted
    ROMS_FOLDER_OK = 100,      // ROMs folder is OK
    ROMS_FOLDER_NOTFOUND,      // ROMs folder error
    FLOPPIES_FOLDER_OK = 200,  // Floppies folder is OK
    FLOPPIES_FOLDER_NOTFOUND,  // Floppies folder error
    HARDDISKS_FOLDER_OK = 300, // Hard disks folder is OK
    HARDDISKS_FOLDER_NOTFOUND, // Hard disks folder error
} StorageStatus;

#define MAX_FOLDER_LENGTH 128 // Max length of the folder names
typedef struct sd_data
{
    char roms_folder[MAX_FOLDER_LENGTH];      // ROMs folder name
    char floppies_folder[MAX_FOLDER_LENGTH];  // Floppies folder name
    char harddisks_folder[MAX_FOLDER_LENGTH]; // Hard disks folder name
    __uint32_t sd_size;                       // SD card size
    __uint32_t sd_free_space;                 // SD card free space
    __uint32_t roms_folder_count;             // ROMs folder count
    __uint32_t floppies_folder_count;         // Floppies folder count
    __uint32_t harddisks_folder_count;        // Hard disks folder count
    __uint16_t status;                        // Status of the SD card
    __uint16_t roms_folder_status;            // ROMs folder status
    __uint16_t floppies_folder_status;        // Floppies folder status
    __uint16_t harddisks_folder_status;       // Hard disks folder status
} SdCardData;

#ifdef _DEBUG
static SdCardData sdCardDataExample = {
    .roms_folder = "/roms",
    .floppies_folder = "/floppies",
    .harddisks_folder = "/harddisks",
    .roms_folder_status = ROMS_FOLDER_OK,
    .floppies_folder_status = FLOPPIES_FOLDER_OK,
    .harddisks_folder_status = HARDDISKS_FOLDER_OK,
    .sd_size = 1024,
    .sd_free_space = 512,
    .roms_folder_count = 256,
    .floppies_folder_count = 128,
    .harddisks_folder_count = 64,
    .status = SD_CARD_MOUNTED};
#endif

__uint16_t get_storage_status(__uint16_t show_bar);
__uint16_t check_folder_roms();
__uint16_t check_folder_floppies();
__uint16_t check_folder_floppies_db();
__uint16_t check_folder_harddisks();
void init_storage();

#endif // STORAGE_H
