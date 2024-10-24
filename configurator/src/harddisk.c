#include "include/harddisk.h"

//================================================================
// Hard disk configuration

static void set_harddisk_config(
    char hd_drive,
    __uint8_t hd_buffer_type,
    __uint8_t hd_rtc,
    __uint8_t hd_timeout,
    __uint8_t hd_fakefloppy)
{
    ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_GEMDRIVE_DRIVE, MAX_KEY_LENGTH);
    snprintf(entry->value, MAX_STRING_VALUE_LENGTH, "%c\0", hd_drive);
    entry->dataType = TYPE_STRING;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);

    entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_GEMDRIVE_BUFF_TYPE, MAX_KEY_LENGTH);
    snprintf(entry->value, MAX_STRING_VALUE_LENGTH, hd_buffer_type == 0 ? "0" : "1");
    entry->dataType = TYPE_INT;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);

    entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_GEMDRIVE_RTC, MAX_KEY_LENGTH);
    snprintf(entry->value, MAX_STRING_VALUE_LENGTH, hd_rtc ? "true" : "false");
    entry->dataType = TYPE_BOOL;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);

    entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_GEMDRIVE_TIMEOUT_SEC, MAX_KEY_LENGTH);
    snprintf(entry->value, MAX_STRING_VALUE_LENGTH, "%d", hd_timeout);
    entry->dataType = TYPE_INT;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);

    entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_GEMDRIVE_FAKEFLOPPY, MAX_KEY_LENGTH);
    snprintf(entry->value, MAX_STRING_VALUE_LENGTH, hd_fakefloppy ? "true" : "false");
    entry->dataType = TYPE_BOOL;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), FLOPPYLOAD_WAIT_TIME, FALSE);
    free(entry);
}

