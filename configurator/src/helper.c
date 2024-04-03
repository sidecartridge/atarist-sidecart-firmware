#include "include/helper.h"

static __uint16_t spinner_loop = 0;
static char spinner_chars[] = {'\\', '|', '/', '-'};

void flush_kbd(void)
{
    // while (Bconstat(2) != 0)
    // {
    //     (void)Bconin(2);
    // }
    while (Cconis() != 0)
    {
        (void)Cnecin();
    }
}

void press_key(char *message)
{
    if (message != NULL)
    {
        Cconws(message);
    }
    flush_kbd();
    (void)Bconin(2);
}

// Function to convert a character to lowercase
char to_lowercase(char c)
{
    if (c >= 'A' && c <= 'Z')
    {
        return c + 32;
    }
    return c;
}

__uint16_t get_download_timeout()
{
    ConfigEntry *download_timeout_secs = get_config_entry(PARAM_DOWNLOAD_TIMEOUT_SEC);
    __uint16_t value = ROMSLOAD_WAIT_TIME;
    if (download_timeout_secs != NULL)
    {
        value = atoi(download_timeout_secs->value);
    }
    if (value < ROMS_LOAD_WAIT_TIME_MIN)
    {
        value = ROMS_LOAD_WAIT_TIME_MIN;
    }
    return value;
}

