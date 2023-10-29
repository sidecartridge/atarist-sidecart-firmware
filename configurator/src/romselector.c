#include "include/romselector.h"

//================================================================
// ROM selector

__uint16_t rom_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    if (check_folder_roms() > 0)
    {
        // Errror. Back to main menu
        return 0; // 0 is go to menu
    }

    printf("Loading available ROM images...");

    send_sync_command(LIST_ROMS, NULL, 0, 10, TRUE);

    printf("\r\n");

    int num_files = -1;
    __uint32_t file_list_mem = (__uint32_t)(FILE_LIST_START_ADDRESS + sizeof(__uint32_t));

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);
#endif
    char *file_array = read_files_from_memory((char *)file_list_mem);

    if (!file_array)
    {
        printf("No files found. Check if you have ROMS copied in your microSD card!\r\n");
        printf("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }
    __int16_t rom_number = display_paginated_content(file_array, get_file_count(file_array), ELEMENTS_PER_PAGE, "ROM images", NULL);

    if (rom_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    locate(0, 22);

    printf("\033KSelected the ROM file: %d. ", rom_number);

    print_file_at_index(file_array, rom_number - 1, 0);

    printf("Loading ROM. Wait until the led in the board blinks a 'E' or 'D' in morse...");

    send_sync_command(LOAD_ROM, &rom_number, 2, 30, TRUE);

    printf("\r\033KROM file loaded. ");

    return 1; // Positive is OK
}
