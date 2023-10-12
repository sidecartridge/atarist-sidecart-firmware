#ifndef NETWORK_H
#define NETWORK_H

#include <sys/types.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "helper.h"
#include "screen.h"
#include "commands.h"

#define MAX_NETWORKS 100
#define MAX_SSID_LENGTH 34
#define MAX_BSSID_LENGTH 20
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
} WifiNetworkInfo;

typedef struct
{
    char ssid[MAX_SSID_LENGTH];     // SSID to connect
    char password[MAX_SSID_LENGTH]; // Password
    __uint16_t auth_mode;           // auth mode
} WifiNetworkAuthInfo;

typedef struct
{
    __uint32_t magic; // Some magic value for identification/validation
    WifiNetworkInfo networks[MAX_NETWORKS];
    __uint16_t count; // The number of networks found/stored
} WifiScanData;

typedef struct connection_data
{
    char ssid[MAX_SSID_LENGTH];             // SSID to connect
    char ipv4_address[IPV4_ADDRESS_LENGTH]; // IP address
    char ipv6_address[IPV6_ADDRESS_LENGTH]; // IPv6 address
    __uint16_t status;                      // connection status
} ConnectionData;

extern ConnectionStatus connection_status;
extern WifiScanData *wifiScanData;
extern ConnectionData *connection_data;
extern __uint16_t previous_connection_status; // Keep track of the previous connection status

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
    .ipv6_address = "UNKNOWN",
    .status = CONNECTED_WIFI_IP};
#endif

__uint8_t network_selector();
__uint16_t force_connection_status(bool show_bar);
__uint16_t get_connection_status(bool show_bar);
__uint8_t roms_from_network_selector();

#endif // NETWORK_H
