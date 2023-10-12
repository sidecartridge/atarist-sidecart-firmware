#ifndef CONFIG_H_
#define CONFIG_H_

#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "helper.h"
#include "screen.h"
#include "commands.h"

#define MAX_ENTRIES 20
#define MAX_KEY_LENGTH 20
#define MAX_STRING_VALUE_LENGTH 64

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

__uint8_t configuration();
bool is_delay_option_enabled();
__uint8_t toggle_delay_option(void);

#endif /* CONFIG_H_ */