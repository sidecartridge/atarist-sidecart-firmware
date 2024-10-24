#include "include/floppyselector.h"

//================================================================
// Floppy selector

static void set_floppy_config(__uint8_t boot_sector_enabled,
                              __uint8_t xbios_enabled,
                              __uint8_t buffer_type,
                              __uint8_t network_enabled)
{
    ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_FLOPPY_BOOT_ENABLED, MAX_KEY_LENGTH);
    strncpy(entry->value, boot_sector_enabled ? "true" : "false", MAX_STRING_VALUE_LENGTH);
    entry->dataType = TYPE_BOOL;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);

    entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_FLOPPY_XBIOS_ENABLED, MAX_KEY_LENGTH);
    strncpy(entry->value, xbios_enabled ? "true" : "false", MAX_STRING_VALUE_LENGTH);
    entry->dataType = TYPE_BOOL;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);

    entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_FLOPPY_BUFFER_TYPE, MAX_KEY_LENGTH);
    snprintf(entry->value, MAX_STRING_VALUE_LENGTH, buffer_type == 0 ? "0" : "1");
    entry->dataType = TYPE_INT;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);

    entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_FLOPPY_NET_ENABLED, MAX_KEY_LENGTH);
    strncpy(entry->value, network_enabled ? "true" : "false", MAX_STRING_VALUE_LENGTH);
    entry->dataType = TYPE_BOOL;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);
}

static void set_floppy_name(char *floppy_image_name, int floppy_disk_number)
{
    ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, floppy_disk_number == 0 ? PARAM_FLOPPY_IMAGE_A : PARAM_FLOPPY_IMAGE_B, MAX_KEY_LENGTH);
    strncpy(entry->value, floppy_image_name, MAX_STRING_VALUE_LENGTH);
    entry->dataType = TYPE_STRING;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);
}

static void set_floppy_name_a(char *floppy_image_name)
{
    set_floppy_name(floppy_image_name, 0);
}

static void set_floppy_name_b(char *floppy_image_name)
{
    set_floppy_name(floppy_image_name, 1);
}

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

static __uint8_t get_floppy_rw_input()
{
    char input;
    while (1)
    {
        flush_kbd();
        input = Cnecin();
        // Check for 'r', 'w' or ESC key
        if (input == 'r' || input == 'w')
        {
            __uint8_t opt = input;
            printf("%c", opt);
            return opt; // Return the valid input
        }
        else if (input == 27)
        {
            return 0; // Return 0 to indicate ESC key
        }
    }
}

static __uint16_t floppy_chooser(char *chosen_image_name)
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

#ifndef _DEBUG
    if (check_folder_floppies() > 0)
    {
        // Errror. Back to main menu
        return 0; // 0 is go to menu
    }

    printf("Loading available Floppy images...");

    send_sync_command(LIST_FLOPPIES, NULL, 0, FLOPPYLIST_WAIT_TIME, TRUE);
