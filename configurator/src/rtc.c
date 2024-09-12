#include "include/rtc.h"

static void set_rtc_type(char *type)
{
    ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
    strncpy(entry->key, PARAM_RTC_TYPE, MAX_KEY_LENGTH);
    strncpy(entry->value, type, MAX_STRING_VALUE_LENGTH);
    entry->dataType = TYPE_STRING;
    send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), RTC_WAIT_TIME, FALSE);
    free(entry);
    send_sync_command(SAVE_CONFIG, NULL, 0, RTC_WAIT_TIME, TRUE);
    __uint16_t err = read_config();
}

__uint16_t rtc_menu()
{
    __uint8_t display = TRUE;
    while (TRUE)
    {
        if (display)
        {
            PRINT_APP_HEADER(VERSION);
            printf("\r\n");
            printf("\r\n");
            printf("\r\n");

#ifndef _DEBUG
            if (check_network_connection() > 0)
            {
                // No network connection. Back to main menu
                return 0;
            }
#endif
            ConfigEntry *ntp_server_host_entry = get_config_entry(PARAM_RTC_NTP_SERVER_HOST);
            ConfigEntry *ntp_server_port_entry = get_config_entry(PARAM_RTC_NTP_SERVER_PORT);
            ConfigEntry *rtc_type_entry = get_config_entry(PARAM_RTC_TYPE);
            ConfigEntry *rtc_utc_offset_entry = get_config_entry(PARAM_RTC_UTC_OFFSET);

            char *ntp_server_host = ntp_server_host_entry != NULL ? ntp_server_host_entry->value : "NOT FOUND";
            int ntp_server_port = ntp_server_port_entry != NULL ? atoi(ntp_server_port_entry->value) : 123;
            char *rtc_type = rtc_type_entry != NULL ? rtc_type_entry->value : "NOT FOUND";
            char *rtc_utc_offset = rtc_utc_offset_entry != NULL ? rtc_utc_offset_entry->value : "+0";

            printf("NTP Server:\t%s:%d\r\n", ntp_server_host, ntp_server_port);
            printf("RTC Type:\t%s\r\n", rtc_type);
            printf("RTC UTC Offset:\t%s\r\n", rtc_utc_offset);
            printf("\r\n");
            printf("Options:\r\n\n");
            printf("[1] - Set SIDECART RTC with a custom firmware\r\n");
            printf("[2] - Set DALLAS 1216 RTC with cartridge emulation\r\n");
            printf("[U] - Change UTC offset\r\n");
            printf("[H] - Change NTP server host\r\n");
            printf("[P] - Change NTP server port\r\n");
            printf("\r\n[B] - Boot emulator\r\n");
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
                // Back to main menu
                return 0; // 0 return to menu, no ask
            }
            // Check if the input is 'U' or 'u'
            if ((key == 'U') || (key == 'u'))
            {
                printf("\r\n\n\033KEnter the offset from UTC (from -13 to +15, other values to default): ");
                char offset[32] = {0};
                read_string(offset, 32);

                int offsetNumber = 0;
                if (strlen(offset) > 0)
                {
                    offsetNumber = atoi(offset);
                    if ((offsetNumber < -13) || (offsetNumber > 15))
                    {
                        offsetNumber = 0;
                    }
                }
                else
                {
                    offsetNumber = 0;
                }

                if (offsetNumber >= 0)
                {
                    sprintf(offset, "+%d", offsetNumber);
                }
                else
                {
                    sprintf(offset, "%d", offsetNumber);
                }

                ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
                strncpy(entry->key, PARAM_RTC_UTC_OFFSET, MAX_KEY_LENGTH);
                strncpy(entry->value, offset, MAX_STRING_VALUE_LENGTH);
                entry->dataType = TYPE_STRING;
                send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), RTC_WAIT_TIME, FALSE);
                free(entry);

                send_sync_command(SAVE_CONFIG, NULL, 0, RTC_WAIT_TIME, TRUE);
                __uint16_t err = read_config();

                display = TRUE;
            }
            // Check if the input is 'H' or 'h'
            if ((key == 'H') || (key == 'h'))
            {
                printf("\r\n\n\033KEnter NTP host name (empty for default): ");
                char host[128] = {0};
                read_string(host, 128);

                if (strlen(host) == 0)
                {
                    strncpy(host, "pool.ntp.org", 128);
                }

                ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
                strncpy(entry->key, PARAM_RTC_NTP_SERVER_HOST, MAX_KEY_LENGTH);
                strncpy(entry->value, host, MAX_STRING_VALUE_LENGTH);
                entry->dataType = TYPE_STRING;
                send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), RTC_WAIT_TIME, FALSE);
                free(entry);

                send_sync_command(SAVE_CONFIG, NULL, 0, RTC_WAIT_TIME, TRUE);
                __uint16_t err = read_config();

                display = TRUE;
            }
            // Check if the input is 'P' or 'p'
            if ((key == 'P') || (key == 'p'))
            {
                printf("\r\n\n\033KEnter NTP host port (empty or invalid for default): ");
                char port[32] = {0};
                read_string(port, 32);

                int portNumber = 123;
                if (strlen(port) > 0)
                {
                    portNumber = atoi(port);
                    if ((portNumber <= 0) || (portNumber > 65535))
                    {
                        portNumber = 123;
                    }
                }
                else
                {
                    portNumber = 123;
                }

                sprintf(port, "%d", portNumber);

                ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
                strncpy(entry->key, PARAM_RTC_NTP_SERVER_PORT, MAX_KEY_LENGTH);
                strncpy(entry->value, port, MAX_STRING_VALUE_LENGTH);
                entry->dataType = TYPE_INT;
                send_sync_command(PUT_CONFIG_INTEGER, entry, sizeof(ConfigEntry), RTC_WAIT_TIME, FALSE);
                free(entry);

                send_sync_command(SAVE_CONFIG, NULL, 0, RTC_WAIT_TIME, TRUE);
                __uint16_t err = read_config();

                display = TRUE;
            }
            // Check if the input is 'B' or 'b'
            if ((key == 'B') || (key == 'b'))
            {
                ConfigEntry *entry = (ConfigEntry *)malloc(sizeof(ConfigEntry));
                strncpy(entry->key, PARAM_BOOT_FEATURE, MAX_KEY_LENGTH);
                strncpy(entry->value, "RTC_EMULATOR", MAX_STRING_VALUE_LENGTH);
                entry->dataType = TYPE_STRING;
                send_sync_command(PUT_CONFIG_STRING, entry, sizeof(ConfigEntry), RTC_WAIT_TIME, FALSE);
                free(entry);
                send_sync_command(SAVE_CONFIG, NULL, 0, RTC_WAIT_TIME, FALSE);
                send_sync_command(BOOT_RTC, NULL, 0, RTC_WAIT_TIME, FALSE);
                return 1; // 1 means restart boot
            }
            // Check if the input is '1', SIDECART
            if (key == '1')
            {
                set_rtc_type("SIDECART");
                display = TRUE;
            }
            // Check if the input is '2', DALLAS
            if (key == '2')
            {
                set_rtc_type("DALLAS");
                display = TRUE;
            }
        }
    }
}
