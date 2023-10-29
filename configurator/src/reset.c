#include "include/reset.h"

__uint16_t reset()
{
    char userResponse;

    PRINT_APP_HEADER(VERSION);
    printf("\r\n");
    printf("WARNING: Resetting SidecarT will delete any ROM or image in FLASH and will\r\n");
    printf("return to the default configuration.\r\n\r\n");
    printf("Do you still want to continue? [Y/n]: ");
    userResponse = getchar(); // Get user input

    // Check if the input is 'Y' or 'y'
    if (userResponse == 'Y' || userResponse == 'y')
    {
        send_sync_command(RESET_DEVICE, NULL, 0, 10, FALSE);
        please_wait("\r\033KResetting SidecarT...", 10);
        printf("\r\033KSidecarT has been reset.\r\n");
    }
    else
    {
        printf("Reset operation cancelled.\r\n");
    }
    return 0; // Return to menu
}
