#include "include/storage.h"

static SdCardData *sd_card_data = NULL;

void init_storage()
{
    sd_card_data = malloc(sizeof(SdCardData));
    memset(sd_card_data, 0, sizeof(SdCardData));
}

__uint16_t get_storage_status(__uint16_t show_bar)
{
    char buffer[STATUS_STRING_BUFFER_SIZE];
    int err = send_sync_command(GET_SD_DATA, NULL, 0, STORAGE_WAIT_TIME, FALSE);
    if (err != 0)
    {
        // snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "Cannot read microSD status. Retrying...");
        return err;
    }
    else
    {

        if (sd_card_data == NULL)
        {
            // We don't know the size of the structure, but 1K should be more than enough
            sd_card_data = malloc(sizeof(SdCardData));
        }
        memcpy(sd_card_data, (SdCardData *)(SD_CARD_DATA_START_ADDRESS + sizeof(__uint32_t)), 512);

        if (show_bar)
        {
            if (sd_card_data->status == SD_CARD_MOUNTED)
            {
                // Make sure you calculate the space used correctly to avoid underflow
                int space = max(0, sd_card_data->sd_size - sd_card_data->sd_free_space);

                // Temporary variables for each part of the formatted string
                int usedSpace = space;
                int totalSpace = sd_card_data->sd_size;
                int romsCount = sd_card_data->roms_folder_status == ROMS_FOLDER_OK ? sd_card_data->roms_folder_count : 0;
                const char *romsFolder = sd_card_data->roms_folder_status == ROMS_FOLDER_OK ? sd_card_data->roms_folder : "Not Found";
                int floppiesCount = sd_card_data->floppies_folder_status == FLOPPIES_FOLDER_OK ? sd_card_data->floppies_folder_count : 0;
                const char *floppiesFolder = sd_card_data->floppies_folder_status == FLOPPIES_FOLDER_OK ? sd_card_data->floppies_folder : "Not Found";
                int harddisksCount = sd_card_data->harddisks_folder_status == HARDDISKS_FOLDER_OK ? sd_card_data->harddisks_folder_count : 0;
                const char *harddisksFolder = sd_card_data->harddisks_folder_status == HARDDISKS_FOLDER_OK ? sd_card_data->harddisks_folder : "Not Found";

                char space_str[128];
                char roms_str[128];
                char floppies_str[128];
                char harddisks_str[128];

                // Create strings for each part
                snprintf(space_str, sizeof(space_str), "%i of %iMB used | ", usedSpace, totalSpace);
                if (sd_card_data->roms_folder_status == ROMS_FOLDER_OK)
                {
                    if (romsCount == 0)
                    {
                        snprintf(roms_str, sizeof(roms_str), "'%s' found | ", romsFolder);
                    }
                    else
                    {
                        snprintf(roms_str, sizeof(roms_str), "%i ROM images | ", romsCount);
                    }
                }
                else
                {
                    snprintf(roms_str, sizeof(roms_str), "No ROMs folder! | ");
                }
                if (sd_card_data->floppies_folder_status == FLOPPIES_FOLDER_OK)
                {
                    if (floppiesCount == 0)
                    {
                        snprintf(floppies_str, sizeof(floppies_str), "'%s' found | ", floppiesFolder);
                    }
                    else
                    {
                        snprintf(floppies_str, sizeof(floppies_str), "%i Disk images | ", floppiesCount);
                    }
                }
                else
                {
                    snprintf(floppies_str, sizeof(floppies_str), "No Floppies folder! | ");
                }
                if (sd_card_data->harddisks_folder_status == HARDDISKS_FOLDER_OK)
                {
                    if (harddisksCount == 0)
                    {
                        snprintf(harddisks_str, sizeof(harddisks_str), "'%s' found", harddisksFolder);
                    }
                    else
                    {
                        snprintf(harddisks_str, sizeof(harddisks_str), "%i Files in HD", harddisksCount);
                    }
                }
                else
                {
                    snprintf(harddisks_str, sizeof(harddisks_str), "No HD folder!");
                }

                // Final snprintf call concatenating all parts
                snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "%s%s%s%s",
                         space_str,
                         roms_str,
                         floppies_str,
                         harddisks_str);
            }
            else
            {
                ConfigEntry *roms_folder = get_config_entry(PARAM_ROMS_FOLDER);
                ConfigEntry *floppies_folder = get_config_entry(PARAM_FLOPPIES_FOLDER);
                ConfigEntry *harddisk_folder = get_config_entry(PARAM_GEMDRIVE_FOLDERS);
                char *roms_folder_value = roms_folder != NULL ? roms_folder->value : "/roms";
                char *floppies_folder_value = floppies_folder != NULL ? floppies_folder->value : "/floppies";
                char *harddisk_folder_value = harddisk_folder != NULL ? harddisk_folder->value : "/hd";
                snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "No microSD card found. Insert one with folders '%s', '%s' and '%s'",
                         roms_folder_value,
                         floppies_folder_value,
                         harddisk_folder_value);
            }
        }
    }
    locate(0, 23);
    printf("\033p");
    int bufferLength = strlen(buffer);
    int totalPadding = STATUS_STRING_BUFFER_SIZE - bufferLength;
    int leftPadding = totalPadding / 2;
    int rightPadding = totalPadding - leftPadding;

    printf("%*s%s%*s", leftPadding, "", buffer, rightPadding, "");
    printf("\033q");
    return err;
}

