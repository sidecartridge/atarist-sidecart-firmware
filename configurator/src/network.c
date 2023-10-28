#include "include/network.h"

ConnectionStatus connection_status = DISCONNECTED;
WifiScanData *wifiScanData = NULL;
// ConnectionData *connection_data = (ConnectionData *)(CONNECTION_STATUS_START_ADDRESS + sizeof(__uint32_t));
ConnectionData *connection_data = NULL;
bool poll_latest_release = true;
bool latest_release_available = false;

static void read_networks_from_memory(char *ssids, WifiNetworkInfo networks[], __uint16_t total_size)
{
    char *current_ssid_position = ssids;

    for (__uint16_t i = 0; i < total_size; i++)
    {
        size_t ssid_len = strlen(networks[i].ssid);
        memcpy(current_ssid_position, networks[i].ssid, ssid_len);
        current_ssid_position[ssid_len] = '\0'; // Null terminate the copied string
        current_ssid_position += ssid_len + 1;  // Move to the next position, taking into account the null terminator
    }
    current_ssid_position[0] = '\0'; // Null terminate the list of ssids
}

static __uint8_t get_network_count(char *file_array)
{
    __uint8_t count = 0;
    char *current_ptr = file_array;

    while (*current_ptr)
    { // as long as we don't hit the double null terminator
        count++;

        // skip past the current filename to the next
        while (*current_ptr)
            current_ptr++;
        current_ptr++; // skip the null terminator for the current filename
    }

    return count;
}

char *get_status_str(ConnectionStatus status)
{
    char *status_str = "Disconnected";
    switch (connection_data->status)
    {
    case DISCONNECTED:
        status_str = "Disconnected";
        break;
    case CONNECTING:
        status_str = "Connecting...";
        break;
    case CONNECTED_WIFI:
        status_str = "Wi-Fi only, no IP";
        break;
    case CONNECTED_WIFI_NO_IP:
        status_str = "Wi-Fi only, no IP";
        break;
    case CONNECTED_WIFI_IP:
        status_str = "Connected";
        break;
    case TIMEOUT_ERROR:
        status_str = "Timeout!";
        break;
    case GENERIC_ERROR:
        status_str = "Error!";
        break;
    case NO_DATA_ERROR:
        status_str = "No data!";
        break;
    case NOT_PERMITTED_ERROR:
        status_str = "Not permitted!";
        break;
    case INVALID_ARG_ERROR:
        status_str = "Invalid args!";
        break;
    case IO_ERROR:
        status_str = "IO error!";
        break;
    case BADAUTH_ERROR:
        status_str = "Bad auth!";
        break;
    case CONNECT_FAILED_ERROR:
        status_str = "Connect failed!";
        break;
    case INSUFFICIENT_RESOURCES_ERROR:
        status_str = "No resources!";
        break;
    case NOT_SUPPORTED:
        status_str = "Networking not supported!";
        break;
    }
    return status_str;
}

__uint16_t get_connection_status(bool show_bar)
{
    char buffer[STATUS_STRING_BUFFER_SIZE];
    int err = send_sync_command(GET_IP_DATA, NULL, (__uint16_t)0, 5, false);

    if (err != 0)
    {
        snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "Cannot read network status. Is the SidecarT connected?");
    }
    else
    {
        if (connection_data == NULL)
        {
            connection_data = malloc(sizeof(ConnectionData));
        }
        memcpy(connection_data, (ConnectionData *)(CONNECTION_STATUS_START_ADDRESS + sizeof(__uint32_t)), sizeof(ConnectionData));
        char *status_str = get_status_str(connection_data->status);
        snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "IP: %s | SSID: %s | Status: %s",
                 connection_data->ipv4_address, connection_data->ssid,
                 status_str);
    }

    if (show_bar)
    {
        locate(0, 24);
        printf("\033p");
        int bufferLength = strlen(buffer);
        int totalPadding = STATUS_STRING_BUFFER_SIZE - bufferLength;
        int leftPadding = totalPadding / 2;
        int rightPadding = totalPadding - leftPadding;

        printf("%*s%s%*s", leftPadding, "", buffer, rightPadding, "");
        printf("\033q");
    }

    check_latest_release();
    return err;
}

bool check_latest_release()
{
    // Check if the latest release is available
    if (connection_data != NULL)
    {
        if (connection_data->status == CONNECTED_WIFI_IP)
        {
            if (poll_latest_release)
            {
                poll_latest_release = false;
                int err = send_sync_command(GET_LATEST_RELEASE, NULL, (__uint16_t)0, 10, false);
                if (err == 0)
                {
                    char *latest_release = (char *)(LATEST_RELEASE_START_ADDRESS + sizeof(__uint32_t));
                    // The latest release is the same as the current version or not?
                    latest_release_available = strlen(latest_release) > 0;
                }
            }
        }
    }
    return latest_release_available;
}

__uint8_t check_network_connection()
{
    if ((connection_data != NULL) && (connection_data->status != CONNECTED_WIFI_IP))
    {
        printf("No WiFi connection found. Connect to a WiFi network first.\r\n");
        printf("Press any key to exit...\r\n");
        getchar();
        return 1; // Error
    }
    return 0; // ok
}

