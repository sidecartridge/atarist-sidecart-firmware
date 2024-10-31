#include "include/network.h"

static __uint16_t poll_latest_release;

static ConnectionStatus connection_status = DISCONNECTED;
static ConnectionData *connection_data = NULL;

// current supported country code https://www.raspberrypi.com/documentation/pico-sdk/networking.html#CYW43_COUNTRY_
// ISO-3166-alpha-2
// XX select worldwide
static char *valid_country_code[] = {
    "XX", "AU", "AR", "AT", "BE", "BR", "CA", "CL",
    "CN", "CO", "CZ", "DK", "EE", "FI", "FR", "DE",
    "GR", "HK", "HU", "IS", "IN", "IL", "IT", "JP",
    "KE", "LV", "LI", "LT", "LU", "MY", "MT", "MX",
    "NL", "NZ", "NG", "NO", "PE", "PH", "PL", "PT",
    "SG", "SK", "SI", "ZA", "KR", "ES", "SE", "CH",
    "TW", "TH", "TR", "GB", "US"};

static const size_t NUM_VALID_COUNTRY_CODES = sizeof(valid_country_code) / sizeof(valid_country_code[0]);

static CountryCodeMapping country_codes[] = {
    {"XX", "Worldwide"},
    {"AU", "Australia"},
    {"AR", "Argentina"},
    {"AT", "Austria"},
    {"BE", "Belgium"},
    {"BR", "Brazil"},
    {"CA", "Canada"},
    {"CL", "Chile"},
    {"CN", "China"},
    {"CO", "Colombia"},
    {"CZ", "Czech Republic"},
    {"DK", "Denmark"},
    {"EE", "Estonia"},
    {"FI", "Finland"},
    {"FR", "France"},
    {"DE", "Germany"},
    {"GR", "Greece"},
    {"HK", "Hong Kong"},
    {"HU", "Hungary"},
    {"IS", "Iceland"},
    {"IN", "India"},
    {"IL", "Israel"},
    {"IT", "Italy"},
    {"JP", "Japan"},
    {"KE", "Kenya"},
    {"LV", "Latvia"},
    {"LI", "Liechtenstein"},
    {"LT", "Lithuania"},
    {"LU", "Luxembourg"},
    {"MY", "Malaysia"},
    {"MT", "Malta"},
    {"MX", "Mexico"},
    {"NL", "Netherlands"},
    {"NZ", "New Zealand"},
    {"NG", "Nigeria"},
    {"NO", "Norway"},
    {"PE", "Peru"},
    {"PH", "Philippines"},
    {"PL", "Poland"},
    {"PT", "Portugal"},
    {"SG", "Singapore"},
    {"SK", "Slovakia"},
    {"SI", "Slovenia"},
    {"ZA", "South Africa"},
    {"KR", "South Korea"},
    {"ES", "Spain"},
    {"SE", "Sweden"},
    {"CH", "Switzerland"},
    {"TW", "Taiwan"},
    {"TH", "Thailand"},
    {"TR", "Turkey"},
    {"GB", "United Kingdom"},
    {"US", "United States"}};

static void read_networks_from_memory(char *ssids, const WifiNetworkInfo networks[], uint16_t total_size)
{
    char *current_ssid_position = ssids;

    for (uint16_t i = 0; i < total_size; i++)
    {
        char line[80]; // 79 characters + null terminator

        // Initialize the line buffer with spaces
        memset(line, ' ', 80);
        line[79] = '\0'; // Null-terminate the line

        // Copy the SSID into positions 0 to 39 (40 characters)
        size_t ssid_len = strlen(networks[i].ssid);
        if (ssid_len > 40)
            ssid_len = 40; // Truncate if necessary
        memcpy(line, networks[i].ssid, ssid_len);

        // Copy the BSSID into positions 40 to 69 (30 characters)
        size_t bssid_len = strlen(networks[i].bssid);
        if (bssid_len > 30)
            bssid_len = 30; // Truncate if necessary
        memcpy(line + 40, networks[i].bssid, bssid_len);

        // Format the RSSI value as a string
        char rssi_str[8]; // Enough to hold up to 8 characters
        int rssi_str_len = snprintf(rssi_str, sizeof(rssi_str), "%ddb", networks[i].rssi);
        if (rssi_str_len < 0)
            rssi_str_len = 0; // Handle snprintf error
        if (rssi_str_len > 8)
            rssi_str_len = 8; // Truncate if necessary

        // Copy the RSSI string into positions 70 to 79
        memcpy(line + 70, rssi_str, rssi_str_len);

        // Copy the 80-character line into the ssids buffer
        memcpy(current_ssid_position, line, 80);
        current_ssid_position += 80;
    }

    // Null-terminate the entire ssids buffer
    *current_ssid_position = '\0';
}

