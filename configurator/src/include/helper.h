#ifndef HELPER_H_
#define HELPER_H_

#include <sys/types.h>
#include <stdio.h>

#include "config.h"

#define EXCHANGE_BUFFER_SIZE 0 // 0 means that the buffer is the full ROM3 memory
#ifdef _DEBUG
static char file_list_example[] = {
    'd', 'o', 'c', 'u', 'm', 'e', 'n', 't', '.', 't', 'x', 't', 0,
    'i', 'm', 'a', 'g', 'e', '.', 'j', 'p', 'g', 0,
    'n', 'o', 't', 'e', 's', '.', 'd', 'o', 'c', 'x', 0,
    'm', 'u', 's', 'i', 'c', '.', 'm', 'p', '3', 0,
    'p', 'r', 'e', 's', 'e', 'n', 't', 'a', 't', 'i', 'o', 'n', '.', 'p', 'p', 't', 'x', 0,
    's', 'p', 'r', 'e', 'a', 'd', 's', 'h', 'e', 'e', 't', '.', 'x', 'l', 's', 'x', 0,
    0, 0, 0, 0, 0, 0 // Two consecutive zeroes at the end
};

static char network_file_list_example[] = {
    'D', 'o', 'c', 'u', 'm', 'e', 'n', 't', ' ', '1', 0,
    'i', 'm', 'a', 'g', 'e', 'F', 'i', 'l', 'e', 0,
    'N', 'o', 't', 'e', 's', ' ', 'A', 'B', 'C', 0,
    'M', 'y', ' ', 'M', 'u', 's', 'i', 'c', 0,
    'P', 'o', 'w', 'e', 'r', ' ', 'P', 'r', 'e', 's', 'e', 'n', 't', 'a', 't', 'i', 'o', 'n', 0,
    'E', 'x', 'c', 'e', 'l', ' ', 'S', 'h', 'e', 'e', 't', 0,
    0, 0 // A double zero at the end to indicate the end of the list
};

static __uint16_t protocol_example[] = {
    0x1234,
    0x5678,
    0x9ABC,
    0xDEF0,
};
static char *latest_release_example = "v1.0.0\0";

static __uint32_t random_seed = 0x12345678;
static __uint32_t random_number = 0x0;
#define ROM4_MEMORY_START &protocol_example[0]
#define ROM3_MEMORY_START &protocol_example[0]
#define RANDOM_SEED_ADDRESS &random_seed
#define RANDOM_NUMBER_ADDRESS &random_number
#define FILE_LIST_START_ADDRESS &file_list_example[0]
#define NETWORK_FILE_LIST_START_ADDRESS &network_file_list_example[0]
#define CONFIG_START_ADDRESS &config_data_example
#define NETWORK_START_ADDRESS &wifi_scan_data_example
#define DB_FILES_LIST_START_ADDRESS &network_file_list_example[0]
#define CONNECTION_STATUS_START_ADDRESS &connection_data_example
#define SD_CARD_DATA_START_ADDRESS &sdCardDataExample
#define LATEST_RELEASE_START_ADDRESS &latest_release_example
#define PROTOCOL_HEADER 0x0000
#define WAIT_TIME 0
#define NETWORK_WAIT_TIME 2
#define NETWORKLOAD_WAIT_TIME 10
#define ROMS_JSON_WAIT_TIME 2
#define ROMSLOAD_WAIT_TIME 10
#define ROMS_LOAD_WAIT_TIME_MIN 10
#define FLOPPYDB_WAIT_TIME 2
#define FLOPPYLOAD_WAIT_TIME 10
#define FLOPPYLIST_WAIT_TIME 2
#define CONFIG_WAIT_TIME 2
#define RTC_WAIT_TIME 2
#define STORAGE_WAIT_TIME 2
#define NETWORK_DATA_WAIT_TIME 2
#define ELEMENTS_PER_PAGE 10
#define COMMAND_NUM_RETRIES 1
#else
#define ROM4_MEMORY_START 0xFA0000
#define ROM3_MEMORY_START 0xFB0000
#define RANDOM_SEED_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE - RANDOM_NUMBER_SIZE)
#define RANDOM_NUMBER_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define FILE_LIST_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define NETWORK_FILE_LIST_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define DB_FILES_LIST_START_ADDRESS (RANDOM_NUMBER_ADDRESS + RANDOM_NUMBER_SIZE)
#define CONFIG_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define NETWORK_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define CONNECTION_STATUS_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define SD_CARD_DATA_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE)
#define LATEST_RELEASE_START_ADDRESS (ROM3_MEMORY_START - EXCHANGE_BUFFER_SIZE - 256)
#define PROTOCOL_HEADER 0xABCD
#define WAIT_TIME 2
#define NETWORK_WAIT_TIME 10
#define NETWORKLOAD_WAIT_TIME 40
#define ROMS_JSON_WAIT_TIME 20
#define ROMSLOAD_WAIT_TIME 40
#define ROMS_LOAD_WAIT_TIME_MIN 10
#define FLOPPYDB_WAIT_TIME 20
#define FLOPPYLOAD_WAIT_TIME 40
#define FLOPPYLIST_WAIT_TIME 10
#define CONFIG_WAIT_TIME 30
#define RTC_WAIT_TIME 10
#define STORAGE_WAIT_TIME 1
#define NETWORK_DATA_WAIT_TIME 1
#define ELEMENTS_PER_PAGE 17
#define COMMAND_NUM_RETRIES 3
#endif

