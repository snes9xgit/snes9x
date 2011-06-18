#ifndef __SNES_HPP
#define __SNES_HPP

#include "snes9x.h"

#define alwaysinline inline
#define debugvirtual

namespace SNES
{

struct Processor
{
    unsigned frequency;
    int64 clock;
};

class CPU
{
public:
    enum { Threaded = false };
    int frequency;

    void enter ()
    {
        return;
    }

    void port_write (uint8 port, uint8 data)
    {

    }

    uint8 port_read (uint8 port)
    {
        return 0;
    }
};

extern CPU cpu;

#include "smp/smp.hpp"
#include "dsp/dsp.hpp"

} /* namespace SNES */

#endif