static __uint16_t get_network_count(char *file_array)
{
    __uint16_t count = 0;
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
    switch (connection_data->network_status)
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

static void check_latest_release()
{
    // Check if the latest release is available
    if (connection_data != NULL)
    {
        ConfigEntry *entry = get_config_entry(PARAM_WIFI_DHCP);
        int is_dhcp_enabled = entry != NULL && (entry->value[0] == 't' || entry->value[0] == 'T');
        if ((connection_data->network_status == CONNECTED_WIFI_IP) || (!is_dhcp_enabled && (connection_data->network_status == CONNECTED_WIFI_NO_IP)))
        {
            if (poll_latest_release)
            {
                poll_latest_release = FALSE;
                send_async_command(GET_LATEST_RELEASE, NULL, 0);
            }
        }
    }
}

void init_connection_status()
{
    connection_data = malloc(sizeof(ConnectionData));
    memset(connection_data, 0, sizeof(ConnectionData));
    poll_latest_release = TRUE;
}

__uint16_t get_connection_status(__uint16_t show_bar)
{
    char buffer[STATUS_STRING_BUFFER_SIZE];
    int err = send_sync_command(GET_IP_DATA, NULL, (__uint16_t)0, NETWORK_DATA_WAIT_TIME, FALSE);

    if (err != 0)
    {
        // snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "Cannot read network status. Retrying...");
        return err;
    }
    else
    {
        if (connection_data == NULL)
        {
            connection_data = malloc(sizeof(ConnectionData));
        }
        memcpy(connection_data, (ConnectionData *)(CONNECTION_STATUS_START_ADDRESS + sizeof(__uint32_t)), sizeof(ConnectionData));
        char *status_str = get_status_str(connection_data->network_status);
        if (connection_data->rssi < 0)
        {
            snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "IP: %s | SSID: %s (%ddb)| Status: %s",
                     connection_data->ipv4_address, connection_data->ssid, connection_data->rssi, status_str);
        }
        else
        {
            snprintf(buffer, STATUS_STRING_BUFFER_SIZE, "IP: %s | SSID: %s | Status: %s",
                     connection_data->ipv4_address, connection_data->ssid, status_str);
        }
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

__uint16_t get_latest_release()
{
    char *latest_release = (char *)(LATEST_RELEASE_START_ADDRESS + sizeof(__uint32_t));
    return strlen(latest_release) > 0;
}

__uint16_t check_network_connection()
{
    if ((connection_data != NULL) && (connection_data->network_status != CONNECTED_WIFI_IP))
    {
        printf("No WiFi connection found. Connect to a WiFi network first.\r\n");
        press_key("Press any key to exit...\r\n");
        return 1; // Error
    }
    return 0; // ok
}

__uint16_t network_selector()
{
    PRINT_APP_HEADER(VERSION);

    printf("\r\n");

    locate(0, 2);
    printf("Scanning the network...");
    send_sync_command(LAUNCH_SCAN_NETWORKS, NULL, (__uint16_t)0, NETWORK_WAIT_TIME, TRUE);

    printf("\r\033KRetrieving networks...");


    uint16_t retries = COMMAND_NUM_RETRIES;
    int err = TRUE;
    while (retries--)
    {
        err = send_sync_command(GET_SCANNED_NETWORKS, NULL, (__uint16_t)0, NETWORK_WAIT_TIME, TRUE);
        if (!err)
        {
            break;
        }
        sleep_seconds(1, TRUE);
    }
    if (err)
    {
        press_key("\r\n\033KError querying networks. Press a key to continue. ");
        // No connection. Back to main menu
        return 0;
    }

    printf("\r\n");

    int num_networks = -1;
    __uint32_t network_list_mem = (__uint32_t)(NETWORK_START_ADDRESS + sizeof(__uint32_t));

#ifdef _DEBUG
    printf("Reading network list from memory address: 0x%08X\r\n", network_list_mem);
#endif

    WifiScanData *wifiScanDataBuff = (WifiScanData *)network_list_mem;
    char *network_array = malloc(((MAX_SSID_LENGTH + MAX_RSSID_LENGTH) * wifiScanDataBuff->count) + 1);
    read_networks_from_memory(network_array, wifiScanDataBuff->networks, wifiScanDataBuff->count);

    if (!network_array)
    {
        printf("No networks found!\r\n");
        press_key("Press any key to exit...\r\n");
        // Back to main menu
        return 0; // 0 is go to menu
    }

    flush_kbd();

    __int16_t network_number = display_paginated_content(network_array, get_network_count(network_array), ELEMENTS_PER_PAGE, "Networks", NULL);

    if (network_number <= 0)
    {
        // Back to main menu
        return 0; // 0 is go to menu
    }

    flush_kbd();

    locate(0, 22);

    printf("\033KSelected network: %s\r\n", wifiScanDataBuff->networks[network_number - 1].ssid);
    printf("\033KPlease enter the password of the network:");

    char password[MAX_SSID_LENGTH] = {0};

    if (wifiScanDataBuff->networks[network_number - 1].auth_mode > 0)
    {
        read_string(password, MAX_SSID_LENGTH);
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

__uint16_t roms_from_network_selector()
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
    uint16_t retries = COMMAND_NUM_RETRIES;
    int err = TRUE;
    while (retries--)
    {
        err = send_sync_command(GET_ROMS_JSON_FILE, NULL, (__uint16_t)0, NETWORK_WAIT_TIME, TRUE);
        if (!err)
        {
            break;
        }
        sleep_seconds(1, TRUE);
    }
    if (err)
    {
        press_key("\r\n\033KError querying database. Press a key to continue. ");
        // No connection. Back to main menu
        return 0;
    }

    printf("\r\n");

    flush_kbd();

#ifdef _DEBUG
    printf("Reading file list from memory address: 0x%08X\r\n", network_file_list_mem);
#endif
    char *file_array = read_files_from_memory((char *)network_file_list_mem);

    if (!file_array)
    {
        printf("No files found. Check if your network connection is working!\r\n");
        press_key("Press any key to exit...\r\n");
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

    printf("\r\nDownloading ROM. Wait until the led in the board blinks a 'E' or 'D' in morse...");

    int download_status = send_sync_command(DOWNLOAD_ROM, &rom_number, 2, get_download_timeout(), COUNTDOWN);
    if (download_status == 0)
    {
        printf("\r\033KROM file downloaded. ");
        return 1; // different than zero is OK
    }
    printf("\r\033KError downloading ROM file: %d. Press a key to continue. ", download_status);
    press_key("");
    return 0; // A zero is return to menu
}

// Get the country name from the country code
const char *get_country_name(const char *code)
{
    static char result[256]; // Large enough buffer for country name and code
    for (int i = 0; i < NUM_VALID_COUNTRY_CODES; i++)
    {
        if (strcmp(code, country_codes[i].code) == 0)
        {
            // Format the result as "Country Name (Code)"
            snprintf(result, sizeof(result), "%s (%s)", country_codes[i].name, country_codes[i].code);
            return result;
        }
    }
    return "Country not found";
}

__uint16_t wifi_menu()
{
    if (connection_data->network_status == DISCONNECTED)
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
        printf("+ Wi-Fi status\r\n");
        printf("+-- SSID: %s\r\n", connection_data->ssid);
        printf("+-- Auth mode: ");
        switch (connection_data->wifi_auth_mode)
        {
        case 0:
            printf("Open\r\n");
            break;
        case 1:
        case 2:
            printf("WPA TKIP PSK\r\n");
            break;
        case 3:
        case 4:
        case 5:
            printf("WPA2 AES PSK\r\n");
            break;
        case 6:
        case 7:
        case 8:
            printf("WPA2 MIXED PSK\r\n");
            break;
        default:
            printf("Unknown (Open)\r\n");
        }
        printf("+-- Scan interval: %ds\r\n", connection_data->wifi_scan_interval);
        printf("+-- Country: %s\r\n", get_country_name(connection_data->wifi_country));

        printf("+-- Status: %s\r\n", get_status_str(connection_data->network_status));
        printf("\r\n+ Network Status\r\n");
        printf("+-- IP: %s\r\n", connection_data->ipv4_address);
        printf("+-- Gateway: %s\r\n", connection_data->gw_ipv4_address);
        printf("+-- Netmask: %s\r\n", connection_data->netmask_ipv4_address);
        printf("+-- DNS: %s\r\n", connection_data->dns_ipv4_address);
        printf("+-- MAC: %s\r\n", connection_data->mac_address);
        printf("+-- Check interval: %ds\r\n", connection_data->network_status_poll_interval);
        printf("\r\n+ Application status\r\n");
        printf("+-- File download timeout: %ds\r\n", get_download_timeout());

        printf("\r\n");
        printf("Press [R]eset to restart the Wifi configuration. [ESC] to exit:");

        flush_kbd();

        while (1)
        {
            if (Bconstat(2) != 0)
            {
                int fullkey = Bconin(2);
                __uint16_t key = fullkey & 0xFF;
                if (fullkey == KEY_ESC)
                {
                    // Back to main menu
                    return 0; // 0 return to menu, no ask
                }
                // Check if the input is 'R' or 'r'
                if ((key == 'R') || (key == 'r'))
                {
                    send_async_command(DISCONNECT_NETWORK, NULL, 0);
                    please_wait("\r\033KInitializing Wifi in the device...", 10);
                    network_selector();
                    return 0; // Return to menu
                }
            }
        }
    }
}