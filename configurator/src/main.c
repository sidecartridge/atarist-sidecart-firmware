#include <sys/types.h>
#include "include/screen.h"
#include "include/helper.h"
#include "include/romselector.h"
#include "include/config.h"
#include "include/network.h"
#include "include/reset.h"

#define EXIT_OPTION 0
#define ROM_MICROSD_SELECTOR_OPTION 1
#define ROM_NETWORK_SELECTOR_OPTION 2
#define NETWORK_SELECTOR_OPTION 3
#define CONFIGURATION_OPTION 4
#define RESET_OPTION 5
#define LAST_OPTION RESET_OPTION
#define MENU_ALIGN_X 10
#define MENU_ALIGN_Y 4
#define PROMT_ALIGN_X 7
#define PROMT_ALIGN_Y 20
#define MENU_CALLBACK_INTERVAL 10 // Every 10 seconds poll for the connection status

typedef struct
{
    int option;
    int line;
    bool connection; // Connection needed?
    bool networking; // Networking needed?
    const char *description;
} MenuItem;

typedef void (*CallbackFunction)();

// Option index, line, connection needed?, networking needed? description
static const MenuItem menuItems[] = {
    {ROM_MICROSD_SELECTOR_OPTION, ROM_MICROSD_SELECTOR_OPTION, false, false, "Emulate ROM image from microSD card"},
    {ROM_NETWORK_SELECTOR_OPTION, ROM_NETWORK_SELECTOR_OPTION, true, true, "Emulate ROM image from Wi-Fi"},
    {NETWORK_SELECTOR_OPTION, NETWORK_SELECTOR_OPTION, false, true, "Wi-Fi configuration"},
    {CONFIGURATION_OPTION, CONFIGURATION_OPTION, false, false, "SidecarT configuration"},
    {RESET_OPTION, RESET_OPTION, false, false, "Reset to default configuration"},
    {EXIT_OPTION, 7, false, false, "Exit"}};

static __int8_t get_number_active_wait(CallbackFunction callback)
{
    __uint16_t callback_interval = 0;
    bool first_display = true;
    while (1)
    {
        char key = '\0';
        if (Cconis())
        {
            key = Crawcin();
            if (key == '0')
            {
                return EXIT_OPTION;
            }
            __int8_t feature = key - '0';
            if ((feature > 0) && (feature <= LAST_OPTION))
            {
                printf("%c", key);
                return feature;
            }
        }
        if (callback != NULL)
        {
            if (callback_interval <= 0)
            {
                callback(true);
                if ((first_display) || (previous_connection_status != connection_data->status))
                {
                    first_display = false;
                    for (int i = 0; i < sizeof(menuItems) / sizeof(MenuItem); i++)
                    {
                        if (!(menuItems[i].networking) || !(menuItems[i].connection) || (connection_data->status == CONNECTED_WIFI_IP))
                        {
                            locate(MENU_ALIGN_X, MENU_ALIGN_Y + menuItems[i].line);
                            printf("%i. %s", menuItems[i].option, menuItems[i].description);
                        }
                    }
                }
                fflush(stdout);
                callback_interval = MENU_CALLBACK_INTERVAL;
            }
            else
            {
                please_wait_silent(1);
                callback_interval--;
            }
        }
    }
}

static __int8_t menu()
{
    PRINT_APP_HEADER(VERSION);

    // locate(MENU_ALIGN_X, MENU_ALIGN_Y + ROM_MICROSD_SELECTOR_OPTION);
    // printf("%i. Emulate ROM image from microSD card", ROM_MICROSD_SELECTOR_OPTION);
    // locate(MENU_ALIGN_X, MENU_ALIGN_Y + NETWORK_SELECTOR_OPTION);
    // printf("%i. Wi-Fi configuration", NETWORK_SELECTOR_OPTION);
    // locate(MENU_ALIGN_X, MENU_ALIGN_Y + CONFIGURATION_OPTION);
    // printf("%i. SidecarT configuration", CONFIGURATION_OPTION);
    // locate(MENU_ALIGN_X, MENU_ALIGN_Y + RESET_OPTION);
    // printf("%i. Reset to default configuration", RESET_OPTION);
    // locate(MENU_ALIGN_X, MENU_ALIGN_Y + 7);
    // printf("%i. Exit", EXIT_OPTION);

    locate(PROMT_ALIGN_X, PROMT_ALIGN_Y);
    char *prompt;
    asprintf(&prompt, "Choose the feature (1 to %d), or press 0 to exit: ", LAST_OPTION);

    get_connection_status(true);

    __int8_t feature = get_number_active_wait(get_connection_status);

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
        case ROM_MICROSD_SELECTOR_OPTION:
            feature = rom_selector();
            break;
        case ROM_NETWORK_SELECTOR_OPTION:
            feature = roms_from_network_selector();
            break;
        case NETWORK_SELECTOR_OPTION:
            feature = network_selector();
            break;
        case CONFIGURATION_OPTION:
            feature = configuration();
            break;
        case RESET_OPTION:
            feature = reset();
            break;
        default:
            break;
        }
    }
    locate(0, 24);
    printf("\033KNow you can reset or power cycle your Atari ST computer.\r\n");

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
