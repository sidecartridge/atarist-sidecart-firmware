#ifndef COMMANDS_H_
#define COMMANDS_H_

#define DOWNLOAD_ROM 0          // Download a ROM from the URL
#define LOAD_ROM 1              // Load a ROM from the SD card
#define LIST_ROMS 2             // List the ROMs in the SD card
#define GET_CONFIG 3            // Get the configuration of the device
#define PUT_CONFIG_STRING 4     // Put a configuration string parameter in the device
#define PUT_CONFIG_INTEGER 5    // Put a configuration integer parameter in the device
#define PUT_CONFIG_BOOL 6       // Put a configuration boolean parameter in the device
#define SAVE_CONFIG 7           // Persist the configuration in the FLASH of the device
#define RESET_DEVICE 8          // Reset the device to the default configuration
#define LAUNCH_SCAN_NETWORKS 9  // Launch the scan the networks. No results should return here
#define GET_SCANNED_NETWORKS 10 // Read the result of the scanned networks
#define CONNECT_NETWORK 11      // Connect to a network. Needs the SSID, password and auth method
#define GET_IP_DATA 12          // Get the IP, mask and gateway of the device
#define DISCONNECT_NETWORK 13   // Disconnect from the network
#define GET_ROMS_JSON_FILE 14   // Download the JSON file of ROMs from the URL
#define LOAD_FLOPPY_RO 15       // Load a floppy image from the SD card in read-only mode
#define LIST_FLOPPIES 16        // List the floppy images in the SD card
#define LOAD_FLOPPY_RW 17       // Load a floppy image from the SD card in read-write mode
#define QUERY_FLOPPY_DB 18      // Query the floppy database. Need to pass the letter or number to query
#define DOWNLOAD_FLOPPY 19      // Download a floppy image from the URL
#define GET_SD_DATA 20          // Get the SD card status, size, free space and folders
#define GET_LATEST_RELEASE 21   // Get the latest release version of the firmware
#define CREATE_FLOPPY 22        // Create a floppy image based in a template
#define BOOT_RTC 23             // Boot the RTC emulator
#define CLEAN_START 24          // Start the configurator when the app starts
#define BOOT_GEMDRIVE 25        // Boot the GEMDRIVE emulator
#define REBOOT 26               // Reboot the device

#endif // COMMANDS_H_