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
#include "include/harddisk.h"

#define ROM_MICROSD_SELECTOR_OPTION '1'
#define ROM_MICROSD_SELECTOR_OPTION_LINE 1
#define ROM_NETWORK_SELECTOR_OPTION '2'
#define ROM_NETWORK_SELECTOR_OPTION_LINE 2
#define FLOPPY_MICROSD_SELECTOR_OPTION '3'
#define FLOPPY_MICROSD_SELECTOR_OPTION_LINE 3
#define HARDDISK_MICROSD_SELECTOR_OPTION '4'
#define HARDDISK_MICROSD_SELECTOR_OPTION_LINE 4
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
#define MENU_BLINKING_POSITION 41
#define MENU_CALLBACK_INTERVAL 6     // Default poll for the connection status in seconds
#define MENU_CALLBACK_INTERVAL_MIN 2 // Minimum value for the menu callback interval in seconds
#define ALLOWED_KEYS "123456DWCRE"   // Only these keys are allowed

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
    {FLOPPY_MICROSD_SELECTOR_OPTION, FLOPPY_MICROSD_SELECTOR_OPTION_LINE, "Emulate Floppy Disk Drive"},
    {HARDDISK_MICROSD_SELECTOR_OPTION, HARDDISK_MICROSD_SELECTOR_OPTION_LINE, "Emulate Hard Disk Drive"},
    {FLOPPY_DB_SELECTOR_OPTION, FLOPPY_DB_SELECTOR_OPTION_LINE, "Download from the Floppy Images database"},
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
        locate(MENU_BLINKING_POSITION, 0);
        printf("\033p%s\033q", blink_toogle ? "!" : " ");
    }
}

static __int8_t get_number_active_wait(CallbackFunction networkCallback, CallbackFunction storageCallback, __uint8_t refresh_interval_sec)
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
                callback_interval = refresh_interval_sec * 50;
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
                    size_t i = 0;
                    for (i = 0; i < sizeof(menuItems) / sizeof(MenuItem); i++)
                    {
                        locate(MENU_ALIGN_X, MENU_ALIGN_Y + menuItems[i].line);
                        printf("%c. %s", menuItems[i].option, menuItems[i].description);
                    }
                    printf("\r\n\n\n          Need help? Read the User Guide at https://docs.sidecartridge.com");
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
    ConfigEntry *menu_refresh_sec = get_config_entry(PARAM_MENU_REFRESH_SEC);
    __uint8_t menu_callback_interval = menu_refresh_sec != NULL ? atoi(menu_refresh_sec->value) : MENU_CALLBACK_INTERVAL;
    // If the value is less than the minimum, set the minimum value
    if (menu_callback_interval < MENU_CALLBACK_INTERVAL_MIN)
    {
        menu_callback_interval = MENU_CALLBACK_INTERVAL_MIN;
    }

    PRINT_APP_HEADER(VERSION);

    locate(PROMT_ALIGN_X, PROMT_ALIGN_Y);
    char *prompt;
    asprintf(&prompt, "Choose the feature (1 to %d), or press 0 to exit: ", LAST_OPTION);
    __int8_t feature = get_number_active_wait((CallbackFunction)get_connection_status, (CallbackFunction)get_storage_status, menu_callback_interval);

    if (feature <= 0)
        feature = -1;

    free(prompt);
    return feature;
}

/* 'get_cookie':  Inquires the value of a cookie.
                  The parameters passed for this are the ID of the
                  cookie to be found, as well as a pointer to the
                  found value.

                  The routine returns the value FALSE if the
                  specified cookie does not exist... */

typedef struct
{
    __uint32_t id;    /* Identification code */
    __uint32_t value; /* Value of the cookie */
} COOKJAR;

static __uint16_t get_cookie(__uint32_t cookie, __uint32_t *value)
{
    COOKJAR *cookiejar;

    cookiejar = *(COOKJAR **)0x05A0;
    if (cookiejar != NULL)
    {
        for (__uint16_t i = 0; cookiejar[i].id; i++)
            if (cookiejar[i].id == cookie)
            {
                if (value)
                    *(__uint32_t *)value = cookiejar[i].value;
                return TRUE;
            }
    }
    return FALSE;
}

