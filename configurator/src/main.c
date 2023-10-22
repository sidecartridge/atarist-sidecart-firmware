#include <sys/types.h>
#include "include/screen.h"
#include "include/helper.h"
#include "include/romselector.h"
#include "include/floppyselector.h"
#include "include/floppydb.h"
#include "include/config.h"
#include "include/network.h"
#include "include/reset.h"

#define ROM_MICROSD_SELECTOR_OPTION '1'
#define ROM_MICROSD_SELECTOR_OPTION_LINE 1
#define ROM_NETWORK_SELECTOR_OPTION '2'
#define ROM_NETWORK_SELECTOR_OPTION_LINE 2
#define FLOPPY_RO_MICROSD_SELECTOR_OPTION '3'
#define FLOPPY_RO_MICROSD_SELECTOR_OPTION_LINE 3
#define FLOPPY_RW_MICROSD_SELECTOR_OPTION '4'
#define FLOPPY_RW_MICROSD_SELECTOR_OPTION_LINE 4
#define FLOPPY_DB_SELECTOR_OPTION '5'
#define FLOPPY_DB_SELECTOR_OPTION_LINE 5
#define DELAY_TOGGLE_SELECTOR_OPTION 'D'
#define DELAY_TOGGLE_SELECTOR_OPTION_LINE FLOPPY_DB_SELECTOR_OPTION_LINE + 2
#define NETWORK_SELECTOR_OPTION 'W'
#define NETWORK_SELECTOR_OPTION_LINE DELAY_TOGGLE_SELECTOR_OPTION_LINE + 1
#define CONFIGURATION_OPTION 'C'
#define CONFIGURATION_OPTION_LINE NETWORK_SELECTOR_OPTION_LINE + 1
#define RESET_OPTION 'R'
#define RESET_OPTION_LINE CONFIGURATION_OPTION_LINE + 1
#define EXIT_OPTION 'E'
#define EXIT_OPTION_LINE RESET_OPTION_LINE + 2
#define LAST_OPTION RESET_OPTION_LINE

#define MENU_ALIGN_X 10
#define MENU_ALIGN_Y 4
#define PROMT_ALIGN_X 7
#define PROMT_ALIGN_Y 20
#define MENU_CALLBACK_INTERVAL 10 // Every 10 seconds poll for the connection status
#define ALLOWED_KEYS "12345DWCRE" // Only these keys are allowed

typedef struct
{
    char option;
    int line;
    bool connection; // Connection needed?
    bool networking; // Networking needed?
    const char *description;
} MenuItem;

typedef void (*CallbackFunction)();

// Option index, line, connection needed?, networking needed? description
static MenuItem menuItems[] = {
    {ROM_MICROSD_SELECTOR_OPTION, ROM_MICROSD_SELECTOR_OPTION_LINE, false, false, "Emulate ROM image from microSD card"},
    {ROM_NETWORK_SELECTOR_OPTION, ROM_NETWORK_SELECTOR_OPTION_LINE, true, true, "Emulate ROM image from Wi-Fi"},
    {FLOPPY_RO_MICROSD_SELECTOR_OPTION, FLOPPY_RO_MICROSD_SELECTOR_OPTION_LINE, false, false, "Emulate Floppy image from microSD in Read-Only mode (PREVIEW)"},
    {FLOPPY_RW_MICROSD_SELECTOR_OPTION, FLOPPY_RW_MICROSD_SELECTOR_OPTION_LINE, false, false, "Emulate Floppy image from microSD in Read-Write mode (PREVIEW)"},
    {FLOPPY_DB_SELECTOR_OPTION, FLOPPY_DB_SELECTOR_OPTION_LINE, true, true, "Download from the Floppy Images database (PREVIEW)"},
    {DELAY_TOGGLE_SELECTOR_OPTION, DELAY_TOGGLE_SELECTOR_OPTION_LINE, false, false, ""},
    {NETWORK_SELECTOR_OPTION, NETWORK_SELECTOR_OPTION_LINE, false, true, "Wi-Fi configuration"},
    {CONFIGURATION_OPTION, CONFIGURATION_OPTION_LINE, false, false, "SidecarT configuration"},
    {RESET_OPTION, RESET_OPTION_LINE, false, false, "Reset to default configuration"},
    {EXIT_OPTION, EXIT_OPTION_LINE, false, false, "Exit"}};

