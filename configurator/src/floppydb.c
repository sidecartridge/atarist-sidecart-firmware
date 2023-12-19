#include "include/floppydb.h"

//================================================================
// Floppy selector

__uint16_t floppy_db()
{
    __uint16_t alphanum_bar_y_pos = 4;

    void erase_table(int y_pos, int num_lines)
    {
        locate(0, y_pos);
        for (int line = 0; line <= num_lines; line++)
        {
            // Delete the line from the current cursor position
            printf("\033K\r\n");
        }
    }

    void print_alphanum_bar()
    {
        locate(0, alphanum_bar_y_pos - 2);
        printf("    Press A..Z and 0..9 to filter the database:");
        locate(0, alphanum_bar_y_pos);
        printf("    A B C D E F G H I J K L M N O P Q R S T U V W X Y Z 0 1 2 3 4 5 6 7 8 9");
        erase_table(alphanum_bar_y_pos + 1, 18);
        locate(0, 22);
        printf("Press [ESC] to return to main menu.");
    }
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    if (check_folder_floppies_db() > 0)
    {
        // Errror. Back to main menu
        return 0; // 0 is go to menu
    }
    if (check_network_connection() > 0)
    {
        // No network connection. Back to main menu
        return 0;
    }

    print_alphanum_bar();
    int padding = 2;
    char nav_arrow_keys[36] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    __uint16_t nav_arrow_keys_index = 0;
    __uint16_t first_time = TRUE;
    char key = 'A';
    long fullkey = 0;
    while (1)
    {
        if ((Bconstat(2) != 0) || first_time)
        {
            print_alphanum_bar();
            if (first_time)
            {
                first_time = FALSE;
            }
            else
            {
                fullkey = Bconin(2);
                key = fullkey & 0xFF;
            }
            if (key == KEY_ESC)
            {
                // Back to main menu
                return 0; // 0 return to menu, no ask
            }
            // upper key case
            if ((key >= 'a') && (key <= 'z'))
            {
                key -= 32;
            }
            if (((key >= 65) && (key <= 90)) || ((key >= 48) && (key <= 57)))
            {
                locate(0, alphanum_bar_y_pos);
                if ((key >= 65) && (key <= 90))
                {
                    for (__uint16_t i = 0; i < padding + (key - 65); i++)
                    {
                        printf("\033C\033C");
                    }
                    nav_arrow_keys_index = key - 65;
                }
                else
                {
                    for (__uint16_t i = 0; i < padding + 26 + (key - 48); i++)
                    {
                        printf("\033C\033C");
                    }
                    nav_arrow_keys_index = 26 + key - 48;
                }
                printf("\033p%c\033q\r", key);
                __uint16_t key16 = ((__uint16_t)key) & 0xFF;
                send_sync_command(QUERY_FLOPPY_DB, &key16, 2, FLOPPYDB_WAIT_TIME, FALSE);

                int num_files = -1;
                __uint32_t db_file_list_mem = (__uint32_t)DB_FILES_LIST_START_ADDRESS;
#ifdef _DEBUG
                printf("Reading db file list from memory address: 0x%08X\r\n", db_file_list_mem);
#endif
                char *file_array = read_files_from_memory((char *)db_file_list_mem);

                if (!file_array)
                {
                    printf("\r\n\r\nNo files found.\r\n");
                }
                else
                {
                    __uint32_t last_key_pressed = 0;
                    __int16_t app_number = display_paginated_content(file_array, get_file_count(file_array), ELEMENTS_PER_PAGE - 4, "Programs", &last_key_pressed);

                    if (app_number >= 0)
                    {
                        // Back to main menu and reset afterwards
                        locate(0, 22);

                        printf("\033KFinding the floppy that contains the program: %d. ", app_number);

                        print_file_at_index(file_array, app_number - 1, 0);

                        printf("\r\nLoading floppy. Wait until the led in the board blinks a 'F' in morse...");

                        send_sync_command(DOWNLOAD_FLOPPY, &app_number, 2, FLOPPYLOAD_WAIT_TIME, TRUE);

                        __uint16_t download_status = *((__uint16_t *)(DB_FILES_LIST_START_ADDRESS));

                        if (download_status == 0)
                        {
                            sleep_seconds(5, FALSE);
                            printf("\r\033KFloppy image file loaded. ");
                            return 1; // different than zero is OK
                        }
                        else
                        {
                            printf("\r\033KError loading floppy image file: %d. Press a key to continue. ", download_status);
                        }
                    }
                    else
                    {
                        if (last_key_pressed == KEY_ESC)
                        {
                            // Back to main menu
                            return 0; // 0 return to menu, no ask
                        }
                        erase_table(alphanum_bar_y_pos + 1, 18);
                        key = last_key_pressed & 0xFF;
                        fullkey = last_key_pressed;
                        first_time = TRUE;
                    }
                }
            }
        }
    }

    return 1; // Positive is OK
}
