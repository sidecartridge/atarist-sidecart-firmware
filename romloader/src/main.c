
#include <sys/types.h>
#include "screen.h"

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
#define PROTOCOL_HEADER 0x0000
#else
#define ROM4_MEMORY_START 0xFA0000
#define ROM3_MEMORY_START 0xFB0000
#define FILE_LIST_START_ADDRESS (ROM3_MEMORY_START - 4096)
#define PROTOCOL_HEADER 0xABCD
#endif

static char *
read_files_from_memory(__uint8_t *memory_location)
{
    __uint8_t *current_ptr = memory_location;
    __uint16_t total_size = 0;

    // Calculate total size required
    while (1)
    {
        // If we encounter two consecutive 0x00 bytes, it means we're at the end of the list
        if (*current_ptr == 0x00 && *(current_ptr + 1) == 0x00)
        {
            total_size += 2; // For two consecutive null characters
            break;
        }

        total_size++;
        current_ptr++;
    }

    // Allocate memory for the file list
    char *output_array = (char *)malloc(total_size * sizeof(char));
    if (!output_array)
    {
        // Allocation failed
        return NULL;
    }

    // Copy the memory content to the output array
    for (int i = 0; i < total_size; i++)
    {
        output_array[i] = memory_location[i];
    }

    return output_array;
}

static __uint8_t get_file_count(char *file_array)
{
    __uint8_t count = 0;
    char *current_ptr = file_array;

    while (*current_ptr)
    { // as long as we don't hit the double null terminator
        count++;

        // skip past the current filename to the next
        while (*current_ptr)
            current_ptr++;
        current_ptr++; // skip the null terminator for the current filename
    }

    return count;
}

static void print_file_at_index(char *file_array, __uint8_t index)
{
    __uint8_t current_index = 0;
    char *current_ptr = file_array;

    while (*current_ptr)
    { // As long as we don't hit the double null terminator
        if (current_index == index)
        {
            while (*current_ptr)
            {
                putchar(*current_ptr);
                current_ptr++;
            }
            putchar('\r');
            putchar('\n');
            return;
        }

        // Skip past the current filename to the next
        while (*current_ptr)
            current_ptr++;
        current_ptr++; // Skip the null terminator for the current filename

        current_index++;
    }

    printf("No file found at index %d\r\n", index);
}

static int get_number_within_range(__uint8_t num_files)
{
    char input[3];
    int number;

    while (1)
    {
        // Prompt the user
        printf("Enter the ROM to load (1 to %d): ", num_files);

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

        if (input[i] == '\0' || input[i] == '\n')
        {
            return -1; // Use -1 or another value to indicate that no valid number was received
        }

        // Convert the string input to an integer
        if (sscanf(input, "%d", &number) == 1)
        {
            if (number >= 1 && number <= num_files)
            {
                // If the number is within the desired range, return the number
                return number;
            }
        }

        // If out of range or not a valid number, print an error message
        printf("Invalid input! Please enter a number between 1 and %d.\n", num_files);
    }
}

//================================================================
// Main program
static int run()
{
    ScreenContext screenContext;
    initScreenContext(&screenContext);

    if (screenContext.savedResolution == HIGH_RES)
    {
        initHighResolution();
    }
    else
    {
        __uint16_t palette[4] = {0xFFF, 0x000, 0x000, 0x000};
        initMedResolution(palette);
    }

    clearHome();
    printf("ATARI ST SIDECART ROM SELECTOR. V%s - (C)2023 Diego Parrilla / @soyparrilla\r\n", VERSION);

    printf("\r\n");

    int num_files = -1;
    __uint32_t file_list_mem = FILE_LIST_START_ADDRESS;
    printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);

    char *file_array = read_files_from_memory((__uint8_t *)file_list_mem);

    if (!file_array)
    {
        printf("No files found. Check if you have ROMS copied in your microSD card!\r\n");
        printf("Press any key to exit...\r\n");
    }
    else
    {

        num_files = get_file_count(file_array);
        printf("Number of ROM files found: %d\r\n\r\n", num_files);

        int index = 1; // Index counter
        char *current_ptr = file_array;

        while (1)
        {
            // If we encounter two consecutive 0x00 bytes, it means we're at the end of the list
            if (*current_ptr == 0x00 && *(current_ptr + 1) == 0x00)
            {
                break;
            }

            // Print the index number
            printf("%d. ", index++);

            // Print the current filename
            while (*current_ptr != 0x00)
            {
                putchar(*current_ptr);
                current_ptr++;
            }

            // Move past the 0x00 byte to the next filename
            current_ptr++;

            // Print a newline for the next filename
            putchar('\r');
            putchar('\n');
        }
    }

    __uint8_t rom_number = (__int8_t)get_number_within_range(num_files);
    printf("\r\n");
    printf("\r\n");

    printf("Selected the ROM file: %d. ", rom_number);

    print_file_at_index(file_array, rom_number - 1);

    // Here comes the commands stuff
    __uint32_t rom3_address = ROM3_MEMORY_START;
    __uint8_t command_header = *((volatile __uint8_t *)(rom3_address + PROTOCOL_HEADER));
    __uint8_t command_code = *((volatile __uint8_t *)(rom3_address + 0x0001));
    __uint8_t command_payload_size = *((volatile __uint8_t *)(rom3_address + 0x0002)); // Always even
    __uint8_t command_payload = *((volatile __uint8_t *)(rom3_address + rom_number));

#ifdef _DEBUG
    printf("ROM3 memory address: 0x%08X\r\n", rom3_address);
    printf("Command header: 0x%02X\r\n", command_header);
    printf("Command code: 0x%02X\r\n", command_code);
    printf("Command payload size: 0x%02X\r\n", command_payload_size);
    printf("Command payload: 0x%02X\r\n", command_payload);
#endif

    free(file_array);

    printf("ROM file loaded. Now you can reset or power cycle your Atari ST computer.\r\n");

#ifdef _DEBUG
    getchar();
#else
    while (1)
    {
        getchar();
    };
#endif

    restoreResolutionAndPalette(&screenContext);
}

//================================================================
// Standard C entry point
int main(int argc, char *argv[])
{
    // switching to supervisor mode and execute run()
    // needed because of direct memory access for reading/writing the palette
    Supexec(&run);
}
