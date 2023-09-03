#include "romselector.h"

//================================================================
// ROM selector

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

__uint8_t rom_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    send_command(LIST_ROMS, NULL, 0);

    please_wait("Loading available ROM images...", WAIT_TIME);

    int num_files = -1;
    __uint32_t file_list_mem = FILE_LIST_START_ADDRESS;

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", file_list_mem);
#endif
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

        while (index <= num_files)
        {
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

    char *prompt;
    asprintf(&prompt, "Enter the ROM to load (1 to %d): ", num_files);

    __int16_t rom_number = (__int16_t)get_number_within_range(prompt, num_files, 1, '\0', '\0');

    if (rom_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    printf("\r\n");
    printf("\r\n");

    printf("Selected the ROM file: %d. ", rom_number);

    print_file_at_index(file_array, rom_number - 1);

    send_command(LOAD_ROM, &rom_number, 2);

    printf("ROM file loaded. ");

    return 1; // Positive is OK
}