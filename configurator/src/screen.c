#include <stdlib.h>
#include "include/screen.h"

void initScreenContext(ScreenContext *screenContext)
{
    screenContext->videoAddress = Logbase();   // Get the logical pointer of the video RAM
    screenContext->savedResolution = Getrez(); // Get current resolutio
    savePalette(screenContext->savedPalette);  // Save the palette
}

// saves the current palette into a buffer (works only in supervisor mode)
void savePalette(__uint16_t *paletteBuffer)
{
    // 16 colors, 16 bits each.
    __uint16_t *src = (void *)PALETTE_ADDRESS;
    for (int i = 0; i < 16; i++)
    {
        paletteBuffer[i] = src[i];
    }
}

// restores the saved palette (works only in supervisor mode)
void restorePalette(__uint16_t *paletteBuffer)
{
    // 16 colors, 16 bits each.
    __uint16_t *dest = (void *)PALETTE_ADDRESS;
    for (int i = 0; i < 16; i++)
    {
        dest[i] = paletteBuffer[i];
    }
}

// restores the saved resolution and its palette
void restoreResolutionAndPalette(ScreenContext *screenContext)
{
    Setscreen(screenContext->videoAddress, screenContext->videoAddress, screenContext->savedResolution);
    restorePalette(screenContext->savedPalette);
}

void initLowResolution(__uint16_t palette[16])
{
    Setscreen(-1, -1, LOW_RES);
    restorePalette(palette);
}

void initMedResolution(__uint16_t palette[4])
{
    Setscreen(-1, -1, MEDIUM_RES);
    restorePalette(palette);
}

void initHighResolution()
{
    Setscreen(-1, -1, HIGH_RES);
}

void restoreScreenContext(ScreenContext *screenContext)
{
    restoreResolutionAndPalette(screenContext);
}
