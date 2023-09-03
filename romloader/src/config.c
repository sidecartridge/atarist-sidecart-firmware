#include "config.h"

static void print_table(ConfigData *configData)
{
    printf("+----+----------------------+------------------------------------------+-------+\r\n");
    printf("|    |       Key            |                Value                     | Type  |\r\n");
    printf("+----+----------------------+------------------------------------------+-------+\r\n");

    for (size_t i = 0; i < configData->count; i++)
    {
        char valueStr[40]; // Buffer to format the value

        switch (configData->entries[i].dataType)
        {
        case TYPE_INT:
        case TYPE_STRING:
        case TYPE_BOOL:
            snprintf(valueStr, sizeof(valueStr), "%s", configData->entries[i].value);
            break;
        default:
            snprintf(valueStr, sizeof(valueStr), "Unknown");
            break;
        }

        char *typeStr;
        switch (configData->entries[i].dataType)
        {
        case TYPE_INT:
            typeStr = "INT";
            break;
        case TYPE_STRING:
            typeStr = "STRING";
            break;
        case TYPE_BOOL:
            typeStr = "BOOL";
            break;
        default:
            typeStr = "???";
            break;
        }

        printf("| %-2d | %-20s | %-40s | %-8s |\r\n", i + 1, configData->entries[i].key, valueStr, typeStr);
    }

    printf("+----+----------------------+------------------------------------------+----------+\r\n");
}

ConfigData load_all_entries()
{
    __uint8_t count = 0;
    __uint32_t currentAddress = CONFIG_START_ADDRESS;
    ConfigData configData = {
        .magic = *((volatile __uint32_t *)currentAddress),
        .count = 0,
    };
    currentAddress += sizeof(__uint32_t);
    while (configData.count < MAX_ENTRIES)
    {
        ConfigEntry entry;
        memcpy(&entry, (void *)currentAddress, sizeof(ConfigEntry));

        currentAddress += sizeof(ConfigEntry);

        // Check for the end of the config entries
        if (entry.key[0] == '\0')
        {
            break; // Exit the loop if we encounter a key length of 0 (end of entries)
        }
        configData.entries[configData.count] = entry;
        configData.count++;
    }
    return configData;
}

static char *read_input(__uint16_t type)
{
    char buffer[MAX_STRING_VALUE_LENGTH + 2]; // +1 for '\0' and +1 to check overflow
    char *result = (char *)malloc(MAX_STRING_VALUE_LENGTH + 1);

    if (!result)
    {
        printf("Memory allocation error.\r\n");
        exit(1);
    }

    // Displaying a prompt based on type
    switch (type)
    {
    case TYPE_INT:
        printf("Please enter an integer value: ");
        break;
    case TYPE_STRING:
        printf("Please enter a string (max %d characters): ", MAX_STRING_VALUE_LENGTH);
        break;
    case TYPE_BOOL:
        printf("Please enter a boolean value (true or false): ");
        break;
    }

    while (1)
    { // Repeat until valid input
        fgets(buffer, sizeof(buffer), stdin);

        // Check if the input was too long
        if (buffer[strlen(buffer) - 1] != '\n')
        {
            printf("Input too long. Please try again.\r\n");
            while (getchar() != '\n')
                ; // Clear the input buffer
            continue;
        }
        buffer[strlen(buffer) - 1] = '\0'; // Remove newline character

        if (type == TYPE_INT)
        {
            int valid = 1;
            for (size_t i = 0; buffer[i]; i++)
            {
                if (buffer[i] < '0' || buffer[i] > '9')
                {
                    valid = 0;
                    break;
                }
            }

            if (valid)
            {
                strncpy(result, buffer, MAX_STRING_VALUE_LENGTH);
                break;
            }
            else
            {
                printf("Invalid integer. Please enter a valid integer: ");
            }
        }
        else if (type == TYPE_STRING)
        {
            if (strlen(buffer) <= MAX_STRING_VALUE_LENGTH)
            {
                strncpy(result, buffer, MAX_STRING_VALUE_LENGTH);
                break;
            }
            else
            {
                printf("String too long. Please enter a string of maximum %d chars: ", MAX_STRING_VALUE_LENGTH);
            }
        }
        else if (type == TYPE_BOOL)
        {
            if (strcasecmp(buffer, "true") == 0 || strcasecmp(buffer, "false") == 0)
            {
                strncpy(result, buffer, MAX_STRING_VALUE_LENGTH);
                break;
            }
            else
            {
                printf("Invalid boolean. Please enter a boolean value (true or false): ");
            }
        }
    }

    return result;
}

__uint8_t configuration()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    send_command(GET_CONFIG, NULL, 0);

    please_wait("Loading configuration...", WAIT_TIME);

    ConfigData configData = load_all_entries();

    while (1)
    {
        print_table(&configData);

        char *prompt;
        asprintf(&prompt, "Enter the parameter to modify (1 to %d), [C]ancel or [S]ave: ", configData.count);

        int param_index = get_number_within_range(prompt, configData.count, 1, 'C', 'S');

        if (param_index <= 0)
        {
            // Back to main menu
            return 0; // 0 means back to main menu
        }

        if (param_index == configData.count + 1)
        {
            // Save button was pressed
            // Save the stuff to the flash
            send_command(SAVE_CONFIG, NULL, 0);
            please_wait("Saving configuration...", WAIT_TIME);
            return 1; // Positive is OK
        }

        param_index = param_index - 1;

        printf("\r\n%s = %s\r\r\n", configData.entries[param_index].key, configData.entries[param_index].value);
        char *input = read_input(configData.entries[param_index].dataType);
        printf("\r\n");

        printf("The input is: %s\r\n", input);
        strncpy(configData.entries[param_index].value, input, MAX_STRING_VALUE_LENGTH);

        switch (configData.entries[param_index].dataType)
        {
        case TYPE_INT:
            send_command(PUT_CONFIG_INTEGER, &configData.entries[param_index], sizeof(ConfigEntry));
            break;
        case TYPE_BOOL:
            send_command(PUT_CONFIG_BOOL, &configData.entries[param_index], sizeof(ConfigEntry));
            break;
        case TYPE_STRING:
            send_command(PUT_CONFIG_STRING, &configData.entries[param_index], sizeof(ConfigEntry));
            break;
        default:
            printf("Unknown data type.\r\n");
            break;
        }
        please_wait("Saving configuration...", WAIT_TIME);

        free(input);
    }
}