__uint8_t network_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    locate(0, 2);
    printf("Scanning the network...");
    send_sync_command(LAUNCH_SCAN_NETWORKS, NULL, (__uint16_t)0, 10, true);

    printf("\r\033KRetrieving networks...");
    send_sync_command(GET_SCANNED_NETWORKS, NULL, (__uint16_t)0, 10, true);

    printf("\r\n");

    int num_networks = -1;
    __uint32_t network_list_mem = NETWORK_START_ADDRESS + sizeof(__uint32_t);

#ifdef _DEBUG
    printf("Reading network list from memory address: 0x%08X\r\n", network_list_mem);
#endif
    WifiScanData *wifiScanDataBuff = (WifiScanData *)network_list_mem;
    char *network_array = malloc(MAX_SSID_LENGTH * wifiScanDataBuff->count + 1);
    read_networks_from_memory(network_array, wifiScanDataBuff->networks, wifiScanDataBuff->count);

    if (!network_array)
    {
        printf("No networks found!\r\n");
        printf("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }

    __int16_t network_number = display_paginated_content(network_array, get_network_count(network_array), ELEMENTS_PER_PAGE, "Networks", NULL);

    if (network_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    locate(0, 22);

    printf("\033KSelected network: %s\r\n", wifiScanDataBuff->networks[network_number - 1].ssid);
    printf("\033KPlease enter the password of the network:");

    char password[MAX_SSID_LENGTH] = {0};

    if (wifiScanDataBuff->networks[network_number - 1].auth_mode > 0)
    {
        // Read the password
        int ch;
        int index = 0;
        // Repeat until valid input
        while (1)
        {
            ch = getchar();
            if (ch == 13)
            {
                // User pressed Enter
                break;
            }
            else if (ch == 127 || ch == '\b')
            {
                // User pressed backspace
                if (index > 0)
                {
                    // Move back one position in the buffer
                    index--;
                    // Optionally, move cursor back one position and overwrite with space
                    printf(" \b");
                    fflush(stdout);
                }
            }
            else if (ch >= ' ')
            {
                // User entered a printable character
                if (index < sizeof(password) - 1)
                {
                    password[index++] = (char)ch;
                    fflush(stdout);
                }
            }
        }
        for (int i = index; i < sizeof(password); i++)
        {
            password[i] = '\0';
        }
        printf("\033KPassword:%s", password);
    }

    // NOT WORKING. NEED TO FIX
    WifiNetworkAuthInfo network_auth_info;
    // Copy ssid and auth_mode from the selected network
    strcpy(network_auth_info.ssid, wifiScanDataBuff->networks[network_number - 1].ssid);
    network_auth_info.auth_mode = wifiScanDataBuff->networks[network_number - 1].auth_mode;
    strcpy(network_auth_info.password, password);

    send_async_command(CONNECT_NETWORK, &network_auth_info, sizeof(network_auth_info));

    printf("\r\n\033KROM network loaded. ");

    free(network_array);

    return 0; // Return 0 to avoid to force a reset
}

__uint8_t roms_from_network_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    if (check_network_connection() > 0)
    {
        // No network connection. Back to main menu
        return 0;
    }

    int num_files = -1;
    __uint16_t *ptr;
    __uint16_t *network_file_list_mem = (__uint16_t *)(NETWORK_FILE_LIST_START_ADDRESS + sizeof(__uint32_t));

    printf("Getting ROMs list...");
    send_sync_command(GET_ROMS_JSON_FILE, NULL, (__uint16_t)0, 10, true);

    printf("\r\n");

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", network_file_list_mem);
#endif
    char *file_array = read_files_from_memory((__uint8_t *)network_file_list_mem);

    if (!file_array)
    {
        printf("No files found. Check if your network connection is working!\r\n");
        printf("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }
    __int16_t rom_number = display_paginated_content(file_array, get_file_count(file_array), ELEMENTS_PER_PAGE, "ROM images", NULL);

    if (rom_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    locate(0, 22);

    printf("\033KSelected the ROM file: %d. ", rom_number);

    print_file_at_index(file_array, rom_number - 1, 0);

    printf("Downloading ROM. Wait until the led in the board blinks a 'E' or 'D' in morse...");

    send_sync_command(DOWNLOAD_ROM, &rom_number, 2, 30, true);

    printf("\r\033KROM file downloaded. ");

    return 1; // Positive is OK
}

__uint8_t wifi_menu()
{
    if (connection_data->status == DISCONNECTED)
    {
        // If disconnected, connect to a network
        network_selector();
        return 0; // Return to menu
    }
    else
    {
        PRINT_APP_HEADER(VERSION);
        printf("\r\n");
        printf("\r\n");
        printf("SSID: %s\r\n", connection_data->ssid);
        printf("IP: %s\r\n", connection_data->ipv4_address);
        printf("MAC: %s\r\n", connection_data->mac_address);
        printf("Status: %s\r\n", get_status_str(connection_data->status));
        printf("\r\n");
        printf("Press [R]eset to restart the Wifi configuration. [ESC] to exit:");

        while (1)
        {
            if (Cconis())
            {
                int fullkey = Crawcin();
                __uint8_t key = fullkey & 0xFF;
                if (fullkey == KEY_ESC)
                {
                    // Back to main menu
                    return 0; // 0 return to menu, no ask
                }
                // Check if the input is 'R' or 'r'
                if ((key == 'R') || (key == 'r'))
                {
                    send_async_command(DISCONNECT_NETWORK, NULL, 0);
                    please_wait("\r\033KInitializing Wifi in the SidecarT...", 10);
                    network_selector();
                    return 0; // Return to menu
                }
            }
        }
    }
}