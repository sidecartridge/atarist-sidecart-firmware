#include "include/floppyselector.h"

//================================================================
// Floppy selector

static __uint16_t floppy_selector(mode_t floppy_command)
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    if (check_folder_floppies() > 0)
    {
        // Errror. Back to main menu
        return 0; // 0 is go to menu
    }

    printf("Loading available Floppy images...");

    send_sync_command(LIST_FLOPPIES, NULL, 0, 10, TRUE);

    printf("\r\n");

    int num_files = -1;
    __uint32_t file_list_mem = (__uint32_t)(FILE_LIST_START_ADDRESS + sizeof(__uint32_t));

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);
#endif
    char *file_array = read_files_from_memory((char *)file_list_mem);

    if (!file_array)
    {
        printf("No files found. Check if you have Floppie images copied in your microSD card!\r\n");
        printf("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }
    __int16_t floppy_number = display_paginated_content(file_array, get_file_count(file_array), ELEMENTS_PER_PAGE, floppy_command == LOAD_FLOPPY_RO ? "Floppy images (Read Only)" : "Floppy images (Read/Write)", NULL);

    if (floppy_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    locate(0, 22);

    printf("\033KSelected the Floppy image file: %d. ", floppy_number);

    print_file_at_index(file_array, floppy_number - 1, 0);

    printf("\r\033KLoading floppy. Wait until the led in the board blinks a 'F' in morse...");

    send_sync_command(floppy_command, &floppy_number, 2, 30, TRUE);

    printf("\r\033KFloppy image file loaded. ");

    return 1; // Positive is OK
}

__uint16_t floppy_selector_ro(void)
{
    return floppy_selector(LOAD_FLOPPY_RO);
}

__uint16_t floppy_selector_rw(void)
{
    return floppy_selector(LOAD_FLOPPY_RW);
}
