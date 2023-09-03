
#include <sys/types.h>
#include "screen.h"
#include "helper.h"
#include "romselector.h"
#include "config.h"

static __int8_t menu()
{
    PRINT_APP_HEADER(VERSION);

    int num_options = 0;
    locate(10, 5);
    printf("%i. Emulate ROM image from microSD card", ++num_options);
    locate(10, 6);
    // printf("%i. Emulate Floppy image from microSD card in drive A", num_options++);
    // locate(10, 7);
    // printf("%i. Emulate Floppy image from microSD card in drive B", num_options++);
    // locate(10, 8);
    printf("%i. SidecarT configuration", ++num_options);
    locate(10, 7);
    printf("%i. Reset to default configuration", ++num_options);
    // locate(10, 9);
    // printf("%i. Wi-Fi configuration", num_options);
    locate(10, 11);
    printf("0. Exit");

    locate(7, 20);
    char *prompt;
    asprintf(&prompt, "Choose the feature (1 to %d), or press 0 to exit: ", num_options);

    __int8_t feature = (__int8_t)get_number_within_range(prompt, num_options, 0, '\0', '\0');

    if (feature <= 0)
        feature = -1;

    return feature;
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

    __uint8_t feature = 0; // Feature 0 is menu
    while (feature == 0)
    {
        feature = menu();

        switch (feature)
        {
        case 1:
            feature = rom_selector();
            break;
        case 2:
            feature = configuration();
            break;
        case 3:
            feature = reset();
            break;
        case 4:
            rom_selector();
            break;
        case 5:
            rom_selector();
            break;
        default:
            break;
        }
    }

    printf("Now you can reset or power cycle your Atari ST computer.\r\n");

#ifdef _DEBUG
    getchar();
#else
    char ch;
    while (1)
    {
        ch = Crawcin();
        if (ch == 27) // ESC key ASCII value
            break;
    }
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