__uint16_t harddisk_menu()
{
    __uint8_t display = TRUE;
    __uint8_t config_changed = FALSE;
    ConfigEntry *hd_drive_entry = get_config_entry(PARAM_GEMDRIVE_DRIVE);
    ConfigEntry *hd_buffer_type_entry = get_config_entry(PARAM_GEMDRIVE_BUFF_TYPE);
    ConfigEntry *hd_rtc_entry = get_config_entry(PARAM_GEMDRIVE_RTC);
    ConfigEntry *hd_folders_entry = get_config_entry(PARAM_GEMDRIVE_FOLDERS);
    ConfigEntry *hd_timeout_entry = get_config_entry(PARAM_GEMDRIVE_TIMEOUT_SEC);
    ConfigEntry *hd_fakefloppy_entry = get_config_entry(PARAM_GEMDRIVE_FAKEFLOPPY);

    char hd_drive = hd_drive_entry != NULL ? (hd_drive_entry->value[0] >= 'a' && hd_drive_entry->value[0] <= 'z' ? hd_drive_entry->value[0] - 32 : hd_drive_entry->value[0]) : 'C'; // C by default
    __uint8_t hd_buffer_type = hd_buffer_type_entry != NULL ? atoi(hd_buffer_type_entry->value) : 0;                                                                                // _dskbuff by default
    __uint8_t hd_rtc = hd_rtc_entry != NULL ? (hd_rtc_entry->value[0] == 't' || hd_rtc_entry->value[0] == 'T') : 1;                                                                 // Enabled by default
    __uint8_t hd_timeout = hd_timeout_entry != NULL ? atoi(hd_timeout_entry->value) : 45;
    __uint8_t hd_fakefloppy = hd_fakefloppy_entry != NULL ? (hd_fakefloppy_entry->value[0] == 't' || hd_fakefloppy_entry->value[0] == 'T') : 1; // Enabled by default
    char *hd_folders = malloc(MAX_FOLDER_LENGTH);
    strcpy(hd_folders, (hd_folders_entry != NULL ? hd_folders_entry->value : "/hd"));

    while (TRUE)
    {
        if (display)
        {
            PRINT_APP_HEADER(VERSION);
            printf("\r\n");
            printf("\r\n");
            printf("\r\n");

#ifndef _DEBUG
            if (check_folder_harddisks() > 0)
            {
                // Error. Back to main menu
                if (hd_folders)
                    free(hd_folders);
                return 0; // 0 is go to menu
            }
#endif
            printf("\r\n");
            printf("MicroSD card folder:\t%s\r\n", hd_folders);
            printf("\r\n");
            printf("[D]rive:\t\t%c:\\ \r\n", hd_drive);
            printf("Temp [M]emory type:\t%s\r\n", hd_buffer_type == 0 ? "_dskbuf" : "heap");
            printf("[R]TC enabled:\t\t%s\tNeeds WiFi to access a NTP server\r\n", hd_rtc ? "YES" : "NO");
            printf("[F]ake Floppy:\t\t%s\tEnable AUTO folder execution without floppies\r\n", hd_fakefloppy ? "YES" : "NO");
            printf("Network [T]imeout:\t%dsec\r\n", hd_timeout);
            printf("\r\n");
            printf("Options:\r\n\n");

            printf("[S] - Start emulation\r\n");
            printf("\n");
            printf("Press an option key or [ESC] to exit:");

            flush_kbd();
            display = FALSE;
        }

        if (Bconstat(2) != 0)
        {
            int fullkey = Bconin(2);
            __uint16_t key = fullkey & 0xFF;
            if (fullkey == KEY_ESC)
            {
                if (config_changed)
                {
                    send_sync_command(SAVE_CONFIG, NULL, 0, FLOPPYLOAD_WAIT_TIME, TRUE);
                    __uint16_t err = read_config();
                }

                // Back to main menu
                if (hd_folders)
                    free(hd_folders);
                return 0; // 0 return to menu, no ask
            }
            // Check if the input is 'D' or 'd'
            else if ((key == 'D') || (key == 'd'))
            {
                flush_kbd();
                // Change drive
                printf("\n\r\033KPlease select the drive to use, from C to Z. Do not overlap with other drives.\n\r");
                char *new_drive = read_input("\033KChanging to drive: ", TYPE_STRING);
                hd_drive = new_drive != NULL ? (new_drive[0] >= 'c' && new_drive[0] <= 'z' ? new_drive[0] - 32 : new_drive[0]) : 'C'; // C by default
                if (hd_drive < 'C' || hd_drive > 'Z')
                {
                    hd_drive = 'C';
                }
                set_harddisk_config(hd_drive, hd_buffer_type, hd_rtc, hd_timeout, hd_fakefloppy);
                config_changed = TRUE;
                display = TRUE;
            }
            // Check if the input is 'M' or 'm'
            else if ((key == 'M') || (key == 'm'))
            {
                // Toggle floppy buffer type
                hd_buffer_type = !hd_buffer_type & 0x01;
                set_harddisk_config(hd_drive, hd_buffer_type, hd_rtc, hd_timeout, hd_fakefloppy);
                display = TRUE;
                config_changed = TRUE;
            }
            else if ((key == 'R') || (key == 'r'))
            {
                // Toggle RTC
                hd_rtc = !hd_rtc & 0x01;
                set_harddisk_config(hd_drive, hd_buffer_type, hd_rtc, hd_timeout, hd_fakefloppy);
                display = TRUE;
                config_changed = TRUE;
            }
            else if ((key == 'T') || (key == 't'))
            {
                // Change timeout
                char *new_timeout = read_input("\n\r\033KPlease enter the timeout in seconds (from 30 to 255): ", TYPE_INT);
                hd_timeout = new_timeout != NULL ? atoi(new_timeout) : 45;
                if (hd_timeout < 30 || hd_timeout > 255)
                {
                    hd_timeout = 45;
                }
                set_harddisk_config(hd_drive, hd_buffer_type, hd_rtc, hd_timeout, hd_fakefloppy);
                config_changed = TRUE;
                display = TRUE;
            }
            else if ((key == 'F') || (key == 'f'))
            {
                // Toggle Fake floppy
                hd_fakefloppy = !hd_fakefloppy & 0x01;
                set_harddisk_config(hd_drive, hd_buffer_type, hd_rtc, hd_timeout, hd_fakefloppy);
                display = TRUE;
                config_changed = TRUE;
            }
            // Check if the input is 'S' or 's' or 'W' or 'w'
            else if ((key == 'S') || (key == 's'))
            {
                display = TRUE;
                if (config_changed)
                {
                    send_sync_command(SAVE_CONFIG, NULL, 0, FLOPPYLOAD_WAIT_TIME, FALSE);
                    __uint16_t err = read_config();
                }
                send_sync_command(BOOT_GEMDRIVE, NULL, 0, FLOPPYLOAD_WAIT_TIME, FALSE);
                return 1; // Positive is OK
            }
            else
            {
                press_key("\r\033KBoot option not found.");
                return 0;
            }
        }
    }
}
