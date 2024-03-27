#ifndef CONFIG_H_
#define CONFIG_H_

#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "screen.h"
#include "commands.h"

#define MAX_ENTRIES 26
#define MAX_KEY_LENGTH 20
#define MAX_STRING_VALUE_LENGTH 64

#define PARAM_DOWNLOAD_TIMEOUT_SEC "DOWNLOAD_TIMEOUT_SEC"
#define PARAM_FLOPPY_BOOT_ENABLED "FLOPPY_BOOT_ENABLED"
#define PARAM_FLOPPY_BUFFER_TYPE "FLOPPY_BUFFER_TYPE"
#define PARAM_FLOPPY_DB_URL "FLOPPY_DB_URL"
#define PARAM_FLOPPY_XBIOS_ENABLED "FLOPPY_XBIOS_ENABLED"
#define PARAM_LASTEST_RELEASE_URL "LASTEST_RELEASE_URL"
#define PARAM_MENU_REFRESH_SEC "MENU_REFRESH_SEC"
#define PARAM_NETWORK_STATUS_SEC "NETWORK_STATUS_SEC"
#define PARAM_ROMS_YAML_URL "ROMS_YAML_URL"
#define PARAM_WIFI_AUTH "WIFI_AUTH"
#define PARAM_WIFI_COUNTRY "WIFI_COUNTRY"
#define PARAM_WIFI_PASSWORD "WIFI_PASSWORD"
#define PARAM_WIFI_SCAN_SECONDS "WIFI_SCAN_SECONDS"
#define PARAM_WIFI_SSID "WIFI_SSID"

#define TYPE_INT ((__uint16_t)0)
#define TYPE_STRING ((__uint16_t)1)
#define TYPE_BOOL ((__uint16_t)2)

typedef __uint16_t DataType;

typedef struct
{
    char key[MAX_KEY_LENGTH];
    DataType dataType;
    char value[MAX_STRING_VALUE_LENGTH];
} ConfigEntry;

typedef struct
{
    int magic;
    ConfigEntry entries[MAX_ENTRIES];
    size_t count;
} ConfigData;

#ifdef _DEBUG
static ConfigData config_data_example = {
    .magic = 0x12345678,
    .count = 3,
    .entries = {
        {
            .key = "KEY1",
            .dataType = TYPE_INT,
            .value = "123",
        },
        {
            .key = "KEY2",
            .dataType = TYPE_STRING,
            .value = "Hello World",
        },
        {
            .key = "KEY3",
            .dataType = TYPE_BOOL,
            .value = "true",
        },
    },
};
#endif

#define FALSE 0
#define TRUE 1

#define NO_SPINNING 0
#define SPINNING 1
#define COUNTDOWN 2

#define STATUS_STRING_BUFFER_SIZE 80 // Buffer size to display

ConfigEntry *get_config_entry(char *key);
__uint16_t read_config();
__uint16_t configuration();
__uint16_t is_delay_option_enabled();
__uint16_t toggle_delay_option(void);
void init_config();

#endif /* CONFIG_H_ */
