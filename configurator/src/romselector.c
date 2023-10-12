#include "include/romselector.h"

//================================================================
// ROM selector

__uint8_t rom_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    send_command(LIST_ROMS, NULL, 0);

    please_wait("Loading available ROM images...", WAIT_TIME);

    printf("\r\n");

    int num_files = -1;
    __uint32_t file_list_mem = FILE_LIST_START_ADDRESS;

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);
#endif
    char *file_array = read_files_from_memory((__uint8_t *)file_list_mem);

    if (!file_array)
    {
        printf("No files found. Check if you have ROMS copied in your microSD card!\r\n");
        printf("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }
    __int16_t rom_number = display_paginated_content(file_array, get_file_count(file_array), ELEMENTS_PER_PAGE, "ROM images");

    if (rom_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    locate(0, 22);

    printf("\033KSelected the ROM file: %d. ", rom_number);

    print_file_at_index(file_array, rom_number - 1, 0);

    send_command(LOAD_ROM, &rom_number, 2);

    please_wait("Loading ROM. Wait until the led in the board blinks a 'E' or 'D' in morse.", WAIT_TIME);

    printf("\033KROM file loaded. ");

    return 1; // Positive is OK
}