//================================================================
// Main program
static int run()
{

    __uint8_t previous_cpu_16mhz_status = 0;
    __uint8_t cpu_16mhz_status = 0;
    __uint32_t machine_type = 0;

    void restore_cpu_speed()
    {
        if (previous_cpu_16mhz_status != 0)
        {
            // Restore the previous CPU configuration
            // CPU speed first
            *(volatile __uint8_t *)0xffff8e21 = previous_cpu_16mhz_status & 0x1;
            // Finally the cache
            *(volatile __uint8_t *)0xffff8e21 = previous_cpu_16mhz_status;
        }
    }

    void change_cpu_speed()
    {

        if (get_cookie(0x5f4d4348, &machine_type)) // '_MCH' is 0x5f4d4348
        {
            switch (machine_type)
            {
            case 0x00010010: // Atari MegaSTE
                cpu_16mhz_status = *(__uint8_t *)0xffff8e21;

#ifdef _DEBUG
                if ((cpu_16mhz_status & 0x1) > 0)
                {
                    printf("Cache enabled.\n\r");
                }
                else
                {
                    printf("Cache disabled.\n\r");
                }
                if ((cpu_16mhz_status & 0x8) > 0)
                {
                    printf("16MHz CPU enabled.\n\r");
                }
                else
                {
                    printf("8MHz CPU enabled.\n\r");
                }
#endif
                previous_cpu_16mhz_status = cpu_16mhz_status;
                // First disable the cache
                *(volatile __uint8_t *)0xffff8e21 = cpu_16mhz_status & 0x1;
                // Then enable the 8MHz CPU
                *(volatile __uint8_t *)0xffff8e21 = 0x0;
                break;
            case 0x00010000: // Atari STE
                break;
            case 0x0002000: // Atari TT
                break;
            case 0x0003000: // Atari Falcon
                break;
            default:
#ifdef _DEBUG
                printf("Atari ST detected with cookie-jar?.\n\r");
#endif
                break;
            }
        }
#ifdef _DEBUG
        else
        {
            printf("Atari ST detected.\n\r");
        }
#endif
    }

    change_cpu_speed();

    send_async_command(CLEAN_START, NULL, 0);
    printf("\r\033KReading the microSD. Be patient...");

    init_kb();
    init_config();
    init_connection_status();
    init_storage();
    __uint16_t err = read_config();

    ScreenContext screenContext;
    initScreenContext(&screenContext);

    __uint8_t is_dark_mode = 0;
    ConfigEntry *dark_mode = get_config_entry(PARAM_CONFIGURATOR_DARK);
    is_dark_mode = (dark_mode != NULL && (dark_mode->value[0] == 't' || dark_mode->value[0] == 'T'));

    if (screenContext.savedResolution == HIGH_RES)
    {
        initHighResolution();
    }
    else
    {
        __uint16_t palette[4] = {0xFFF, 0x000, 0x000, 0x000};
        if (is_dark_mode)
        {
            palette[0] = 0x000;
            palette[1] = 0xFFF;
            palette[2] = 0xFFF;
            palette[3] = 0xFFF;
        }
        initMedResolution(palette);
    }

    __uint16_t feature = err; // If the config is not loaded, exit the program. Otherwise, show the menu
    while (feature == 0)
    {
        send_async_command(CLEAN_START, NULL, 0);
        feature = menu();
        switch (feature)
        {
        case ROM_MICROSD_SELECTOR_OPTION:
            feature = rom_selector();
            break;
        case ROM_NETWORK_SELECTOR_OPTION:
            feature = roms_from_network_selector();
            break;
        case FLOPPY_MICROSD_SELECTOR_OPTION:
            feature = floppy_menu();
            break;
        case HARDDISK_MICROSD_SELECTOR_OPTION:
            feature = harddisk_menu();
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
                restore_cpu_speed();
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

    restore_cpu_speed();

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
                "clr.l 0x00000420\n\t" // Invalidate memory system variables
                "clr.l 0x0000043A\n\t"
                "clr.l 0x0000051A\n\t"
                "move.l (0x4), %a0\n\t" // Now we can safely jump to the reset vector
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

    // Restore the CPU speed
    //    *((unsigned int *)0x00ff8e20L) = cpu_16mhz_status;

    Pterm(0);
}
