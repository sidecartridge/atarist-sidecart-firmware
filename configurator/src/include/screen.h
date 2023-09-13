#ifndef SCREEN_H_
#define SCREEN_H_

#include <sys/types.h>
#include <stdio.h>

/* CONSOLE UTILITY FUNCTIONS */
#define clearHome() puts("\033E\033H")
#define locate(x, y) printf("\033Y%c%c", (char)(32 + y), (char)(32 + x))

/* SCREEN DEFINITIONS AND FUNCTIONS */
#define LOW_RES 0
#define MEDIUM_RES 1
#define HIGH_RES 2
#define PALETTE_ADDRESS (void *)0xFF8240

typedef struct ScreenContext ScreenContext;
struct ScreenContext
{
    __uint32_t *videoAddress;
    __uint16_t savedResolution;
    __uint16_t savedPalette[16];
};

void initLowResolution(__uint16_t palette[16]);

void initMedResolution(__uint16_t palette[4]);

void initHighResolution();

void restoreScreenContext(ScreenContext *screenContext);

void initScreenContext(ScreenContext *screenContext);

// restores the saved resolution and its palette
void restoreResolutionAndPalette(ScreenContext *screenContext);

// saves the current palette into a buffer (works only in supervisor mode)
void savePalette(__uint16_t *paletteBuffer);

#endif
