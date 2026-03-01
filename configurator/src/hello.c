#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

int main()
{
    uint16_t boot;

     __asm__ volatile (
        "move.w  0x1000.w,%0\n\t"
        : "=r"(boot)
    );

    printf("Return to GEM.\r\n");
    
    if (boot == 0xAA55) {
        __asm__ volatile (
            "clr.l   -(%%sp)\n\t"
            "move.w  #0x20,-(%%sp)\n\t"
            "trap    #1\n\t"
            "addq.l  #6,%%sp\n\t"
            "move.l  %%d0,%%sp\n\t"
            "rts\n\t"
            :
            :
            : "memory"
        );
        __builtin_unreachable();
    } else {
        // Normal run - normal exit
        exit(0);
    }

    return 0; // Never reached
}