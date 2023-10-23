#include "include/storage.h"

SdCardData *sd_card_data = SD_CARD_DATA_START_ADDRESS + sizeof(__uint32_t);

__uint16_t get_storage_status(bool show_bar)
{
    send_sync_command(GET_SD_DATA, NULL, (__uint16_t)0, 5, false);

    locate(0, 23);
    if (show_bar)
    {
        char buffer[128];
        if (sd_card_data->status == SD_CARD_MOUNTED)
        {
            sprintf(buffer, "%i of %iMB used | %i ROMs in %s | %i Floppies in %s",
                    sd_card_data->sd_size - sd_card_data->sd_free_space,
                    sd_card_data->sd_size,
                    sd_card_data->roms_folder_status == ROMS_FOLDER_OK ? sd_card_data->roms_folder_count : 0,
                    sd_card_data->roms_folder_status == ROMS_FOLDER_OK ? sd_card_data->roms_folder : "Not Found",
                    sd_card_data->floppies_folder_status == FLOPPIES_FOLDER_OK ? sd_card_data->floppies_folder_count : 0,
                    sd_card_data->floppies_folder_status == FLOPPIES_FOLDER_OK ? sd_card_data->floppies_folder : "Not Found");
        }
        else
        {
            sprintf(buffer, "No microSD card found. Insert one with the folders '%s' and '%s'",
                    sd_card_data->roms_folder,
                    sd_card_data->floppies_folder);
        }

        if (strlen(buffer) % 2 == 1)
        {
            printf(" ");
        }
        printf("\033p");
        for (int i = 0; i < (80 - strlen(buffer)) / 2; i++)
        {
            printf(" ");
        }
        printf(buffer);
        for (int i = 0; i < (80 - strlen(buffer)) / 2; i++)
        {
            printf(" ");
        }
        printf("\033q");
    }
    return sd_card_data->status;
}
