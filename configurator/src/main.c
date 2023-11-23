#include <sys/types.h>
#include "include/screen.h"
#include "include/helper.h"
#include "include/romselector.h"
#include "include/floppyselector.h"
#include "include/floppydb.h"
#include "include/config.h"
#include "include/network.h"
#include "include/reset.h"
#include "include/storage.h"
#include "include/rtc.h"

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
#define RTC_SELECTOR_OPTION '6'
#define RTC_SELECTOR_OPTION_LINE 6
#define DELAY_TOGGLE_SELECTOR_OPTION 'D'
#define DELAY_TOGGLE_SELECTOR_OPTION_LINE RTC_SELECTOR_OPTION_LINE + 2
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
#define MENU_CALLBACK_INTERVAL 6   // Every 6 seconds poll for the connection status
#define ALLOWED_KEYS "123456DWCRE" // Only these keys are allowed

typedef struct
{
    char option;
    int line;
    const char *description;
} MenuItem;

typedef void (*CallbackFunction)();

// Option index, line, connection needed?, networking needed? description
static MenuItem menuItems[] = {
    {ROM_MICROSD_SELECTOR_OPTION, ROM_MICROSD_SELECTOR_OPTION_LINE, "Emulate ROM image from microSD card"},
    {ROM_NETWORK_SELECTOR_OPTION, ROM_NETWORK_SELECTOR_OPTION_LINE, "Emulate ROM image from Wi-Fi"},
    {FLOPPY_RO_MICROSD_SELECTOR_OPTION, FLOPPY_RO_MICROSD_SELECTOR_OPTION_LINE, "Emulate Floppy image from microSD in Read-Only mode (PREVIEW)"},
    {FLOPPY_RW_MICROSD_SELECTOR_OPTION, FLOPPY_RW_MICROSD_SELECTOR_OPTION_LINE, "Emulate Floppy image from microSD in Read-Write mode (PREVIEW)"},
    {FLOPPY_DB_SELECTOR_OPTION, FLOPPY_DB_SELECTOR_OPTION_LINE, "Download from the Floppy Images database (PREVIEW)"},
    {RTC_SELECTOR_OPTION, RTC_SELECTOR_OPTION_LINE, "Real Time Clock"},
    {DELAY_TOGGLE_SELECTOR_OPTION, DELAY_TOGGLE_SELECTOR_OPTION_LINE, ""},
    {NETWORK_SELECTOR_OPTION, NETWORK_SELECTOR_OPTION_LINE, "Wi-Fi configuration"},
    {CONFIGURATION_OPTION, CONFIGURATION_OPTION_LINE, "SidecarT configuration"},
    {RESET_OPTION, RESET_OPTION_LINE, "Reset to default configuration"},
    {EXIT_OPTION, EXIT_OPTION_LINE, "Exit"}};

// Flag if loading from ROM boot or GEM program
static __uint16_t is_rom_boot = FALSE;

// Modify if more items added before this selector
static __int8_t delay_toogle_selector_index = 6;
static __int8_t exit_option_index = (sizeof(menuItems) / sizeof(menuItems[0])) - 1;

// BLink if there is a new verson available
static void blink_if_new_version_available(__uint16_t blink_toogle)
{
    if (get_latest_release())
    {
        locate(47, 0);
        printf("\033p%s\033q", blink_toogle ? "!" : " ");
    }
}

static __int8_t get_number_active_wait(CallbackFunction networkCallback, CallbackFunction storageCallback)
{
    __uint16_t callback_interval = 0;
    __uint16_t first_time = TRUE;
    flush_kbd();
    while (1)
    {
        char key = '\0';
        if (Cconis() != 0)
        {
            Vsync(); // Wait for VBL interrupt
            key = Cnecin();
            // upper key case
            if ((key >= 'a') && (key <= 'z'))
            {
                key -= 32;
            }
            // only allowed keys
            if ((strchr(ALLOWED_KEYS, key) != NULL) && (key != '\0'))
            {
                return (__int8_t)key;
            }
        }
        else
        {
            if (callback_interval <= 0)
            {
                // The order of the call matters. Check the usage of the memory
                if (storageCallback != NULL)
                {
                    storageCallback(TRUE);
                }
                if (networkCallback != NULL)
                {
                    networkCallback(TRUE);
                }
                callback_interval = MENU_CALLBACK_INTERVAL * 50;
                if (first_time)
                {
                    first_time = FALSE;
                    // Change the DELAY_TOGGLE_SELECTOR_OPTION description according to the value of is_delay_option_enabled()
                    if (is_delay_option_enabled())
                    {
                        menuItems[delay_toogle_selector_index].description = "Disable ROM delay / Ripper mode";
                    }
                    else
                    {
                        menuItems[delay_toogle_selector_index].description = "Enable ROM delay / Ripper mode";
                    }
                    // Change the exit option description according to the value of is_rom_boot
                    if (is_rom_boot)
                    {
                        menuItems[exit_option_index].description = "Reboot";
                    }
                    else
                    {
                        menuItems[exit_option_index].description = "Exit to GEM";
                    }
                    for (int i = 0; i < sizeof(menuItems) / sizeof(MenuItem); i++)
                    {
                        locate(MENU_ALIGN_X, MENU_ALIGN_Y + menuItems[i].line);
                        printf("%c. %s", menuItems[i].option, menuItems[i].description);
                    }
                }
            }
            else
            {
                blink_if_new_version_available((callback_interval / 50) % 2);
                Vsync(); // Wait for VBL interrupt
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
    __int8_t feature = get_number_active_wait((CallbackFunction)get_connection_status, (CallbackFunction)get_storage_status);

    if (feature <= 0)
        feature = -1;

    free(prompt);
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

    init_config();
    init_connection_status();
    init_storage();
    __uint16_t err = read_config();

    __uint16_t feature = err; // If the config is not loaded, exit the program. Otherwise, show the menu
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
        case RTC_SELECTOR_OPTION:
            feature = rtc_menu();
            break;
        case DELAY_TOGGLE_SELECTOR_OPTION:
            feature = toggle_delay_option();
            break;
        case NETWORK_SELECTOR_OPTION:
            feature = wifi_menu();
            break;
        case CONFIGURATION_OPTION:
            feature = configuration();
            break;
        case RESET_OPTION:
            feature = reset();
            break;
        case EXIT_OPTION:
            if (!is_rom_boot)
            {
                restoreResolutionAndPalette(&screenContext);
                return 0;
            }
            break;
        default:
            break;
        }
    }
    locate(0, 22);
    printf("\033K\r\n\033K\r\n\033KPress any key to reset your Atari ST computer...");
    flush_kbd();
#ifdef _DEBUG
    Cnecin();
    restoreResolutionAndPalette(&screenContext);
#else
    char ch;
    while (1)
    {
        ch = Bconin(2);
        if (ch == 27)
        {
            // ESC key ASCII value
            break;
        }
        else
        {
            __asm__(
                "move.l (0x4), %a0\n\t"
                "jmp (%a0)");
        }
    }
#endif
}

//================================================================
// Standard C entry point
int main(int argc, char *argv[])
{
    is_rom_boot = (argc == 1);
    // switching to supervisor mode and execute run()
    // needed because of direct memory access for reading/writing the palette
    Supexec(&run);

    Pterm(0);
}