int get_number_within_range(char *prompt, __uint16_t num_items, __uint16_t first_value, char cancel_char)
{
    char input[3];
    int number;

    while (1)
    {
        // Prompt the user
        Cconws(prompt);

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

int send_async_command(__uint16_t command, void *payload, __uint16_t payload_size)
{
    if (payload_size % 2 != 0)
    {
        payload_size++;
    }
    __uint32_t rom3_address = (__uint32_t)ROM3_MEMORY_START;
    __uint8_t command_header = *((volatile __uint8_t *)(rom3_address + PROTOCOL_HEADER));
    __uint8_t command_code = *((volatile __uint8_t *)(rom3_address + command));
    __uint8_t command_payload_size = *((volatile __uint8_t *)(rom3_address + payload_size)); // Always even!

#ifdef _DEBUG
    locate(0, 0);
    printf("ROM3: 0x%08X, Size: %d, Header: 0x%02X, Code: 0x%02X, Payload: 0x%02X\r\n", rom3_address, payload_size, command_header, command_code, command_payload_size);
#endif

    if ((payload_size > 0) && (payload != NULL))
    {

        for (__uint16_t i = 0; i < payload_size; i += 2)
        {
            __uint8_t value = *((volatile __uint8_t *)(rom3_address + *((__uint16_t *)(payload + i))));
#ifdef _DEBUG
            locate(0, 1);
            printf("Payload[%i]: 0x%04X / 0x%02X\r\n", i, *((__uint16_t *)(payload + i)), value);
#endif
        }
    }
    return 0;
}

int send_sync_command(__uint16_t command, void *payload, __uint16_t payload_size, __uint32_t timeout, __uint16_t spinner_type)
{
    switch (spinner_type)
    {
    case 1: // Spinning
        printf(" ");
        break;
    case 2: // Countdown
        printf("  ");
        break;
    }
    if (payload_size % 2 != 0)
    {
        payload_size++;
    }
    __uint32_t random_seed = *((volatile __uint32_t *)RANDOM_SEED_ADDRESS);
    __uint32_t rom3_address = (__uint32_t)ROM3_MEMORY_START;
    __uint8_t command_header = *((volatile __uint8_t *)(rom3_address + PROTOCOL_HEADER));
    __uint8_t command_code = *((volatile __uint8_t *)(rom3_address + command));
    __uint8_t command_payload_size = *((volatile __uint8_t *)(rom3_address + payload_size + RANDOM_NUMBER_SIZE)); // Always even!
#ifdef _DEBUG
    locate(0, 0);
    printf("ROM3: 0x%08X, Size: %d, Header: 0x%02X, Code: 0x%02X\r\n", rom3_address, payload_size, command_header, command_code);
    locate(0, 1);
    printf("Cmd payload size: 0x%02X, Seed addr: 0x%08X, New seed: 0x%08X, Timeout: %d\r\n", command_payload_size, RANDOM_SEED_ADDRESS, random_seed, timeout);
#else
    // randomize the random seed
    random_seed = random_seed * *((__uint32_t *)_VBLOCK_ADDRESS);
#endif

    // Send the random seed as part of the payload
    __uint8_t value = *((volatile __uint8_t *)(rom3_address + (__uint16_t)(random_seed & 0xFFFF)));
    value = *((volatile __uint8_t *)(rom3_address + (__uint16_t)((random_seed >> 16) & 0xFFFF)));

    if ((payload_size > 0) && (payload != NULL))
    {
        for (__uint16_t i = 0; i < payload_size; i += 2)
        {
            __uint8_t value = *((volatile __uint8_t *)(rom3_address + *((__uint16_t *)(payload + i))));
#ifdef _DEBUG
            locate(0, 2);
            printf("Payload[%i]: 0x%04X / 0x%02X\r\n", i, *((__uint16_t *)(payload + i)), value);
#endif
        }
    }

    __uint32_t active_wait = timeout * 50; // Assuming PAL system. 50 VBLs per second
    __uint32_t remote_random_numer = 0xFFFFFFFF;
    while (active_wait > 0 && (remote_random_numer != random_seed))
    {
        Vsync();
        switch (spinner_type)
        {
        case 1: // Spinning
            spinner(1);
            break;
        case 2: // Countdown
            printf("\b\b%02d", active_wait / 50);
            break;
        }
        active_wait--;
        remote_random_numer = *((volatile __uint32_t *)RANDOM_NUMBER_ADDRESS);
    }

    // If the active wait is 0, it means that the request was succesful
    // If the active wait is 1, it means that the request timedout
#ifdef _DEBUG
    locate(0, 3);
    printf("Active wait: %d, Remote random number: 0x%08X\r\n", active_wait, remote_random_numer);
#endif
    return (active_wait == 0);
}

void sleep_seconds(__uint16_t seconds, __uint16_t silent)
{
    for (__uint16_t j = 0; j < seconds; j++)
    {
        // Assuming PAL system; for NTSC, use 60.
        for (__uint16_t i = 0; i < 50; i++)
        {
            if (!silent)
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

void please_wait(char *message, __uint16_t seconds)
{
    Cconws(message); // Show the message
    Cconws(" ");     // Leave a space for the spinner
    sleep_seconds(seconds, FALSE);
}

void please_wait_silent(__uint16_t seconds)
{
    sleep_seconds(seconds, TRUE);
}

char *read_files_from_memory(char *memory_location)
{
    char *current_ptr = memory_location;
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

__uint16_t get_file_count(char *file_array)
{
    __uint16_t count = 0;
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

char *get_file_at_index(char *file_array, __uint16_t index)
{
    __uint16_t current_index = 0;
    char *current_ptr = file_array;

    while (*current_ptr)
    { // as long as we don't hit the double null terminator
        if (current_index == index)
        {
            return current_ptr;
        }

        // skip past the current filename to the next
        while (*current_ptr)
            current_ptr++;
        current_ptr++; // skip the null terminator for the current filename
        current_index++;
    }

    return NULL;
}

__uint16_t get_index_of_filename(const char *file_array, char *filename)
{
    __uint16_t current_index = 0;
    char *current_ptr = (char *)file_array;

    while (*current_ptr)
    { // as long as we don't hit the double null terminator
        if (strcmp(current_ptr, filename) == 0)
        {
            return current_index;
        }

        // skip past the current filename to the next
        while (*current_ptr)
            current_ptr++;
        current_ptr++; // skip the null terminator for the current filename
        current_index++;
    }

    return -1;
}

char *print_file_at_index(char *current_ptr, __uint16_t index, int num_columns)
{

    __uint16_t current_index = 0;
    while (*current_ptr)
    { // As long as we don't hit the double null terminator
        if (current_index == index)
        {
            // Print the entire string using printf
            //            printf("\033K%s", current_ptr);
            Cconws("\033K");
            Cconws(current_ptr);

            current_ptr += strlen(current_ptr);

            // printf("\r\n");
            return current_ptr;
        }

        // Skip past the current filename to the next
        while (*current_ptr)
            current_ptr++;
        current_ptr++; // Skip the null terminator for the current filename
        current_index++;
    }

    printf("No file found at index %d\r\n", index);
    return current_ptr;
}

int display_paginated_content(char *file_array, int num_files, int page_size, char *item_name, __uint32_t *keypress)
{
    void highlight_and_print(char *file_array, __uint16_t index, __uint16_t offset, int current_line, int num_columns, __uint16_t highlight)
    {
        locate(0, current_line + 2 + index - offset);
        if (highlight)
            Cconws("\033p"); // Enter reverse video mode (VT52)
        Cconws("\033K");     // Erase to end of line (VT52)
        print_file_at_index(file_array, index, num_columns);
        if (highlight)
            Cconws("\033q"); // Exit reverse video mode (VT52)
    }

    int selected_rom = -1;
    int page_number = 0; // Page number starts at 0
    int current_index = 0;
    locate(0, 22);
    Cconws("Use [UP] and [DOWN] arrows to select. [LEFT] and [RIGHT] to paginate.\r\n");
    Cconws("Press [ENTER] or [RETURN] to confirm. [ESC] to return to main menu.");

    while (selected_rom < 0)
    {
        int start_index = page_number * page_size;
        int end_index = start_index + page_size - 1;
        int max_page = (num_files / page_size);

        if (start_index >= num_files)
        {
            Cconws("No content for this page number!\r\n");
            return -1;
        }

        if (end_index >= num_files)
        {
            end_index = num_files - 1;
        }

        char *current_ptr = file_array;
        int index = 0;
        int current_line = 2 + (ELEMENTS_PER_PAGE - page_size);
        locate(0, current_line);
        Cconws("\033K"); // Erase to end of line (VT52)
        printf("%s found: %d. ", item_name, num_files);
        printf("Page %d of %d\r\n", page_number + 1, max_page + 1);
        locate(0, current_line + 1);
        Cconws("\033K"); // Erase to end of line (VT52)
        while (index <= num_files)
        {
            if ((start_index <= index) && (index <= end_index))
            {
                // Print the index number
                locate(0, current_line + 2 + index - page_size * page_number);
                current_ptr = print_file_at_index(file_array, index, 80);
                index++;
            }
            else
            {
                // Skip past the current filename to the next
                while (*current_ptr != 0x00)
                    current_ptr++;
                current_ptr++; // skip the null terminator for the current filename
                index++;
            }
        }
        for (int i = index; i <= page_size * (page_number + 1); i++)
        {
            // THe -1 at the begining it's because of the previous increment
            locate(0, -1 + current_line + 2 + i - (page_size * page_number));
            Cconws("\033K"); // Erase to end of line (VT52)
        }

        flush_kbd();
        long key;
        __uint16_t change_page = FALSE;
        while ((selected_rom < 0) && (!change_page))
        {
            highlight_and_print(file_array, current_index, page_size * page_number, current_line, 80, TRUE);
            key = Bconin(2);
            if (keypress != NULL)
            {
                *keypress = key;
            }
            switch (key)
            {
            case KEY_UP_ARROW:
                if (current_index > start_index)
                {
                    highlight_and_print(file_array, current_index, page_size * page_number, current_line, 80, FALSE);
                    current_index = current_index - 1;
                }
                break;

            case KEY_DOWN_ARROW:
                if (current_index < end_index)
                {
                    highlight_and_print(file_array, current_index, page_size * page_number, current_line, 80, FALSE);
                    current_index = current_index + 1;
                }
                break;
            case KEY_LEFT_ARROW:
                if (page_number > 0)
                {
                    page_number--;
                    current_index = page_number * page_size;
                    change_page = TRUE;
                }
                break;
            case KEY_RIGHT_ARROW:
                if (page_number < max_page)
                {
                    page_number++;
                    current_index = page_number * page_size;
                    change_page = TRUE;
                }
                break;
            case KEY_ENTER:
            case KEY_RETURN:
                selected_rom = current_index + 1;
                return selected_rom;
            case KEY_ESC:
                return -1;
            default:
                // Handle other keys
                if (keypress != NULL)
                {
                    *keypress = key;
                }
                return -1;
                break;
            }

#ifdef _DEBUG
            locate(0, 20);
            printf("Key pressed: 0x%04X\r\n", key);
            locate(0, 21);
            printf("Current index: %d\r\n", current_index);
#endif
        }
    }
}

void print_centered(const char *str, int screen_width)
{
    int len = strlen(str);
    if (len >= screen_width)
    {
        printf("%s", str);
    }
    else
    {
        int padding = (screen_width - len) / 2; // calculate padding needed
        if (padding > 0)
        {
            printf("%*s", padding, ""); // print padding
        }
        printf("%s", str);
    }
}

// Function to prepend a file name to a buffer containing null-terminated strings.
// The buffer is expected to end with an extra null terminator.
char *prepend_file_to_array(char *file_array, const char *new_file)
{
    size_t new_file_len = strlen(new_file) + 1; // +1 for the null terminator
    size_t existing_array_len = 0;

    if (file_array != NULL)
    {
        // Calculate the length of the existing array
        char *ptr = file_array;
        while (*ptr != '\0' || *(ptr + 1) != '\0')
        { // Double null at the end
            while (*ptr++ != '\0')
                ;
        }
        existing_array_len = ptr - file_array + 1; // Include the double null terminator
    }

    // Allocate a new array that can hold the new file and the existing files
    char *new_array = malloc(new_file_len + existing_array_len);
    if (new_array == NULL)
    {
        printf("Failed to allocate memory for new file array\r\n");
        return NULL;
    }

    // Copy the new file to the new array and add a null terminator
    memcpy(new_array, new_file, new_file_len);

    // Copy the existing files after the new file, including the double null terminator
    if (file_array != NULL)
    {
        memcpy(new_array + new_file_len, file_array, existing_array_len);
    }
    else
    {
        // If there were no existing files, add an extra null terminator
        new_array[new_file_len] = '\0';
    }

    // Free the old array if it was dynamically allocated
    free(file_array);

    return new_array;
}

// Function to read a string from stdin
void read_string(char *string, size_t max_length)
{
    int ch;
    size_t index = 0;

    // Initialize the string buffer
    for (size_t i = 0; i < max_length; i++)
    {
        string[i] = '\0';
    }

    // Repeat until Enter is pressed
    while (1)
    {
        ch = getchar();
        if (ch == '\n' || ch == '\r')
        {
            // User pressed Enter
            break;
        }
        else if (ch == 127 || ch == '\b')
        {
            // User pressed backspace
            if (index > 0)
            {
                index--;
                // Move cursor back one position and overwrite with space
                Cconws(" \b");
                fflush(stdout);
            }
        }
        else if (ch >= ' ' && index < max_length - 1)
        {
            // User entered a printable character
            string[index++] = (char)ch;
        }
    }
    // Ensure the string is null-terminated
    string[index] = '\0';
}
