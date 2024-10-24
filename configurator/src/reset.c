#include "include/reset.h"

__uint16_t reset()
{
    char userResponse;

    PRINT_APP_HEADER(VERSION);
    printf("\r\n");
    printf("WARNING: Resetting device will delete any ROM or image in FLASH and will\r\n");
    printf("return to the default configuration and restarting the application.\r\n\r\n");
    printf("Do you still want to continue? [Y/n]: ");

    flush_kbd();

    userResponse = getchar(); // Get user input

    // Check if the input is 'Y' or 'y'
    if (userResponse == 'Y' || userResponse == 'y')
    {
        send_sync_command(RESET_DEVICE, NULL, 0, NETWORK_WAIT_TIME, FALSE);
        please_wait("\r\033KResetting device...", 10);
        printf("\r\033KDevice has been reset.\r\n");
    }
    else
    {
        printf("Reset operation cancelled.\r\n");
        return 0; // Return to menu
    }
    return 1; // Exit program
}