#define RANDOM_NUMBER_SIZE 4  // 4 bytes
#define _VBLOCK_ADDRESS 0x462 // Number of vertical blanks since boot

#define KEY_UP_ARROW 0x480000
#define KEY_DOWN_ARROW 0x500000
#define KEY_LEFT_ARROW 0x4B0000
#define KEY_RIGHT_ARROW 0x4D0000
#define KEY_RETURN 0x1C000D
#define KEY_ENTER 0x72000D
#define KEY_ESC 0x1001B

#define PARAM_DOWNLOAD_TIMEOUT_SEC "DOWNLOAD_TIMEOUT_SEC"
#define PARAM_MENU_REFRESH_SEC "MENU_REFRESH_SEC"

#define PRINT_APP_HEADER(version)                                                                                      \
    do                                                                                                                 \
    {                                                                                                                  \
        clearHome();                                                                                                   \
        locate(0, 0);                                                                                                  \
        printf("\033p  SidecarTridge Multi-Device Configurator v%s - (C)2023-24 @sidecartridge   \033q\r\n", version); \
    } while (0)

#define max(a, b) ((a) > (b) ? (a) : (b))

void flush_kbd(void);
void press_key(char *message);
int get_number_within_range(char *prompt, __uint16_t num_items, __uint16_t first_value, char cancel_char);
int send_async_command(__uint16_t command, void *payload, __uint16_t payload_size);
int send_sync_command(__uint16_t command, void *payload, __uint16_t payload_size, __uint32_t timeout, __uint16_t show_spinner);
void please_wait(char *message, __uint16_t seconds);
void please_wait_silent(__uint16_t seconds);
void sleep_seconds(__uint16_t seconds, __uint16_t silent);
void spinner(__uint16_t spinner_update_frequency);
char *read_files_from_memory(char *memory_location);
__uint16_t get_file_count(char *file_array);
char *get_file_at_index(char *file_array, __uint16_t index);
char *print_file_at_index(char *current_ptr, __uint16_t index, int num_columns);
__uint16_t get_index_of_filename(const char *file_array, char *filename);
int display_paginated_content(char *file_array, int num_files, int page_size, char *item_name, __uint32_t *keypress);
void print_centered(const char *str, int screen_width);
char *prepend_file_to_array(char *file_array, const char *new_file);
void read_string(char *string, size_t max_length);
__uint16_t get_download_timeout();

#endif /* HELPER_H_ */