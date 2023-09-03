#ifndef HELPER_H_
#define HELPER_H_

#include <sys/types.h>
#include <stdio.h>
#include <osbind.h>

#include "config.h"

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
static __uint16_t protocol_example[] = {
    0x1234,
    0x5678,
    0x9ABC,
    0xDEF0,
};
#define ROM4_MEMORY_START &protocol_example[0]
#define ROM3_MEMORY_START &protocol_example[0]
#define FILE_LIST_START_ADDRESS &file_list_example[0]
#define CONFIG_START_ADDRESS &config_data_example
#define PROTOCOL_HEADER 0x0000
#define WAIT_TIME 0
#else
#define ROM4_MEMORY_START 0xFA0000
#define ROM3_MEMORY_START 0xFB0000
#define FILE_LIST_START_ADDRESS (ROM3_MEMORY_START - 4096)
#define CONFIG_START_ADDRESS (ROM3_MEMORY_START - 4096)
#define PROTOCOL_HEADER 0xABCD
#define WAIT_TIME 2
#endif

#define PRINT_APP_HEADER(version)                                                                                         \
    do                                                                                                                    \
    {                                                                                                                     \
        clearHome();                                                                                                      \
        locate(0, 0);                                                                                                     \
        printf("\033pATARI ST SIDECART CONFIGURATOR. V%s - (C)2023 Diego Parrilla / @sidecartridge\033q\r\n", version); \
    } while (0)

int get_number_within_range(char *prompt, __uint8_t num_items, __uint8_t first_value, char cancel_char, char save_char);
int send_command(__uint16_t command, void *payload, __uint16_t payload_size);
void please_wait(char *message, __uint8_t seconds);
void sleep_seconds(__uint8_t seconds);
void spinner(__uint16_t spinner_update_frequency);

#endif /* HELPER_H_ */