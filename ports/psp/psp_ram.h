#ifndef PSP_RAM_H
#define PSP_RAM_H

#include <psptypes.h>


#define RAM_BLOCK      (1024 * 1024)



class psp_ram
{
public:
    psp_ram();
    u32 ramAvailableLineareMax();
    u32 ramAvailable();
};

#endif // PSP_RAM_H
