#ifndef CONFIG_H_
#define CONFIG_H_

#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "helper.h"
#include "screen.h"
#include "commands.h"

#define MAX_ENTRIES 47
#define MAX_KEY_LENGTH 20
#define MAX_STRING_VALUE_LENGTH 64

#define PARAM_BOOT_FEATURE "BOOT_FEATURE"
#define PARAM_CONFIGURATOR_DARK "CONFIGURATOR_DARK"
#define PARAM_DELAY_ROM_EMULATION "DELAY_ROM_EMULATION"
#define PARAM_DOWNLOAD_TIMEOUT_SEC "DOWNLOAD_TIMEOUT_SEC"
#define PARAM_FILE_COUNT_ENABLED "FILE_COUNT_ENABLED"
#define PARAM_FLOPPY_BOOT_ENABLED "FLOPPY_BOOT_ENABLED"
#define PARAM_FLOPPY_BUFFER_TYPE "FLOPPY_BUFFER_TYPE"
#define PARAM_FLOPPY_DB_URL "FLOPPY_DB_URL"
#define PARAM_FLOPPY_IMAGE_A "FLOPPY_IMAGE_A"
#define PARAM_FLOPPY_IMAGE_B "FLOPPY_IMAGE_B"
#define PARAM_FLOPPIES_FOLDER "FLOPPIES_FOLDER"
#define PARAM_FLOPPY_NET_ENABLED "FLOPPY_NET_ENABLED"
#define PARAM_FLOPPY_NET_TOUT_SEC "FLOPPY_NET_TOUT_SEC"
#define PARAM_FLOPPY_XBIOS_ENABLED "FLOPPY_XBIOS_ENABLED"
#define PARAM_GEMDRIVE_BUFF_TYPE "GEMDRIVE_BUFF_TYPE"
#define PARAM_GEMDRIVE_DRIVE "GEMDRIVE_DRIVE"
#define PARAM_GEMDRIVE_FOLDERS "GEMDRIVE_FOLDERS"
#define PARAM_GEMDRIVE_RTC "GEMDRIVE_RTC"
#define PARAM_GEMDRIVE_TIMEOUT_SEC "GEMDRIVE_TIMEOUT_SEC"
#define PARAM_GEMDRIVE_FAKEFLOPPY "GEMDRIVE_FAKEFLOPPY"
#define PARAM_HOSTNAME "HOSTNAME"
#define PARAM_LASTEST_RELEASE_URL "LASTEST_RELEASE_URL"
#define PARAM_MENU_REFRESH_SEC "MENU_REFRESH_SEC"
#define PARAM_NETWORK_STATUS_SEC "NETWORK_STATUS_SEC"
#define PARAM_ROMS_CSV_URL "ROMS_CSV_URL"
#define PARAM_ROMS_FOLDER "ROMS_FOLDER"
#define PARAM_ROMS_YAML_URL "ROMS_YAML_URL"
#define PARAM_RTC_NTP_SERVER_HOST "RTC_NTP_SERVER_HOST"
#define PARAM_RTC_NTP_SERVER_PORT "RTC_NTP_SERVER_PORT"
#define PARAM_RTC_TYPE "RTC_TYPE"
#define PARAM_RTC_UTC_OFFSET "RTC_UTC_OFFSET"
#define PARAM_SAFE_CONFIG_REBOOT "SAFE_CONFIG_REBOOT"
#define PARAM_SD_MASS_STORAGE "SD_MASS_STORAGE"
#define PARAM_SD_BAUD_RATE_KB "SD_BAUD_RATE_KB"
#define PARAM_WIFI_AUTH "WIFI_AUTH"
#define PARAM_WIFI_CONNECT_TIMEOUT "WIFI_CONNECT_TIMEOUT"
#define PARAM_WIFI_COUNTRY "WIFI_COUNTRY"
#define PARAM_WIFI_DHCP "WIFI_DHCP"
#define PARAM_WIFI_DNS "WIFI_DNS"
#define PARAM_WIFI_IP "WIFI_IP"
#define PARAM_WIFI_NETMASK "WIFI_NETMASK"
#define PARAM_WIFI_GATEWAY "WIFI_GATEWAY"
#define PARAM_WIFI_PASSWORD "WIFI_PASSWORD"
#define PARAM_WIFI_POWER "WIFI_POWER"
#define PARAM_WIFI_RSSI "WIFI_RSSI"
#define PARAM_WIFI_SCAN_SECONDS "WIFI_SCAN_SECONDS"
#define PARAM_WIFI_SSID "WIFI_SSID"

#define TYPE_INT ((__uint16_t)0)
#define TYPE_STRING ((__uint16_t)1)
#define TYPE_BOOL ((__uint16_t)2)
#define TYPE_CHAR ((__uint16_t)3)

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

typedef struct _osheader
{
    __uint16_t os_entry;
    __uint16_t os_version;
    void *reseth;
    struct _osheader *os_beg;
    char *os_end;
    char *os_rsv1;
    char *os_magic;
    __int32_t os_date;
    __uint16_t os_conf;
    __uint16_t os_dosdate;
    /* Available as of TOS 1.02 */
    char **p_root;
    char **p_kbshift;
    char **p_run;
    char *p_rsv2;
} OSHEADER;

#define _sysbase ((OSHEADER **)0x4F2)

char *p_kbshift;
#define Kbstate() *p_kbshift

ConfigEntry *get_config_entry(char *key);
__uint16_t read_config();
__uint16_t configuration();
char *read_input(const char *input_text, __uint16_t type);
__uint16_t is_delay_option_enabled();
__uint16_t toggle_delay_option(void);
void init_config();

OSHEADER *GetROMSysbase(void);
void init_kb();

#endif /* CONFIG_H_ */
