#ifndef NETWORK_H
#define NETWORK_H

#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "helper.h"
#include "screen.h"
#include "commands.h"

#define MAX_NETWORKS 100
#define MAX_SSID_LENGTH 36 // SSID can have up to 32 characters + null terminator + padding
#define MAX_BSSID_LENGTH 20
#define MAX_PASSWORD_LENGTH 68 // Password can have up to 64 characters + null terminator + padding
#define MAX_RSSID_LENGTH 10
#define IPV4_ADDRESS_LENGTH 16
#define IPV6_ADDRESS_LENGTH 40
typedef enum
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED_WIFI,
    CONNECTED_WIFI_NO_IP,
    CONNECTED_WIFI_IP,
    TIMEOUT_ERROR,
    GENERIC_ERROR,
    NO_DATA_ERROR,
    NOT_PERMITTED_ERROR,
    INVALID_ARG_ERROR,
    IO_ERROR,
    BADAUTH_ERROR,
    CONNECT_FAILED_ERROR,
    INSUFFICIENT_RESOURCES_ERROR,
    NOT_SUPPORTED
} ConnectionStatus;

typedef struct
{
    char ssid[MAX_SSID_LENGTH];   // SSID can have up to 32 characters + null terminator
    char bssid[MAX_BSSID_LENGTH]; // BSSID in the format xx:xx:xx:xx:xx:xx + null terminator
    __uint16_t auth_mode;         // MSB is not used, the data is in the LSB
    __int16_t rssi;               // Received Signal Strength Indicator
} WifiNetworkInfo;

typedef struct
{
    char ssid[MAX_SSID_LENGTH];         // SSID to connect
    char password[MAX_PASSWORD_LENGTH]; // Password
    __uint16_t auth_mode;               // auth mode
} WifiNetworkAuthInfo;

typedef struct
{
    __uint32_t magic; // Some magic value for identification/validation
    WifiNetworkInfo networks[MAX_NETWORKS];
    __uint16_t count; // The number of networks found/stored
} WifiScanData;

// Adjust alignment to 4 bytes always to pass the whole structure to the Atari ST
typedef struct connection_data
{
    char ssid[MAX_SSID_LENGTH];                     // SSID to connect
    char ipv4_address[IPV4_ADDRESS_LENGTH];         // IP address
    char ipv6_address[IPV6_ADDRESS_LENGTH];         // IPv6 address
    char mac_address[MAX_BSSID_LENGTH];             // MAC address
    char gw_ipv4_address[IPV4_ADDRESS_LENGTH];      // Gateway IP address
    char gw_ipv6_address[IPV6_ADDRESS_LENGTH];      // Gateway IPv6 address
    char netmask_ipv4_address[IPV4_ADDRESS_LENGTH]; // Netmask IP address
    char netmask_ipv6_address[IPV6_ADDRESS_LENGTH]; // Netmask IPv6 address
    char dns_ipv4_address[IPV4_ADDRESS_LENGTH];     // DNS IP address
    char dns_ipv6_address[IPV6_ADDRESS_LENGTH];     // DNS IPv6 address
    char wifi_country[4];                           // WiFi country iso-3166-1 alpha-2 code. 2 characters. Example: ES. 2 extra characters for padding
    __uint16_t wifi_auth_mode;                      // WiFi auth mode (WPA, WPA2, WPA3, etc)
    __uint16_t wifi_scan_interval;                  // WiFi scan interval in seconds
    __uint16_t network_status_poll_interval;        // Network status poll interval in seconds
    __uint16_t network_status;                      // Network connection status
    __uint16_t file_downloading_timeout;            // File downloading timeout in seconds
    __int16_t rssi;                                 // Received Signal Strength Indicator
} ConnectionData;

typedef struct
{
    char *code;
    char *name;
} CountryCodeMapping;

#ifdef _DEBUG
static WifiScanData wifi_scan_data_example = {
    .magic = 0x12345678,
    .count = 6,
    .networks = {
        {
            .ssid = "HOME_NETWORK",
            .bssid = "00:11:22:33:44:55",
            .auth_mode = 0x0001,
        },
        {
            .ssid = "OFFICE_NET",
            .bssid = "AA:BB:CC:DD:EE:FF",
            .auth_mode = 0x0002,
        },
        {
            .ssid = "GUEST_WIFI",
            .bssid = "FF:EE:DD:CC:BB:AA",
            .auth_mode = 0x0003,
        },
        {
            .ssid = "COFFE_SHOP",
            .bssid = "11:22:33:44:55:66",
            .auth_mode = 0x0,
        },
        {
            .ssid = "MOBILE_HOSTSPOT",
            .bssid = "66:55:44:33:22:11",
            .auth_mode = 0x0005,
        },
        {
            .ssid = "LIBARY_WIFI",
            .bssid = "11:22:33:44:55:66",
            .auth_mode = 0x0006,
        }},
};
static ConnectionData connection_data_example = {
    .ssid = "HOME_NETWORK",
    .ipv4_address = "1.2.3.4",
    .ipv6_address = "UNKNOWN"};
#endif

__uint16_t get_latest_release();
__uint16_t wifi_menu();
__uint16_t force_connection_status(__uint16_t show_bar);
__uint16_t get_connection_status(__uint16_t show_bar);
__uint16_t roms_from_network_selector();
__uint16_t check_network_connection();

void init_connection_status();

#endif // NETWORK_H
