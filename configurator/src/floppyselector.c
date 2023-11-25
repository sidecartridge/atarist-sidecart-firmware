#include "include/floppyselector.h"

//================================================================
// Floppy selector

static __uint8_t get_floppy_size_input()
{
    char input;
    while (1)
    {
        flush_kbd();
        input = Cnecin();
        // Check for '1', '2', '3', or ESC key
        if (input == '1' || input == '2' || input == '3')
        {
            __uint8_t opt = (input - '1') + 1;
            printf("%d", opt);
            return opt; // Return the valid input
        }
        else if (input == 27)
        {
            return 0; // Return 0 to indicate ESC key
        }
    }
}

static __uint16_t floppy_selector(mode_t floppy_command)
{
    __uint8_t again = TRUE;
    while (again)
    {
        again = FALSE;
        PRINT_APP_HEADER(VERSION);

        printf("\r\n");

#ifndef _DEBUG
        if (check_folder_floppies() > 0)
        {
            // Errror. Back to main menu
            return 0; // 0 is go to menu
        }

        printf("Loading available Floppy images...");

        send_sync_command(LIST_FLOPPIES, NULL, 0, 10, TRUE);
#endif
        printf("\r\n");

        int num_files = -1;
        __uint32_t file_list_mem = (__uint32_t)(FILE_LIST_START_ADDRESS + sizeof(__uint32_t));

#ifdef _DEBUG
        printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);
#endif
        char *file_array = read_files_from_memory((char *)file_list_mem);

        flush_kbd();

        if (floppy_command == LOAD_FLOPPY_RW)
        {

            char *new_file = "< CREATE A BLANK FLOPPY IMAGE >";
            file_array = prepend_file_to_array(file_array, new_file);
        }

        if (!file_array)
        {
            printf("No files found. Check if you have Floppie images copied in your microSD card!\r\n");
            press_key("Press any key to exit...\r\n");
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

        if ((floppy_number == 1) && (floppy_command == LOAD_FLOPPY_RW))
        {
            // Create a new floppy image. Ask for parameters
            printf("\r\033KSize [1]-SS/DD (360KB), [2]-DS/DD (720KB), [3]-HD (1.44MB). [ESC] exit: ");
            __uint8_t floppy_type = get_floppy_size_input();

            if (floppy_type == 0)
            {
                // ESC key pressed. Back to main menu
                return 0; // 0 is go to menu
            }

            FloppyImageHeader floppy_header = {
                .template = floppy_type,
                .num_tracks = floppy_type >= 2 ? 80 : 40,
                .num_sectors = floppy_type > 2 ? 18 : 9,
                .num_sides = floppy_type >= 2 ? 2 : 1,
                .overwrite = 1,
                .volume_name = "",
                .floppy_name = ""};

            printf("\r\n\033KVolume name (max 8 chars, dot, max 3 chars): ");
            read_string(floppy_header.volume_name, 12);
            printf("\r\n\033KImage name (max 255 chars). Empty to cancel: ");
            read_string(floppy_header.floppy_name, 255);

            if (strlen(floppy_header.floppy_name) == 0)
            {
                // Empty floppy name. Back to main menu
                return 0; // 0 is go to menu
            }

#ifdef _DEBUG
            printf("Floppy size: %d\r\n", floppy_header.template);
            printf("Floppy tracks: %d\r\n", floppy_header.num_tracks);
            printf("Floppy sectors: %d\r\n", floppy_header.num_sectors);
            printf("Floppy sides: %d\r\n", floppy_header.num_sides);
            printf("Floppy volume name: %s\r\n", floppy_header.volume_name);
            printf("Floppy name: %s\r\n", floppy_header.floppy_name);
#endif

            printf("\r\033KCreating new floppy image...");

#ifndef _DEBUG
            send_sync_command(CREATE_FLOPPY, &floppy_header, sizeof(FloppyImageHeader), 30, TRUE);
#endif
            press_key("\r\033KNew floppy image created. Please select it from the list.");

            again = TRUE;
        }
        else
        {
            printf("\033KSelected the Floppy image file: %d. ", floppy_number);

            print_file_at_index(file_array, floppy_number - 1, 0);

            printf("\r\033KLoading floppy. Wait until the led in the board blinks a 'F' in morse...");

            if (floppy_command == LOAD_FLOPPY_RW)
                floppy_number--; // Bypass the menu added option to create images

            send_sync_command(floppy_command, &floppy_number, 2, 30, TRUE);

            sleep_seconds(5, FALSE);

            printf("\r\033KFloppy image file loaded.");
        }
    }

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
