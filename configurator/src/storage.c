#include "include/storage.h"

// SdCardData *sd_card_data = (SdCardData *)SD_CARD_DATA_START_ADDRESS + sizeof(__uint32_t);
SdCardData *sd_card_data = NULL;

__uint16_t get_storage_status(bool show_bar)
{
    char buffer[STATUS_STRING_BUFFER_SIZE];
    int err = send_sync_command(GET_SD_DATA, NULL, (__uint16_t)0, 5, false);
    if (err != 0)
    {
        snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "Cannot read microSD status. Is the SidecarT connected?");
    }
    else
    {
        if (sd_card_data == NULL)
        {
            // We don't know the size of the structure, but 1K should be more than enough
            sd_card_data = malloc(sizeof(SdCardData));
        }
        memcpy(sd_card_data, (SdCardData *)(SD_CARD_DATA_START_ADDRESS + sizeof(__uint32_t)), sizeof(SdCardData));

        if (show_bar)
        {
            if (sd_card_data->status == SD_CARD_MOUNTED)
            {
                // Make sure you calculate the space used correctly to avoid underflow
                int space = max(0, sd_card_data->sd_size - sd_card_data->sd_free_space);

                snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "%i of %iMB used | %i ROMs in %s | %i Floppies in %s",
                         space,
                         sd_card_data->sd_size,
                         sd_card_data->roms_folder_status == ROMS_FOLDER_OK ? sd_card_data->roms_folder_count : 0,
                         sd_card_data->roms_folder_status == ROMS_FOLDER_OK ? sd_card_data->roms_folder : "Not Found",
                         sd_card_data->floppies_folder_status == FLOPPIES_FOLDER_OK ? sd_card_data->floppies_folder_count : 0,
                         sd_card_data->floppies_folder_status == FLOPPIES_FOLDER_OK ? sd_card_data->floppies_folder : "Not Found");
            }
            else
            {
                snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "No microSD card found. Insert one with the folders '%s' and '%s'",
                         sd_card_data->roms_folder,
                         sd_card_data->floppies_folder);
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

static __uint8_t check_folder(__uint16_t service)
{
    __uint8_t status = 0; // 0 is ok
    if ((sd_card_data == NULL) || (sd_card_data->status != SD_CARD_MOUNTED))
    {
        printf("No microSD card found. Insert one with the folder ");
        switch (service)
        {
        case ROMS:
            printf("'%s'", sd_card_data->roms_folder);
            break;
        case FLOPPIES:
        case FLOPPIES_DB:
            printf("'%s'", sd_card_data->floppies_folder);
            break;
        case HARDDISKS:
            printf("'%s'", sd_card_data->harddisks_folder);
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
            printf("The folder '%s' was not found in the microSD card.\r\nCreate one and put your ROMS inside.\r\n", sd_card_data->roms_folder);
            status = 2;
        }
    }
    if ((service == FLOPPIES) && (status == 0))
    {
        if (sd_card_data->floppies_folder_status != FLOPPIES_FOLDER_OK)
        {
            printf("The folder '%s' was not found in the microSD card.\r\nCreate one and put your floppy images inside.\r\n", sd_card_data->floppies_folder);
            status = 3;
        }
    }
    if ((service == FLOPPIES_DB) && (status == 0))
    {
        if (sd_card_data->floppies_folder_status != FLOPPIES_FOLDER_OK)
        {
            printf("The folder '%s' was not found in the microSD card.\r\nPlease create one, SidecarT will download the floppy images inside.\r\n", sd_card_data->floppies_folder);
            status = 4;
        }
    }
    if ((service == HARDDISKS) && (status == 0))
    {
        if (sd_card_data->harddisks_folder_status != HARDDISKS_FOLDER_OK)
        {
            printf("The folder '%s' was not found in the microSD card.\r\nCreate one and put your hard disk images inside.\r\n", sd_card_data->harddisks_folder);
            status = 5;
        }
    }
    if (status != 0)
    {
        printf("Press any key to exit...\r\n");
        getchar();
    }
    return status;
}

__uint8_t check_folder_roms()
{
    return check_folder(ROMS);
}

__uint8_t check_folder_floppies()
{
    return check_folder(FLOPPIES);
}

__uint8_t check_folder_floppies_db()
{
    return check_folder(FLOPPIES_DB);
}

__uint8_t check_folder_harddisks()
{
    return check_folder(HARDDISKS);
}