// Modify if more items added before this selector
static __int8_t delay_toogle_selector_index = 5;

static __int8_t get_number_active_wait(CallbackFunction callback)
{
    __uint16_t callback_interval = 0;
    while (1)
    {
        char key = '\0';
        if (Cconis())
        {
            key = Crawcin();
            // upper key case
            if ((key >= 'a') && (key <= 'z'))
            {
                key -= 32;
            }
            // only allowed keys
            if (strchr(ALLOWED_KEYS, key) != NULL)
            {
                printf("%c", key);
                return (__int8_t)key;
            }
        }
        if (callback != NULL)
        {
            if (callback_interval <= 0)
            {
                callback(true);
                // Change the DELAY_TOGGLE_SELECTOR_OPTION description according to the value of is_delay_option_enabled()
                if (is_delay_option_enabled())
                {
                    menuItems[delay_toogle_selector_index].description = "Disable ROM delay / Ripper mode";
                }
                else
                {
                    menuItems[delay_toogle_selector_index].description = "Enable ROM delay / Ripper mode";
                }
                for (int i = 0; i < sizeof(menuItems) / sizeof(MenuItem); i++)
                {
                    if ((!(menuItems[i].networking) || (connection_data->status != NOT_SUPPORTED)) && (!(menuItems[i].connection) || (connection_data->status == CONNECTED_WIFI_IP)))
                    {
                        locate(MENU_ALIGN_X, MENU_ALIGN_Y + menuItems[i].line);
                        printf("%c. %s", menuItems[i].option, menuItems[i].description);
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

    locate(PROMT_ALIGN_X, PROMT_ALIGN_Y);
    char *prompt;
    asprintf(&prompt, "Choose the feature (1 to %d), or press 0 to exit: ", LAST_OPTION);
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

    read_config();

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
        case FLOPPY_RO_MICROSD_SELECTOR_OPTION:
            feature = floppy_selector_ro();
            break;
        case FLOPPY_RW_MICROSD_SELECTOR_OPTION:
            feature = floppy_selector_rw();
            break;
        case FLOPPY_DB_SELECTOR_OPTION:
            feature = floppy_db();
            break;
        case DELAY_TOGGLE_SELECTOR_OPTION:
            feature = toggle_delay_option();
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
        case EXIT_OPTION:
            restoreResolutionAndPalette(&screenContext);
            return 0;
            break;
        default:
            break;
        }
    }
    locate(0, 24);
    printf("\033KPress any key to reset your Atari ST computer.\r\n");

#ifdef _DEBUG
    getchar();
    restoreResolutionAndPalette(&screenContext);
#else
    char ch;
    while (1)
    {
        ch = Crawcin();
        if (ch == 27)
        {
            // ESC key ASCII value
            break;
        }
        else
        {
            typedef void (*tFunction)(void);

            static const kWarmStartAddress_U32 = 0xE00000ul;
            tFunction JumpToTOS;
            __uint32_t myJumpAddress_U32;

            myJumpAddress_U32 = (*(volatile __uint32_t *)(kWarmStartAddress_U32 + sizeof(__uint32_t))); /* slot #0 is occupied by SSP	*/
            JumpToTOS = (tFunction)myJumpAddress_U32;
            JumpToTOS();
        }
    }
#endif
}

//================================================================
// Standard C entry point
int main(int argc, char *argv[])
{
    // switching to supervisor mode and execute run()
    // needed because of direct memory access for reading/writing the palette
    Supexec(&run);

    Pterm(0);
}
