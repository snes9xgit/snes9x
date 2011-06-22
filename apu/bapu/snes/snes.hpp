#ifndef __SNES_HPP
#define __SNES_HPP

#define CYCLE_ACCURATE

#include "snes9x.h"

#define alwaysinline inline
#define debugvirtual

namespace SNES
{

struct Processor
{
    unsigned frequency;
    int clock;
};

#include "smp/smp.hpp"
#include "dsp/dsp.hpp"

class CPU
{
public:
    enum { Threaded = false };
    int frequency;
    uint8 registers[4];

    inline void enter ()
    {
        return;
    }

    inline void reset ()
    {
        registers[0] = registers[1] = registers[2] = registers[3] = 0;
    }

    inline void port_write (uint8 port, uint8 data)
    {
        registers[port & 3] = data;
    }

    inline uint8 port_read (uint8 port)
    {
       // printf ("APU Read %2x from port %d\n", registers[port & 3], port & 3);
        return registers[port & 3];
    }
};

extern CPU cpu;

} /* namespace SNES */

#endif
