#include "reset.h"

__uint8_t reset()
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
        send_command(RESET_DEVICE, NULL, 0);
        printf("SidecarT has been reset.\r\n");
        return 1; // Indicate that reset was successful (or some other status code you wish)
    }
    else
    {
        printf("Reset operation cancelled.\r\n");
        return 0; // Indicate that reset was cancelled. Return to menu
    }
}