#endif
    printf("\r\n");

    int num_files = -1;
    __uint32_t file_list_mem = (__uint32_t)(FILE_LIST_START_ADDRESS + sizeof(__uint32_t));

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);
#endif
    char *file_array = read_files_from_memory((char *)file_list_mem);

    flush_kbd();

    if (!file_array)
    {
        printf("No files found. Check if you have Floppie images copied in your microSD card!\r\n");
        press_key("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }
    __int16_t floppy_number = display_paginated_content(file_array, get_file_count(file_array), ELEMENTS_PER_PAGE, "Floppy images", NULL);
    if (floppy_number > 0)
    {
        strcpy(chosen_image_name, get_file_at_index(file_array, floppy_number - 1));
    }
    return floppy_number;
}

__uint16_t floppy_menu()
{
    __uint8_t display = TRUE;
    __uint8_t config_changed = FALSE;
    ConfigEntry *floppy_boot_enabled_entry = get_config_entry(PARAM_FLOPPY_BOOT_ENABLED);
    ConfigEntry *floppy_buffer_type_entry = get_config_entry(PARAM_FLOPPY_BUFFER_TYPE);
    ConfigEntry *floppy_xbios_enabled_entry = get_config_entry(PARAM_FLOPPY_XBIOS_ENABLED);
    ConfigEntry *floppies_folder_entry = get_config_entry(PARAM_FLOPPIES_FOLDER);
    ConfigEntry *floppies_floppy_image_a_entry = get_config_entry(PARAM_FLOPPY_IMAGE_A);
    ConfigEntry *floppies_floppy_image_b_entry = get_config_entry(PARAM_FLOPPY_IMAGE_B);
    ConfigEntry *floppy_net_enabled_entry = get_config_entry(PARAM_FLOPPY_NET_ENABLED);

    __uint8_t floppy_boot_enabled = floppy_boot_enabled_entry != NULL ? (floppy_boot_enabled_entry->value[0] == 't' || floppy_boot_enabled_entry->value[0] == 'T') : 1;     // Enabled by default
    __uint8_t floppy_buffer_type = floppy_buffer_type_entry != NULL ? atoi(floppy_buffer_type_entry->value) : 0;                                                            // _dskbuff by default
    __uint8_t floppy_xbios_enabled = floppy_xbios_enabled_entry != NULL ? (floppy_xbios_enabled_entry->value[0] == 't' || floppy_xbios_enabled_entry->value[0] == 'T') : 1; // Enabled by default
    __uint8_t floppy_net_enabled = floppy_net_enabled_entry != NULL ? (floppy_net_enabled_entry->value[0] == 't' || floppy_net_enabled_entry->value[0] == 'T') : 0;         // Disabled by default
    char *floppies_folder = malloc(MAX_FOLDER_LENGTH);
    strcpy(floppies_folder, (floppies_folder_entry != NULL ? floppies_folder_entry->value : "/floppies"));
    char *floppy_image_a = malloc(MAX_FOLDER_LENGTH);
    strcpy(floppy_image_a, (floppies_floppy_image_a_entry != NULL) ? floppies_floppy_image_a_entry->value : "");
    char *floppy_image_b = malloc(MAX_FOLDER_LENGTH);
    strcpy(floppy_image_b, (floppies_floppy_image_b_entry != NULL) ? floppies_floppy_image_b_entry->value : "");

    while (TRUE)
    {
        if (display)
        {
            PRINT_APP_HEADER(VERSION);
            printf("\r\n");
            printf("\r\n");
            printf("\r\n");

#ifndef _DEBUG
            if (check_folder_floppies() > 0)
            {
                // Errror. Back to main menu
                if (floppy_image_a)
                    free(floppy_image_a);
                if (floppy_image_b)
                    free(floppy_image_b);
                if (floppies_folder)
                    free(floppies_folder);
                return 0; // 0 is go to menu
            }
#endif
            printf("MicroSD card folder:\t%s\r\n", floppies_folder);
            printf("Floppy A:\t\t%s\r\n", strlen(floppy_image_a) > 0 ? floppy_image_a : "<EMPTY>");
            printf("Floppy B:\t\t%s\r\n", strlen(floppy_image_b) > 0 ? floppy_image_b : "<EMPTY>");
            printf("\r\n");
            printf("[E]xecute boot sector:\t%s\tDisable if a boot sector virus is detected\r\n", floppy_boot_enabled ? "YES" : "NO");
            printf("[X]BIOS interception:\t%s\r\n", floppy_xbios_enabled ? "YES" : "NO");
            printf("Temp [M]emory type:\t%s\r\n", floppy_buffer_type == 0 ? "_dskbuf" : "heap");
            printf("[W]eb manager:\t\t%s\tEXPERIMENTAL (Needs WiFi)\r\n", floppy_net_enabled ? "YES" : "NO");
            printf("\r\n");
            printf("Options:\r\n\n");

            // Calculate the length of the strings
            size_t len_floppy_image_a = strlen(floppy_image_a);
            size_t len_floppy_image_b = strlen(floppy_image_b);
            size_t len_extension = strlen(".st.rw");
            char *start_of_extension_a = NULL;
            char *start_of_extension_b = NULL;
            if (len_floppy_image_a >= len_extension)
            {
                // Find the starting position of the extension in floppy_image_a
                start_of_extension_a = &floppy_image_a[len_floppy_image_a - len_extension];
            }
            if (len_floppy_image_b >= len_extension)
            {
                // Find the starting position of the extension in floppy_image_b
                start_of_extension_b = &floppy_image_b[len_floppy_image_b - len_extension];
            }
            // printf("start of extension a: %s\r\n", start_of_extension_a);
            // printf("start of extension b: %s\r\n", start_of_extension_b);

            if (strlen(floppy_image_a) == 0)
            {
                printf("[A] - Select floppy image for drive A\r\n");
            }
            else if (strlen(floppy_image_a) > 0)
            {
                printf("[SHIFT + A] - Eject floppy image in drive A\r\n");
            }

            if (strlen(floppy_image_b) == 0)
            {
                printf("[B] - Select floppy image for drive B\r\n");
            }
            else if (strlen(floppy_image_b) > 0)
            {
                printf("[SHIFT + B] - Eject floppy image in drive B\r\n");
            }

            if ((strlen(floppy_image_a) > 0) || (strlen(floppy_image_b) > 0))
            {
                printf("[S] - Start the emulation\r\n");
            }

            if ((start_of_extension_a != NULL && (strcasecmp(start_of_extension_a, ".st.rw") != 0)) ||
                (start_of_extension_b != NULL && (strcasecmp(start_of_extension_b, ".st.rw") != 0)))
            {
                printf("[W] - Start the emulation and convert images to read-write mode\r\n");
            }

            printf("[F] - Format a floppy image\r\n");
            printf("\n");
            printf("Press an option key or [ESC] to exit:");

            flush_kbd();
            display = FALSE;
        }

        if (Bconstat(2) != 0)
        {
            int fullkey = Bconin(2);
            __uint16_t key = fullkey & 0xFF;
            __uint32_t shift = Kbstate() & 0x03; // Shift keys mask
            if (fullkey == KEY_ESC)
            {
                if (config_changed)
                {
                    send_sync_command(SAVE_CONFIG, NULL, 0, FLOPPYLOAD_WAIT_TIME, TRUE);
                    __uint16_t err = read_config();
                }

                // Back to main menu
                if (floppy_image_a)
                    free(floppy_image_a);
                if (floppy_image_b)
                    free(floppy_image_b);
                if (floppies_folder)
                    free(floppies_folder);
                return 0; // 0 return to menu, no ask
            }
            // Check if the input is 'E' or 'e'
            if ((key == 'E') || (key == 'e'))
            {
                // Toggle floppy boot enabled
                floppy_boot_enabled = !floppy_boot_enabled & 0x01;
                set_floppy_config(floppy_boot_enabled, floppy_xbios_enabled, floppy_buffer_type, floppy_net_enabled);
                display = TRUE;
                config_changed = TRUE;
            }
            // Check if the input is 'X' or 'x'
            else if ((key == 'X') || (key == 'x'))
            {
                // Toggle floppy xbios enabled
                floppy_xbios_enabled = !floppy_xbios_enabled & 0x01;
                set_floppy_config(floppy_boot_enabled, floppy_xbios_enabled, floppy_buffer_type, floppy_net_enabled);
                display = TRUE;
                config_changed = TRUE;
            }
            // Check if the input is 'M' or 'm'
            else if ((key == 'M') || (key == 'm'))
            {
                // Toggle floppy buffer type
                floppy_buffer_type = !floppy_buffer_type & 0x01;
                set_floppy_config(floppy_boot_enabled, floppy_xbios_enabled, floppy_buffer_type, floppy_net_enabled);
                display = TRUE;
                config_changed = TRUE;
            }
            // Check if the input is 'W' or 'w'
            else if ((key == 'W') || (key == 'w'))
            {
                // Toggle network enabled
                floppy_net_enabled = !floppy_net_enabled & 0x01;
                set_floppy_config(floppy_boot_enabled, floppy_xbios_enabled, floppy_buffer_type, floppy_net_enabled);
                display = TRUE;
                config_changed = TRUE;
            }
            // Check if the input is 'A' or 'a'
            else if ((key == 'A') || (key == 'a'))
            {
                if (strlen(floppy_image_a) == 0)
                {
                    // Select floppy image A
                    floppy_chooser(floppy_image_a);
                    if (floppy_image_a != NULL && strlen(floppy_image_a) > 0 && strcmp(floppy_image_a, floppy_image_b) == 0)
                    {
                        strcpy(floppy_image_a, "");
                    }
                    display = TRUE;
                }
                else if (shift)
                {
                    // Eject floppy image A
                    strcpy(floppy_image_a, "");
                    display = TRUE;
                    set_floppy_name_a(floppy_image_a);
                }
            }
            // Check if the input is 'B' or 'b'
            else if ((key == 'B') || (key == 'b'))
            {
                if (strlen(floppy_image_b) == 0)
                {
                    // Select floppy image B
                    floppy_chooser(floppy_image_b);
                    if (floppy_image_b != NULL && strlen(floppy_image_b) > 0 && strcmp(floppy_image_b, floppy_image_a) == 0)
                    {
                        strcpy(floppy_image_b, "");
                    }
                    display = TRUE;
                }
                else if (shift)
                {
                    // Eject floppy image B
                    strcpy(floppy_image_b, "");
                    display = TRUE;
                    set_floppy_name_b(floppy_image_b);
                }
            }
            // Check if the input is 'F' or 'f'
            else if ((key == 'F') || (key == 'f'))
            {
                display = TRUE;
                // Format a floppy image
                // Create a new floppy image. Ask for parameters
                printf("\r\033KSize [1]-SS/DD (360KB), [2]-DS/DD (720KB), [3]-HD (1.44MB). [ESC] exit: ");
                __uint8_t floppy_type = get_floppy_size_input();

                if (floppy_type != 0)
                {
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
                    if (strlen(floppy_header.floppy_name) > 0)
                    {
#ifdef _DEBUG
                        printf("Floppy size: %d\r\n", floppy_header.template);
                        printf("Floppy tracks: %d\r\n", floppy_header.num_tracks);
                        printf("Floppy sectors: %d\r\n", floppy_header.num_sectors);
                        printf("Floppy sides: %d\r\n", floppy_header.num_sides);
                        printf("Floppy volume name: %s\r\n", floppy_header.volume_name);
                        printf("Floppy name: %s\r\n", floppy_header.floppy_name);
#endif

                        printf("\r\033KCreating a new floppy image...");

#ifndef _DEBUG
                        send_sync_command(CREATE_FLOPPY, &floppy_header, sizeof(FloppyImageHeader), FLOPPYLOAD_WAIT_TIME, SPINNING);
#endif
                        press_key("\r\033KNew floppy image created.");
                        strcpy(floppy_image_a, floppy_header.floppy_name);
                        strcat(floppy_image_a, ".st.rw");
                    }
                }
            }
            // Check if the input is 'S' or 's' or 'W' or 'w'
            else if (((key == 'S') || (key == 's') || (key == 'W') || (key == 'w')) &&
                     ((strcmp(floppy_image_a, "<EMPTY>") != 0) || (strcmp(floppy_image_b, "<EMPTY>") != 0)))
            {
                display = TRUE;
                printf("\r\033KLoading. Wait until the led in the board blinks a 'F' in morse...");

#ifndef _DEBUG
                send_sync_command(LIST_FLOPPIES, NULL, 0, FLOPPYLIST_WAIT_TIME, SPINNING);
#endif

                __uint32_t file_list_mem = (__uint32_t)(FILE_LIST_START_ADDRESS + sizeof(__uint32_t));
#ifdef _DEBUG
                printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);
#endif
                char *file_array = read_files_from_memory((char *)file_list_mem);
                __int16_t floppy_number_a = -1;
                __int16_t floppy_number_b = -1;
                if (strcmp(floppy_image_a, "<EMPTY>") != 0)
                {
                    floppy_number_a = get_index_of_filename(file_array, floppy_image_a);
                }
                if (strcmp(floppy_image_b, "<EMPTY>") != 0)
                {
                    floppy_number_b = get_index_of_filename(file_array, floppy_image_b);
                }

                __uint16_t floppy_command = LOAD_FLOPPY_RO;
                if ((key == 'W') || (key == 'w'))
                {
                    floppy_command = LOAD_FLOPPY_RW;
                }

                if (config_changed)
                {
                    send_sync_command(SAVE_CONFIG, NULL, 0, FLOPPYLOAD_WAIT_TIME, SPINNING);
                    __uint16_t err = read_config();
                }

                if (floppy_number_a >= 0)
                {
                    __uint32_t params = ((__uint32_t)(floppy_number_a + 1)) << 16 | 0x0; // Always add 1 to the index. Drive A: 0x0
                    send_sync_command(floppy_command, &params, 4, get_download_timeout(), SPINNING);
                    printf("\r\033KFloppy A: image file loaded.\r\n");
                }
                else
                {
                    printf("\r\033KFloppy A: image file A not found.\r\n");
                }
                if (floppy_number_b >= 0)
                {
                    __uint32_t params = ((__uint32_t)(floppy_number_b + 1)) << 16 | 0x1; // Always add 1 to the index. Drive B: 0x1
                    send_sync_command(floppy_command, &params, 4, get_download_timeout(), SPINNING);
                    printf("\r\033KFloppy B: image file loaded.\r\n");
                }
                else
                {
                    printf("\r\033KFloppy B: image file B not found.\r\n");
                }

                if (floppy_number_a >= 0 || floppy_number_b >= 0)
                {
                    send_async_command(REBOOT, NULL, 0);
                    return 1; // Positive is OK
                }
                else
                {
                    return 0;
                }
            }
        }
    }
}
