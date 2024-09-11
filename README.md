# ATARI ST Sidecart ROM Emulator Firmware

This repository hosts the firmware code for the Sidecart ROM Emulator designed for Atari ST/STE/Mega systems. In tandem with the [Sidecart Raspberry Pico firmware](https://github.com/sidecartridge/atarist-sidecart-raspberry-pico), this firmware facilitates the functioning of the Sidecart ROM Emulator.

## Introduction

The Sidecart ROM Emulator mimics the behavior of Atari ST cartridges and their contained ROM memory. The code in this repository prepares the emulator to operate as a classic TOS Atari ST application.

The source is bifurcated into:

1. A configuration tool, a C program found in the `/configurator` directory.

2. A bootstrapping ROM, an assembly program housed in the `/src` directory. This ROM embeds the configuration tool as an executable.

**Note**: This ROM cannot be loaded or emulated like conventional ROMs. It has to be merged directly into the Sidecart RP2040 ROM Emulator firmware. Additional details are available in the [Sidecart Raspberry Pico firmware](https://github.com/sidecartridge/atarist-sidecart-raspberry-pico).

Newcomers to the Sidecart ROM Emulator are encouraged to peruse the official [Sidecart ROM Emulator website](https://sidecartridge.com) for a comprehensive understanding.

## Requirements

- An Atari STe computer (or a compatible emulator). Various emulators are accessible for platforms like Windows, Linux, and Mac. [Hatari](http://hatari.tuxfamily.org/) comes recommended, but [MiSTer](https://misterfpga.org/) is also a worthy contender. The requirement is a minimum of 512KB of RAM on any Atari ST.

- The [atarist-toolkit-docker](https://github.com/sidecartridge/atarist-toolkit-docker) is pivotal. Familiarize yourself with its installation and usage.

- A `git` client, command line or GUI, your pick.

- A Makefile attuned with GNU Make.

## Building the ROM

Having your Atari ST computer, Hatari emulator, or MiSTer Atari ST operational, follow these steps to build the program:

1. Clone this repository:

```
$ git clone https://github.com/sidecartridge/atarist-sidecart-firmware.git
```

2. Navigate to the cloned repository:

```
cd atarist-sidecart-firmware
```

3. Trigger the `build.sh` script to build the ROM images:

```
./build.sh
```

4. The `dist` folder now houses the binary files: `BOOT.BIN`, which needs to be incorporated into the Sidecart RP2040 ROM Emulator firmware, and `FIRMWARE.IMG`, a raw binary file tailored for direct emulation by SidecarT (intended for testing).

## Building the ROM Loader

For those inclined to tweak the ROM loader, it's possible. The ROM loader, crafted in C, compiles via the [atarist-toolkit-docker](https://github.com/sidecartridge/atarist-toolkit-docker).

For illustration, let's use the Hatari emulator on macOS:

1. Begin by ensuring the repository is cloned. If not:

```
$ git clone https://github.com/sidecartridge/atarist-sidecart-firmware.git
```

2. Enter the cloned repository, specifically the `configurator` directory:

```
cd atarist-sidecart-firmware/configurator
```

3. Establish the `ST_WORKING_FOLDER` environment variable, linking it to the `/configurator` directory of the cloned repository:

```
export ST_WORKING_FOLDER=<ABSOLUTE_PATH_TO_THE_FOLDER_WHERE_YOU_CLONED_THE_REPO/configurator>
```

4. Embark on your code modifications within the `/src` folder. For insights on leveraging the environment, refer to the [atarist-toolkit-docker](https://github.com/sidecartridge/atarist-toolkit-docker) examples.

5. Leverage the provided Makefile for the build. The `stcmd` command connects with the tools in the Docker image. Engage the `_DEBUG` flag (set to 1) to activate debug messages and bypass direct ROM usage:

```
stcmd make DEBUG_MODE=1 
```

6. The outcome is `SIDECART.TOS` in the `dist` folder. This file is ready for execution on the Atari ST emulator or computer. If using Hatari, you can launch it as follows (assuming `hatari` is path-accessible):

```
hatari --fast-boot true --tos-res med configurator/dist/SIDECART.TOS &
```

## Releases

For releases, head over to the [Releases page](https://github.com/sidecartridge/atarist-sidecart-firmware/releases). The latest release is always recommended.

Note: The build output isn't akin to standard ROM images. The release files have to be incorporated into the Sidecart RP2040 ROM Emulator firmware.

## Resources 

- [Sidecart ROM Emulator website](https://sidecartridge.com)
- [Sidecart Raspberry Pico firmware](https://github.com/sidecartridge/atarist-sidecart-raspberry-pico) - Where the second phase of the Sidecart ROM Emulator firmware evolution unfolds.

## License

The project is licensed under the GNU General Public License v3.0. The full license is accessible in the [LICENSE](LICENSE) file.
