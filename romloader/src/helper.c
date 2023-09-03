#include "helper.h"

static __uint16_t spinner_loop = 0;
static char spinner_chars[] = {'\\', '|', '/', '-'};

// Function to convert a character to lowercase
char to_lowercase(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + 32;
    }
    return c;
}

int get_number_within_range(char *prompt, __uint8_t num_items, __uint8_t first_value, char cancel_char, char save_char)
{
    char input[3];
    int number;

    while (1)
    {
        // Prompt the user
        printf(prompt);

        // Get the input as a string (this is to handle empty input)
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            return -1; // Use -1 or another value to indicate that no valid number was received
        }

        // Check if the string is empty or contains only the newline character
        int i = 0;
        while (input[i] != '\0')
        {
            if (input[i] != '\n' && input[i] != ' ')
            {
                break;
            }
            i++;
        }

        if (save_char != '\0')
        {
            if (to_lowercase(input[i]) == to_lowercase(save_char))
            {
                return num_items + first_value; // If returned the number of elements plus first_value, then save command was selected
            }
        }

        if (cancel_char != '\0')
        {
            if (to_lowercase(input[i]) == to_lowercase(cancel_char))
            {
                return -1; // Use -1 or another value to indicate that no valid number was received
            }
        }
        else
        {
            if (input[i] == '\0' || input[i] == '\n')
            {
                return -1; // Use -1 or another value to indicate that no valid number was received
            }
        }

        // Convert the string input to an integer
        if (sscanf(input, "%d", &number) == 1)
        {
            if (number >= first_value && number <= num_items)
            {
                // If the number is within the desired range, return the number
                return number;
            }
        }

        // If out of range or not a valid number, print an error message
        printf("Invalid input! Please enter a number between %d and %d.\n", first_value, num_items);
    }
}

int send_command(__uint16_t command, void *payload, __uint16_t payload_size)
{
    if (payload_size % 2 != 0)
    {
        payload_size++;
    }
    __uint32_t rom3_address = ROM3_MEMORY_START;
    __uint8_t command_header = *((volatile __uint8_t *)(rom3_address + PROTOCOL_HEADER));
    __uint8_t command_code = *((volatile __uint8_t *)(rom3_address + command));
    __uint8_t command_payload_size = *((volatile __uint8_t *)(rom3_address + payload_size)); // Always even!

#ifdef _DEBUG
    printf("ROM3 memory address: 0x%08X\r\n", rom3_address);
    printf("Payload size: %d\r\n", payload_size);
    printf("Command header: 0x%02X\r\n", command_header);
    printf("Command code: 0x%02X\r\n", command_code);
    printf("Command payload size: 0x%02X\r\n", command_payload_size);
#endif

    for (__uint8_t i = 0; i < payload_size; i += 2)
    {
        __uint8_t value = *((volatile __uint8_t *)(rom3_address + *((__uint16_t *)(payload + i))));
#ifdef _DEBUG
        printf("Payload[%i]: 0x%02X\r\n", i, value);
#endif
    }
    return 0;
}

void sleep_seconds(__uint8_t seconds)
{
    for (__uint8_t j = 0; j < seconds; j++)
    {
        // Assuming PAL system; for NTSC, use 60.
        for (__uint8_t i = 0; i < 50; i++)
        {
            spinner(1);
            Vsync(); // Wait for VBL interrupt
        }
    }
}

void spinner(__uint16_t spinner_update_frequency)
{
    if (spinner_loop % spinner_update_frequency == 0)
    {
        printf("\b%c", spinner_chars[(spinner_loop / spinner_update_frequency) % 4]);
    }
    spinner_loop++;
}

void please_wait(char *message, __uint8_t seconds)
{
    printf(message); // Show the message
    printf(" ");     // Leave a space for the spinner
    sleep_seconds(seconds);
    printf("\r\n");
}