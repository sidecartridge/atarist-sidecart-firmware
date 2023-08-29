# ATARI ST Sidecart Firmware for the ROM Emulator

This repository contains the firmware code for the Sidecart ROM Emulator on the Atari ST/STE/Mega side. This firmware works jointly with the [Sidecart Raspberry Pico firmware](https://github.com/diegoparrilla/atarist-sidecart-raspberry-pico). Both firmwares are required to make the Sidecart ROM Emulator work.

## Introduction

The Sidecart ROM Emulator emulates the behavior of Atari ST cartridges and the ROM memory contained. This repository contains the code to configure the emulator as a classic TOS Atari ST application. 

In order to make the code usable by the Atari computers, the code is composed of two parts:
1. The configuration tool as a C program in the `/romloader` folder.
2. The bootstraping ROM as an assembly program in the `/src`folder. The ROM will include the configuration tool as an executable.

This ROM can't be loaded or emulated like other ROMs, it must be directly included in the Sidecart RP2040 ROM Emulator firmware. See the [Sidecart Raspberry Pico firmware](https://github.com/diegoparrilla/atarist-sidecart-raspberry-pico).

For those new to the Sidecart ROM Emulator, we recommend visiting the official [Sidecart ROM Emulator website](https://sidecart.xyz) for a comprehensive overview.

## Requirements

- An Atari STe computer (or emulator). There are several emulators available for Windows, Linux, and Mac. I recommend [Hatari](http://hatari.tuxfamily.org/), and I'm also a big fan of [MiSTer](https://misterfpga.org/). It should work on any Atari ST with at least 512KB of RAM.

- The [atarist-toolkit-docker](https://github.com/diegoparrilla/atarist-toolkit-docker): You should read first how to install it and how to use it. It's very easy.

- A `git` client. You can use the command line or a GUI client.

- A Makefile compatible with GNU Make.

## Building the ROM

Once you have your real Atari ST computer, Hatari emulator or MiSTer Atari ST up and running, you can build the program using the following steps:

1. Clone this repository:

```
$ git clone https://github.com/diegoparrilla/atarist-sidecart-firmware.git
```

2. Enter into the repository cloned:

```
cd atarist-sidecart-firmware
```

3. Build the ROM images. Run the `build.sh` script:

```
./build.sh
```

4. The binary files created in the `dist` folder are `BOOT.BIN`, the raw binary file to be included in the Sidecart RP2040 ROM Emulator firmware, and `FIRMWARE.IMG`, the raw binary file that can be directly emulated by the SidecarT itself (for testing purposes).


## Building the ROM loader

If you want to modify the ROM loader, you can do it. The ROM loader is a C program that is compiled using the [atarist-toolkit-docker](https://github.com/diegoparrilla/atarist-toolkit-docker). 

In this example we will use Hatari emulator on Mac OS X. The steps are similar for other platforms.

1. We asssume the repository is already cloned. If not, clone it:

```
$ git clone https://github.com/diegoparrilla/atarist-sidecart-firmware.git
```

2. Enter into the repository cloned and the `romloader` folder:

```
cd atarist-sidecart-firmware/romloader
```

3. Export the `ST_WORKING_FOLDER` environment variable with the absolute path of the `/romloader` folder where you cloned the repository:

```
export ST_WORKING_FOLDER=<ABSOLUTE_PATH_TO_THE_FOLDER_WHERE_YOU_CLONED_THE_REPO/romloader>
```

4. Modify the code as you wish. The code is in the `/src` folder. You can have a look at the [atarist-toolkit-docker](https://github.com/diegoparrilla/atarist-toolkit-docker) examples to learn how to use the environment.

5. Build the code using the Makefile included. Use the `stcmd` command to access the tools in the Docker image. Also set the _DEBUG flag to 1 to enable debug messages and not use directly the ROM:

```
stcmd make DEBUG_MODE=1 
```

6. The TOS executable file created in the `dist` folder is `ROMLOAD.TOS`. This file can be directly executed in the Atari ST emulator or computer. For example, with Hatari it's possible to open it as follows (we assume `hatari` is in the path):

```
hatari --fast-boot true --tos-res med romloader/dist/ROMLOAD.TOS &
```

## Releases

The releases are available in the [Releases page](https://github.com/diegoparrilla/atarist-sidecart-firmware/releases). We strongly recommend to use the latest release.

As explained above, the output of the build process cannot be directly used as a ROM image. The release files are included in the Sidecart RP2040 ROM Emulator firmware.


## Resources 

- [Sidecart ROM Emulator website](https://sidecart.xyz)
- [Sidecart Raspberry Pico firmware](https://github.com/diegoparrilla/atarist-sidecart-raspberry-pico): The second part of the Sidecart ROM Emulator firmware construction happens here.

## License
This project is licenses under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.