static __uint16_t check_folder(__uint16_t service)
{
    __uint16_t status = 0; // 0 is ok
    ConfigEntry *roms_folder = get_config_entry(PARAM_ROMS_FOLDER);
    ConfigEntry *floppies_folder = get_config_entry(PARAM_FLOPPIES_FOLDER);
    ConfigEntry *harddisks_folder = get_config_entry(PARAM_GEMDRIVE_FOLDERS);
    char *roms_folder_value = roms_folder != NULL ? roms_folder->value : "/roms";
    char *floppies_folder_value = floppies_folder != NULL ? floppies_folder->value : "/floppies";
    char *harddisks_folder_value = harddisks_folder != NULL ? harddisks_folder->value : "/hd";
    if ((sd_card_data == NULL) || (sd_card_data->status != SD_CARD_MOUNTED))
    {
        printf("No microSD card found. Insert one with the folder ");
        switch (service)
        {
        case ROMS:
            printf("'%s'", roms_folder_value);
            break;
        case FLOPPIES:
        case FLOPPIES_DB:
            printf("'%s'", floppies_folder_value);
            break;
        case HARDDISKS:
            printf("'%s'", harddisks_folder_value);
            break;
        default:
            break;
        }
        printf("\r\n");
        status = 1;
    }
    if ((service == ROMS) && (status == 0))
    {
        if (sd_card_data->roms_folder_status != ROMS_FOLDER_OK)
        {
            printf("The folder '%s' was not found in the microSD card.\r\nCreate one and put your ROMS inside.\r\n", roms_folder_value);
            status = 2;
        }
    }
    if ((service == FLOPPIES) && (status == 0))
    {
        if (sd_card_data->floppies_folder_status != FLOPPIES_FOLDER_OK)
        {
            printf("The folder '%s' was not found in the microSD card.\r\nCreate one and put your floppy images inside.\r\n", floppies_folder_value);
            status = 3;
        }
    }
    if ((service == FLOPPIES_DB) && (status == 0))
    {
        if (sd_card_data->floppies_folder_status != FLOPPIES_FOLDER_OK)
        {
            printf("The folder '%s' was not found in the microSD card.\r\nPlease create one, device will download the floppy images inside.\r\n", floppies_folder_value);
            status = 4;
        }
    }
    if ((service == HARDDISKS) && (status == 0))
    {
        if (sd_card_data->harddisks_folder_status != HARDDISKS_FOLDER_OK)
        {
            printf("The folder '%s' was not found in the microSD card.\r\nCreate one and put your hard disk images inside.\r\n", harddisks_folder_value);
            status = 5;
        }
    }
    if (status != 0)
    {
        press_key("Press any key to exit...\r\n");
    }
    return status;
}

__uint16_t check_folder_roms()
{
    return check_folder(ROMS);
}

__uint16_t check_folder_floppies()
{
    return check_folder(FLOPPIES);
}

__uint16_t check_folder_floppies_db()
{
    return check_folder(FLOPPIES_DB);
}

__uint16_t check_folder_harddisks()
{
    return check_folder(HARDDISKS);
}
